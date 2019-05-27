//
// Created by kiran.narayanaswamy on 1/3/19.
//

#include "../../header/apps/TaskD.h"
zmq::context_t *TaskClusterInfoCollectorContext;
zmq::socket_t *TaskClusterInfoCollectorSubscriber;


/* Constructor that reads XML Config file to populate required variables */
TASK_DISTRIBUTION::TASK_DISTRIBUTION(std::string ID, LOGGING_UTILITY *_LogsObj, LOCAL_INFO *_LocalInfoObj, TASK_CLUSTERINFO *_ClsuterObj, COMMUNICATION *_CommObj, STATISTICS *_StatsObj) {
    CarExists = true;
    VID = ID;
    LogsObj = _LogsObj;
    LocalInfoObj = _LocalInfoObj;
    ClusterObj = _ClsuterObj;
    CommObj = _CommObj;
    StatsObj = _StatsObj;
    TaskSeqNum = 1;
    ClusterObj->ClusterState = "INIT";

    pugi::xml_document doc;
    if (!doc.load_file("../config/task/config.xml")) {
        LogsObj->LogError("ERROR:: Config File Missing ../config/task/config.xml !!\n");
    } else {
        pugi::xml_node tools = doc.child("VMCPI").child("config");
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                if (name == "task_interval")
                     TaskInterval = (__useconds_t )std::stoi(ait->value());
                else if(name == "task_maxtime")
                    TaskMaxTime = (__useconds_t )std::stoi(ait->value());
                else if(name == "task_mintime")
                    TaskMinTime = (__useconds_t )std::stoi(ait->value());
            }
        }
    }

    auto _VisualsIPInfo = new struct Visuals_IP_Info();
    VisualsIPInfo = _VisualsIPInfo;


    if (!doc.load_file("../config/visuals/config.xml")) {
        std::cout << "ERROR:: Config File Missing ../config/visuals/config.xml !!\n";
	LogsObj->LogError("ERROR:: Config File Missing ../config/visuals/config.xml !!");
    } else {
        pugi::xml_node tools = doc.child("VMCPI").child("config");
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                if (name == "IP")
                    VisualsIPInfo->VisualsIP = ait->value();
                else if (name == "Port")
                    VisualsIPInfo->VisualsPort = ait->value();
            }
        }
    }
}

/**
	Computes subtask(virtual subtask) and send the results to Cluster Head
	@param DataString is the subtask assignment packet
*/

void TASK_DISTRIBUTION::CompleteTaskAndSendResults(ALLDATA *DataString) {

    auto CommObj = new COMMUNICATION("DEFAULT", LogsObj);
    long MaxTime = DataString->taskmaxtime();

	/* Sets a random timer between 1 to MaxTime (before which subtask results should be computed) */
    __useconds_t FinishTime = rand() % (__useconds_t )MaxTime;
    usleep(FinishTime);

    auto TaskResultsObj = new TASK_RESULTS();
    TaskResultsObj->FinishTime = FinishTime;
    TaskResultsObj->SrcID = VID;
    TaskResultsObj->Timestamp = LogsObj->GetCurrentTime();
    TaskResultsObj->DatatType = "TASK_RESULTS";
    TaskResultsObj->Result = "Result " + VID;
    TaskResultsObj->TaskSeqNum = DataString->taskseqnum();

	/* Sends subtask results in a unicast message */
    CommObj->SendUnicastMsg(VID, DataString->id(), TaskResultsObj, (char*)"A", "TASK_RESULTS");
    StatsObj->TotalSubTasksCompleted++;
    LogsObj->LogStats("Total subtasks computed " + std::to_string(StatsObj->TotalSubTasksCompleted));
    delete CommObj;
    delete TaskResultsObj;

}

/**
	Marks which Cluster Member has sent subtask results */

void TASK_DISTRIBUTION::ProcessTaskResults(ALLDATA *DataString){
    ResultsCollectionChart[DataString->id()] = true;
}

/**
        Function is called when Task Distribution module receives message 
        @param NewMessage The new message that is received 
*/


void TASK_DISTRIBUTION::ProcessReceivedMsg(std::string NewMessage) {
    MESSAGEFROMUDM Message;
    Message.ParseFromString(NewMessage);
    std::string DataType, SourceType;
    SourceType = Message.src_type();
    /*! Checks the source type and data type and process it accordingly */
    ALLDATA DataString = Message.data();
    DataType = DataString.datatype();
    if(SourceType == "CAR") {
        if(DataType == "TASK_ASSIGN") {
            LogsObj->LogDebug("Received Task Assignment from " + DataString.id());
            CompleteTaskAndSendResults(&DataString);
        } else if(DataType == "TASK_RESULTS") {
            LogsObj->LogDebug("Received Task Results from " + DataString.id());
            ProcessTaskResults(&DataString);
        }
    }
}


/** 
	Creates virtual tasks 
	@return time in micro seconds that Cluster Head waited for Cluster Members results
*/
__useconds_t TASK_DISTRIBUTION::CreateTasksAndSendToCM() {
    COMMUNICATION *CommObj = new COMMUNICATION("DEFAULT", LogsObj);

    /* Creates a virtual subtask */
    TASK NewTask;
    NewTask.SrcID = VID;
    NewTask.Timestamp = LogsObj->GetCurrentTime();
    NewTask.DatatType = "TASK_ASSIGN";
    NewTask.MaxTime = (rand() % (TaskMaxTime-TaskMinTime)) + (TaskMinTime);
    NewTask.TaskSeqNum = TaskSeqNum;
    PrevCMList = ClusterObj->CMList;
    int Count = 0;

	/* Send subtask assignment message to all cluster members */
    for(auto it = PrevCMList.begin(); it != PrevCMList.end(); it++) {
        CommObj->SendUnicastMsg(VID, *it, &NewTask, (char*)"A", "TASK_ASSIGN");
        ResultsCollectionChart[*it] = false;
        Count++;
    }
    StatsObj->TotalSubTasksAssigned += Count;
    LogsObj->LogStats("Total Subtasks assigned to CMs " + std::to_string(StatsObj->TotalSubTasksAssigned));
    LogsObj->LogDebug("Sending Task Assignment to " + std::to_string(Count) + " number of CMs");

    TaskSeqNum++;
    delete CommObj;

	/* Wait for specified time (NewTask.MaxTime + 1000) micro seconds for the subtask results */
    __useconds_t SleepTime = ((__useconds_t)NewTask.MaxTime + 1000) * 1000;
    usleep(SleepTime);
    Count = 0;
    for (auto it = ResultsCollectionChart.begin(); it != ResultsCollectionChart.end(); it++) {
        if (it->second)
            Count++;
    }
    StatsObj->TotalSubTasksResultsReceived += Count;
    LogsObj->LogStats("Total Subtasks Results received " + std::to_string(StatsObj->TotalSubTasksResultsReceived));

    if(Count == ResultsCollectionChart.size()) {
        LogsObj->LogDebug("ALL Cluster members completed tasks in time and CH received results");
    } else {
        LogsObj->LogDebug("Out of " + std::to_string(ResultsCollectionChart.size()) + " CH received " + std::to_string(Count) + " number of CMs results :: CH may be out of reach to some CM\n");
    }
    ResultsCollectionChart.clear();
    return SleepTime;
}

/**
	Function that distriute subtasks periodically
*/

void TASK_DISTRIBUTION::DistributeTasks() {
    __useconds_t SleepTime = 0;
    while (CarExists) {
		/* Cluster Head creates and send subtask assignment messages periodically */
        if (ClusterObj->AM_I_CH && ClusterObj->ClusterState != "INIT") {
            usleep((TaskInterval * 1000000) - SleepTime);
            SleepTime = CreateTasksAndSendToCM();
            StatsObj->TotalTasksAssigned++;
            LogsObj->LogStats("Total Tasks Assigned to CM " + std::to_string(StatsObj->TotalTasksAssigned));
            continue;
        }
        sleep(1);
    }
}

/** 
	Receiver created in Communication module to receive messages in Task Distribution application 
*/

void TASK_DISTRIBUTION::CollectResults() {
    CommObj->ReceiveMessage((void*)this, VID + "TASKD", (char*)"A", "TASKD");
}

/* Thread that receives Car exit status */

void T_CarExitStatusUpdate(TASK_DISTRIBUTION *TaskDObj) {
    zmq::message_t Address, MessageReceived;
    zmq::context_t *Context;
    zmq::socket_t *Subscriber;
    COMMUNICATION *CommObj;
    CommObj = new COMMUNICATION(TaskDObj->LogsObj);

    if(true) {
        Context = new zmq::context_t(1);
        Subscriber = new zmq::socket_t(*Context, ZMQ_SUB);
        Subscriber->connect("tcp://" + CommObj->GetParserIP() + ":" + (std::to_string(std::stoi(CommObj->GetUDMPort()) + 3)));
        std::string ID = TaskDObj->GetVehicleID() + "TASKDEXITID";
        Subscriber->setsockopt(ZMQ_SUBSCRIBE, ID.c_str(), std::strlen(ID.c_str()));
        Subscriber->recv(&Address);
        Subscriber->recv(&MessageReceived);
        CommObj->KillReceiverSocket();
        TaskClusterInfoCollectorSubscriber->close();
        TaskClusterInfoCollectorContext->close();
        TaskDObj->CarExists = false;
    }

    delete CommObj;
    Subscriber->close();
    Context->close();
}

void T_DistributeTasks(TASK_DISTRIBUTION *TaskDObj) {
    TaskDObj->DistributeTasks();
}

void T_Collect_Results(TASK_DISTRIBUTION *TaskDObj) {
    TaskDObj->CollectResults();
}

void T_SendTaskDetailsToVisuals(TASK_DISTRIBUTION *TaskObj) {
    TaskObj->MessageToVisuals();
}


/** 
        Send Task Distribution application details to Visual Component periodically 
*/

void TASK_DISTRIBUTION::MessageToVisuals() {
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_PUSH);

    socket.connect("tcp://"+VisualsIPInfo->VisualsIP+":"+VisualsIPInfo->VisualsPort);

    while(CarExists) {
        VISUALSDATA DataCountInfo;

        DataCountInfo.set_messagetype("TASK");
        DataCountInfo.set_srctype("CAR");
        DataCountInfo.set_srcid(VID);
        DataCountInfo.set_totalsubtasks(StatsObj->TotalSubTasksAssigned);
        DataCountInfo.set_totalsubtasksresults(StatsObj->TotalSubTasksResultsReceived);
        DataCountInfo.set_totalsubtaskscomputed(StatsObj->TotalSubTasksCompleted);

        std::string DataToSend;
        DataCountInfo.SerializeToString(&DataToSend);

        zmq::message_t Packet(DataToSend.size());
        memcpy ((void*)Packet.data(), (void*)DataToSend.data(), DataToSend.length());
        socket.send(Packet);
        sleep(4);
    }
}


/**
        Creates necessary threads for this application 
*/

void TASK_DISTRIBUTION::TaskDistributionService() {

    LogsObj->LogDebug("Starting 'TaskD Distribute Tasks' thread");
    LogsObj->LogInfo("Starting 'TaskD Distribute Tasks' thread");
    std::thread DistributeTasks(T_DistributeTasks, this);

    LogsObj->LogDebug("Starting 'TaskD Collect Results' thread");
    LogsObj->LogInfo("Starting 'TaskD Collect Results' thread");
    std::thread CollectResults(T_Collect_Results, this);

    LogsObj->LogDebug("Starting 'TaskD Car Exit status' thread");
    LogsObj->LogInfo("Starting 'TaskD Car Exit status' thread");
    std::thread CarExitStatusUpdate(T_CarExitStatusUpdate, this);

    std::thread SendTaskDetailsToVisuals(T_SendTaskDetailsToVisuals, this);

    CollectClusterInfo();

    DistributeTasks.join();
    CollectResults.join();
    CarExitStatusUpdate.join();
}

/** 
        Subscribes to Clustering service and receives cluster status updates 
*/

void TASK_DISTRIBUTION::CollectClusterInfo() {
    auto ClusterInfoCollectorContext = new zmq::context_t(1);
    auto ClusterInfoCollectorSubscriber = new zmq::socket_t(*ClusterInfoCollectorContext, ZMQ_SUB);
    ClusterInfoCollectorSubscriber->connect("tcp://" + LocalInfoObj->GetCarServerIP() +":" +LocalInfoObj->GetCarClusterPort());
    ClusterInfoCollectorSubscriber->setsockopt( ZMQ_SUBSCRIBE, "APP", 3);

    CARCLUSTERINFORESPONSE ResponseMsg;
    zmq::message_t MessageReceived;
    zmq::message_t Address;

    while(true) {
        try {
            ClusterInfoCollectorSubscriber->recv(&Address);
            ClusterInfoCollectorSubscriber->recv(&MessageReceived);
        } catch(zmq::error_t& e) {
            std::cout << "Exception raised\n";
            break;
        }

        std::string RequestBuffer(static_cast<char *>(MessageReceived.data()), MessageReceived.size());
        if(RequestBuffer.empty()) {
            CarExists = false;
            LogsObj->LogDebug("CAR Exited from the network ");
            LogsObj->LogInfo("CAR Exited from the network ");
            break;
        }

        ResponseMsg.ParseFromString(RequestBuffer);
        LogsObj->LogDebug("Received Cluster State from Car Cluster Service State = " + ClusterObj->ClusterState);
        LogsObj->LogInfo("Received Cluster State from Car Cluster Service State = " + ClusterObj->ClusterState);
        ClusterObj->DefaultClusterManager = ResponseMsg.default_manager();

        if(ResponseMsg.cluster_state() != "INIT") {
            ClusterObj->CMList.clear();
            ClusterObj->AM_I_CH = ResponseMsg.is_cluster_head();
            if(!ClusterObj->AM_I_CH)
                ClusterObj->CH = ResponseMsg.ch();
            else {
                ClusterObj->CH = VID;
                ClusterObj->ClusterManager = ResponseMsg.cluster_manager();
                ClusterObj->MC_ID = ResponseMsg.cluster_id();
            }
            LogsObj->LogDebug("Cluster Head = " + ClusterObj->CH);
            std::string ListOfCM = ResponseMsg.cm_list();
            size_t pos = 0;
            std::string token, delimiter = " ";
            while ((pos = ListOfCM.find(delimiter)) != std::string::npos) {
                token = ListOfCM.substr(0, pos);
                ClusterObj->CMList.push_front(token);
                ListOfCM.erase(0, pos + delimiter.length());
            }
            for(auto it = ClusterObj->CMList.begin(); it != ClusterObj->CMList.end(); it++) {
                LogsObj->LogDebug("Cluster Member " + *it);
            }
        } 
        ClusterObj->ClusterState = ResponseMsg.cluster_state();
        StatsObj->ClusterSequenceNumber = ResponseMsg.cluster_seq_num();
    }
}

