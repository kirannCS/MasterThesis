//
// Created by kiran on 10/27/18.
//

#include "../../header/apps/DCA.h"
zmq::context_t *ClusterInfoCollectorContext;
zmq::socket_t *ClusterInfoCollectorSubscriber;


/** 
        Constructor that reads XML Config file to populate required variables 
*/
DCA_DATACOLLECTION::DCA_DATACOLLECTION(std::string ID, LOGGING_UTILITY *_LogsObj
        , LOCAL_INFO *_LocalInfoObj, DCA_CLUSTERINFO *_DCAClsuterObj, COMMUNICATION *_CommObj, STATISTICS *_StatsObj) {
    VID = ID;
    LogsObj = _LogsObj;
    LocalInfoObj = _LocalInfoObj;
    DCAClusterObj = _DCAClsuterObj;
    CommObj = _CommObj;
    StatsObj = _StatsObj;

    /* Cars initial state is set to 'INIT' */
    DCAClusterObj->ClusterState = "INIT";
    pugi::xml_document doc;
    if (!doc.load_file("../config/dca/config.xml")) {
        LogsObj->LogError("ERROR:: Config File Missing ../config/dca/config.xml !!\n");
    } else {
        pugi::xml_node tools = doc.child("VMCPI").child("config");
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                if (name == "CMDataUpdateInterval")
                    SetCMDataUpdateInterval(std::stod(ait->value()));
                else if(name == "DataFilePath")
                    DataFilePath = ait->value();
                else if(name == "CHReceivedDataFilePath")
                    CHReceivedDataFilePath = ait->value();
                else if(name == "CHGeneratedDataFilePath")
                    CHGeneratedDataFilePath = ait->value();
                else if(name == "ChunkSize")
                    ChunkSize = std::stoi(ait->value());
                else if(name == "DataSize")
                    DataSize = std::stoi(ait->value());
                /*else if(name == "APDataFilePath")
                    APDataFilePath = ait->value();*/
                else if(name == "AggregationRequired")
                    IsAggregationReq = (strcmp(ait->value(), "1") == 0) ? true: false;
                else if(name == "AggregationPercentage")
                    AggregationPercentage = std::stod(ait->value());
                else if(name == "APDataUpdateInterval")
                    APDataUpdateInterval = std::stoi(ait->value());
            }
        }
    }

    auto _VisualsIPInfo = new struct Visuals_IP_Info();
    VisualsIPInfo = _VisualsIPInfo;

    pugi::xml_node tools = doc.child("VMCPI").child("config");
    if (!doc.load_file("../config/visuals/config.xml")) {
        std::cout << "ERROR:: Config File Missing ../config/visuals/config.xml !!\n";
	LogsObj->LogError("ERROR:: Config File Missing ../config/visuals/config.xml !!");
    } else {
        tools = doc.child("VMCPI").child("config");

        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                std::cout << "Name = " << name << " Vaue = " << ait->value() << std::endl;
                if (name == "IP")
                    VisualsIPInfo->VisualsIP = ait->value();
                else if (name == "Port")
                    VisualsIPInfo->VisualsPort = ait->value();
            }
        }
    }
    
    CarExists = true;
    /* Counts packets(fragments) sent to Cluster Head */
    CMSequenceNum = 1;
    /* Counts packets(fragments) sent to Accesspoint */
    CHSequenceNum = 1;
}

void T_SendDataToCH(DCA_BASE_DATACOLLECTION *DCAObj) {
    DCAObj->SendDataToCH();
}

void T_SendDataToAP(DCA_BASE_DATACOLLECTION *DCAObj) {
    DCAObj->SendDataToAP();
}

/** 
	Aggregates data 
	@return pair of <aggregated_filename, aggregated_filesize>

*/

std::pair<std::string, long unsigned int>  DCA_DATACOLLECTION::AggregateData() {
    DIR *dir;
    struct dirent *ent;
    std::string Data;
    if ((dir = opendir (CHReceivedDataFilePath.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if(strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                std::string Name(ent->d_name);
                if (Name.find("part") == std::string::npos && Name.find("[R"+VID+"]") != std::string::npos) {
                    std::ifstream ifs(CHReceivedDataFilePath + ent->d_name);
                    std::string Content((std::istreambuf_iterator<char>(ifs)),
                                        (std::istreambuf_iterator<char>()));
                    Data.append(Content);
                    ifs.close();
                    Content.clear();
                    remove((CHReceivedDataFilePath+ent->d_name).c_str());
                }
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
        LogsObj->LogError("Error opening file " + CHReceivedDataFilePath+ent->d_name);
    }
    if(IsAggregationReq) {
        LogsObj->LogDebug("Requires data aggregation of percentage " + std::to_string(AggregationPercentage));
        Data.resize((AggregationPercentage / 100.0) * Data.size());
        LogsObj->LogDebug("After data aggregation the size = " + std::to_string(Data.size()));
    }
    if(Data.size() != 0) {
        std::string Filename = "[CH"+VID+"][" + VID + "][" + std::to_string(CHSequenceNum) + "][" + LogsObj->GetCurrentTime() + "]";
        std::ofstream Out(CHGeneratedDataFilePath + Filename);
        Out << Data;
        Out.close();
        LogsObj->LogDebug("Total Number of packets sent from CH " + VID + " to AP = " + std::to_string(CHSequenceNum));
        CHSequenceNum++;
        return std::make_pair(Filename, Data.size());
    }
    return std::make_pair("", Data.size());
}

/**
	Periodically sends data to Accesspoint 

*/

void DCA_DATACOLLECTION::SendDataToAP() {
    useconds_t Interval = (useconds_t )APDataUpdateInterval * 1000000;
    while(CarExists) {
        if(DCAClusterObj->AM_I_CH) {
            std::pair<std::string, long unsigned int> FileInfo = AggregateData();
            if(!FileInfo.first.empty()) {
                SplitDataFile(CHGeneratedDataFilePath, FileInfo.first);
                TransferDataToAP(FileInfo.first, FileInfo.second);
            }
            usleep(Interval);
            continue;
        }
        sleep(1);
    }

	/* This is a case when Cluster Head exits in the last timestep of Mobility Trace, transfer data to Accesspoint
   	   If it is not the last timestep, No car has any data to send to Accesspoint */		
    std::pair<std::string, long unsigned int> FileInfo = AggregateData();
    if(!FileInfo.first.empty()) {
        SplitDataFile(CHGeneratedDataFilePath, FileInfo.first);
        TransferDataToAP(FileInfo.first, FileInfo.second);
    }
	
}

/**
	Transfers data to AP through unicast messages
	@param Filename Name of the file to be transferred to Accesspoint
	@param TotalDataSize Size of the file in bytes
*/

void DCA_DATACOLLECTION::TransferDataToAP(std::string Filename, long unsigned int TotalDataSize) {
    long unsigned int TotalTransfers = (long unsigned int)(((double)TotalDataSize) / 1024.0) / ChunkSize;
    int Count = 1;
    if( TotalDataSize % (ChunkSize * 1024) != 0 )
        TotalTransfers += 1;
    COMMUNICATION *CommObj = new COMMUNICATION("DEFAULT", LogsObj);
    DATA DATAPACKET;
    DATAPACKET.Timestamp = LogsObj->GetCurrentTime();

    DATAPACKET.Dataline1 = "Hello, This is a DATA Packet UNICAST message";
    DATAPACKET.Dataline2 = "From " + VID;
    DATAPACKET.SrcID = VID;
    DATAPACKET.Datatype = "CHDATA";
    DATAPACKET.Filename = Filename;
    DATAPACKET.ClusterSeqNum = StatsObj->ClusterSequenceNumber;
    DATAPACKET.ClusterID = DCAClusterObj->MC_ID;

    while(Count <= TotalTransfers) {
        if(Count == TotalTransfers)
            DATAPACKET.LastPkt = true;
        else
            DATAPACKET.LastPkt = false;
        DATAPACKET.ChunkNum = Count;
		/* Block of code to measure delays in messages 
		Since no time synchronization between the systems cannot be rely upon completely */
            struct timeval tv;

            gettimeofday(&tv, NULL);

            unsigned long long millisecondsSinceEpoch =
                    (unsigned long long) (tv.tv_sec) * 1000 +
                    (unsigned long long) (tv.tv_usec) / 1000;
            DATAPACKET.StartTime = millisecondsSinceEpoch;

        std::string SendFileName = CHGeneratedDataFilePath+Filename+std::to_string(Count)+".part";
        std::ifstream ifs(SendFileName);
        std::string Content( (std::istreambuf_iterator<char>(ifs) ),
                             (std::istreambuf_iterator<char>()    ) );
        DATAPACKET.Data = Content;
        CommObj->SendUnicastMsg(VID, this->DCAClusterObj->ClusterManager, &DATAPACKET, (char*)"A", "CHDATA");
	
	/* Logs total amount of data sent to Accespoint */
        StatsObj->TotalDataSentToAP += Content.size();
        LogsObj->LogDebug("Data aggregated and sent to " + this->DCAClusterObj->ClusterManager);
        LogsObj->LogInfo("Data aggregated and sent to " + this->DCAClusterObj->ClusterManager);

	/* Once the data is sent, that file is removed from the directory */
        std::string RemoveFile = CHGeneratedDataFilePath+Filename+std::to_string(Count)+".part";
        remove(RemoveFile.c_str());
        Count++;
        ifs.close();
    }
    LogsObj->LogStats("Total Data Sent to AP = " + std::to_string(StatsObj->TotalDataSentToAP));
    delete CommObj;
}

/**
	Function is called when Data Collection and aggregation module receives message 
	@param NewMessage The new message that is received 
*/

void DCA_DATACOLLECTION::ProcessReceivedMsg(std::string NewMessage) {
    MESSAGEFROMUDM Message;
    Message.ParseFromString(NewMessage);
    std::string DataLine1, DataLine2, DataType, SourceType;
    SourceType = Message.src_type();
    /*! Checks the source type and data type and process it accordingly */
    ALLDATA DataString = Message.data();
    DataLine1 = DataString.dataline1();
    DataLine2 = DataString.dataline2();
    DataType = DataString.datatype();
    if(SourceType == "CAR") {
        if(DataType == "DATA") {
		/* Some stats collected */
            if (StatsObj->ClusterSequenceNumber >= 10) {
                    StatsObj->TotalDataReceivedCHInterval += DataString.data().size();
                    StatsObj->TotalDataReceivedFromCM += DataString.data().size();
            }
            LogsObj->LogStats("Total Data Received from CM = " + std::to_string(StatsObj->TotalDataReceivedFromCM));
            std::string Filepath = CHReceivedDataFilePath + "[R"+VID+"]" + DataString.filename() + std::to_string(DataString.chunknum())+".part";
		
		/* Save data into a file fragment */
            std::ofstream Out(Filepath);
            Out << DataString.data();
            Out.close();

		/* If it is a last packet, re-assemble all other file fragments */
            if(DataString.lastpkt()) {
                LogsObj->LogInfo("Data Received from cluster member " + DataString.id());
                LogsObj->LogDebug("Data Received from cluster member " + DataString.id());

                CkFileAccess fac;
                std::string reassembledFilename = CHReceivedDataFilePath + "[R"+VID+"]" + DataString.filename();
                std::string prefix = "[R"+VID+"]" + DataString.filename();
                const char *partPrefix = prefix.c_str();
                const char *partExtension = "part";

                int Success = fac.ReassembleFile(CHReceivedDataFilePath.c_str(),partPrefix,partExtension,reassembledFilename.c_str());
                if (Success) {
                }
                else {
                    LogsObj->LogError(fac.lastErrorText());
                }

		/* After file is re assembled remove the file fragments from the directory */
                int Count = 1;
                while(Count <= DataString.chunknum()) {
                    std::string RemoveFile = CHReceivedDataFilePath + "[R"+VID+"]" + DataString.filename() + std::to_string(Count) + ".part";
                    remove(RemoveFile.c_str());
                    Count++;
                }
            }

        }
    }
}


/**
	Transfers data to Cluster Head through unicast messages
	@param Filename Name of the file to be transferred to Accesspoint
*/
void DCA_DATACOLLECTION::TransferDataTOCH(std::string Filename) {
    int TotalTransfers = DataSize / ChunkSize;
    if (DataSize % (ChunkSize * 1024) != 0)
        TotalTransfers += 1;
    int Count = 1;
    COMMUNICATION *CommObj = new COMMUNICATION("DEFAULT", LogsObj);

	/* Construct a data packet */
    DATA DATAPACKET;
    DATAPACKET.Dataline1 = "Hello, This is a DATA Packet UNICAST message";
    DATAPACKET.Dataline2 = "From " + VID;
    DATAPACKET.SrcID = VID;
    DATAPACKET.Datatype = "DATA";
    DATAPACKET.Filename = Filename;
    DATAPACKET.ClusterID = DCAClusterObj->MC_ID;

    while(Count <= TotalTransfers) {
        DATAPACKET.Timestamp = LogsObj->GetCurrentTime();
        if(Count == TotalTransfers)
            DATAPACKET.LastPkt = true;
        else
            DATAPACKET.LastPkt = false;
        DATAPACKET.ChunkNum = Count;

        std::string FilePath = DataFilePath+Filename+std::to_string(Count)+".part";
        std::ifstream ifs(DataFilePath+Filename+std::to_string(Count)+".part");
        std::string Content( (std::istreambuf_iterator<char>(ifs) ),
                             (std::istreambuf_iterator<char>()    ) );
        DATAPACKET.Data = Content;
	if (StatsObj->ClusterSequenceNumber >= 10)
	    StatsObj->TotalDataSentToCH += Content.size();
        CommObj->SendUnicastMsg(VID, this->DCAClusterObj->CH, &DATAPACKET, (char *) "A", "DATA");

	/* After data from file fragment is sent, remove the file from directory */
        std::string RemoveFile = DataFilePath+Filename+std::to_string(Count)+".part";
        remove(RemoveFile.c_str());
        Count++;
        ifs.close();
    }
    LogsObj->LogStats("Total Data Sent to CH = " + std::to_string(StatsObj->TotalDataSentToCH));
    LogsObj->LogInfo("Data Sent to CH " + this->DCAClusterObj->CH);
    LogsObj->LogDebug("Data Sent to CH " + this->DCAClusterObj->CH);

    delete CommObj;
}

/**
	Periodically Cluster Member sends data to Cluster Head
*/

void DCA_DATACOLLECTION::SendDataToCH() {
    useconds_t Interval = (useconds_t )GetCMDataUpdateInterval() * 1000000;
    while(CarExists) {
        if(DCAClusterObj->ClusterState != "INIT") {
		/* Get generated data */
            std::string Filename = CollectDataFromApp();
            if(!DCAClusterObj->AM_I_CH) {
		/* Send data generated to Cluster Head if the car is in Cluster Member state */
                TransferDataTOCH(Filename);
		/* Once the file data is sent remove the file from the directory */
                std::string RemoveFile = DataFilePath + Filename;
                remove(RemoveFile.c_str());
            }
            usleep(Interval);
            continue;
        }
        sleep(1);
    }
}

/**
	Generates data for Cluster Member to send to Cluster Head 
*/

std::string DCA_DATACOLLECTION::GenerateData(bool AM_I_CH) {
    std::string Filename ="[" + VID + "][" + std::to_string(CMSequenceNum) + "][" + LogsObj->GetCurrentTime()\
    +"]";
    std::string Path;
    if (AM_I_CH) {
        Path = CHReceivedDataFilePath;
        Filename = "[R"+VID+"]" + Filename;
    } else
        Path = DataFilePath;
	std::ofstream ofs(Path+Filename, std::ios::binary | std::ios::out);
    ofs.seekp((DataSize*1024) - 1);
    ofs.write("", 1);
    CMSequenceNum++;
    return Filename;
}

/**
	Splits file into fragment files
	@param Filename Name of the file that needs to be split into file fragments 
	@param Path Path where Filename file exists
*/

void DCA_DATACOLLECTION::SplitDataFile(std::string Path, std::string Filename) {
    Filename = Path + Filename;
    int TempChunkSize = ChunkSize * 1024;
    CkFileAccess fac;
    const char *partPrefix = Filename.c_str();
    const char *partExtension = "part";
    bool success = fac.SplitFile(Filename.c_str(),partPrefix,partExtension,TempChunkSize,Path.c_str());

    if (!success) {
        std::cout << fac.lastErrorText() << "\r\n";
        LogsObj->LogError(fac.lastErrorText());
    }
}

/** 
	Collect data (call to generate data) which will be sent to Cluster Head
*/

std::string DCA_DATACOLLECTION::CollectDataFromApp() {
    std::string Filename;
    bool AM_I_CH = DCAClusterObj->AM_I_CH;
    if(AM_I_CH) {
	/* Stats of amount of data that a Cluster Head generates (Not Cluster Member) */
        if (StatsObj->ClusterSequenceNumber >= 10) {
                StatsObj->TotalDataCHGenerated += DataSize * 1024;
                LogsObj->LogStats("Total Data I generated = " + std::to_string(StatsObj->TotalDataCHGenerated));
        }
    }
	/* Generates data in both Cluster Head and Cluster Member */
    Filename = GenerateData(AM_I_CH);

	/* Splits file only if the car is Cluster Member (because only Cluster Members needs to send data to Cluster Head) */
    if(!AM_I_CH) {
        SplitDataFile(DataFilePath, Filename);
    }
    return Filename;
}

void T_CollectDataFromCM(DCA_BASE_DATACOLLECTION *DCAObj) {
    DCAObj->CollectDataFromCM();
}



/** 
	Receiver created in Communication module to receive new messages
*/

void DCA_DATACOLLECTION::CollectDataFromCM() {
    CommObj->ReceiveMessage((void*)this, VID + "DCA", (char*)"A", "DCA");
}



/**	
	Thread that receives Car exit status 
*/

void T_CarExitStatusUpdate(DCA_DATACOLLECTION *DCAObj) {
    zmq::message_t Address, MessageReceived;
    zmq::context_t *Context;
    zmq::socket_t *Subscriber;
    COMMUNICATION *CommObj;
    CommObj = new COMMUNICATION(DCAObj->LogsObj);

    if(true) {
        Context = new zmq::context_t(1);
        Subscriber = new zmq::socket_t(*Context, ZMQ_SUB);
        Subscriber->connect("tcp://" + CommObj->GetParserIP() + ":" + (std::to_string(std::stoi(CommObj->GetUDMPort()) + 3)));
        std::string ID = DCAObj->GetVehicleID() + "DCAEXITID";
        Subscriber->setsockopt(ZMQ_SUBSCRIBE, ID.c_str(), std::strlen(ID.c_str()));
        Subscriber->recv(&Address);
        Subscriber->recv(&MessageReceived);
        CommObj->KillReceiverSocket();
        ClusterInfoCollectorSubscriber->close();
        ClusterInfoCollectorContext->close();
        DCAObj->CarExists = false;
    }

    delete CommObj;
    Subscriber->close();
    Context->close();
}

void T_SendDataCountToVisuals(DCA_DATACOLLECTION *DCAObj) {
    DCAObj->MessageToVisuals();
}


/** 
	Sends Data Collection and Data Aggregation application details to Visual Component periodically 
*/

void DCA_DATACOLLECTION::MessageToVisuals() {
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_PUSH);

    socket.connect("tcp://"+VisualsIPInfo->VisualsIP+":"+VisualsIPInfo->VisualsPort);

    while(CarExists) {
        VISUALSDATA DataCountInfo;
        std::string MCDataCount = "";
            DataCountInfo.set_messagetype("DATA");
            DataCountInfo.set_datacount(StatsObj->TotalDataSentToAP);
            DataCountInfo.set_srcid(GetVehicleID());
            DataCountInfo.set_srctype("CAR");
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

void DCA_DATACOLLECTION::DataCollectionService() {
    LogsObj->LogDebug("Starting 'DCA Send Data to CH' thread");
    LogsObj->LogInfo("Starting 'DCA Send Data to CH' thread");
    std::thread SendDataToCH(T_SendDataToCH, this);

    LogsObj->LogDebug("Starting 'DCA Send Data to CH' thread");
    LogsObj->LogInfo("Starting 'DCA Send Data to CH' thread");
    std::thread SendDataToAP(T_SendDataToAP, this);

    LogsObj->LogDebug("Starting 'DCA Collect Data from CM' thread");
    LogsObj->LogInfo("Starting 'DCA Collect Data from CM' thread");
    std::thread CollectDataFromCM(T_CollectDataFromCM, this);

    std::thread SendDataCountToVisuals(T_SendDataCountToVisuals, this);

    std::thread CarExitStatusUpdate(T_CarExitStatusUpdate, this);

    CollectClusterInfo();
    CarExitStatusUpdate.join();
    SendDataToCH.join();
    SendDataToAP.join();
    CollectDataFromCM.join();
}

/** 
	Subscribes to Clustering service and receives cluster status updates 
*/
void DCA_DATACOLLECTION::CollectClusterInfo() {
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
        if(DCAClusterObj->ClusterState != "INIT" && DCAClusterObj->ClusterState != "CM") {
            LogsObj->LogStats(
                    "CH belongs to " + DCAClusterObj->MC_ID + " and Data Received during previous interval = " +
                    std::to_string(StatsObj->TotalDataReceivedCHInterval));
            StatsObj->TotalDataReceivedCHInterval = 0;
        }
        if(RequestBuffer.empty()) {
            CarExists = false;
            LogsObj->LogDebug("CAR Exited from the network ");
            LogsObj->LogInfo("CAR Exited from the network ");
            break;
        }

        ResponseMsg.ParseFromString(RequestBuffer);
        LogsObj->LogDebug("Received Cluster State from Car Cluster Service State = " + DCAClusterObj->ClusterState);
        LogsObj->LogInfo("Received Cluster State from Car Cluster Service State = " + DCAClusterObj->ClusterState);
        DCAClusterObj->DefaultClusterManager = ResponseMsg.default_manager();

        if(ResponseMsg.cluster_state() != "INIT") {
            DCAClusterObj->CMList.clear();
            DCAClusterObj->AM_I_CH = ResponseMsg.is_cluster_head();
            if(!DCAClusterObj->AM_I_CH)
                DCAClusterObj->CH = ResponseMsg.ch();
            else {
                DCAClusterObj->CH = VID;
                DCAClusterObj->ClusterManager = ResponseMsg.cluster_manager();
                DCAClusterObj->MC_ID = ResponseMsg.cluster_id();
            }
            LogsObj->LogDebug("Cluster Head = " + DCAClusterObj->CH);
            std::string ListOfCM = ResponseMsg.cm_list();
            size_t pos = 0;
            std::string token, delimiter = " ";
            while ((pos = ListOfCM.find(delimiter)) != std::string::npos) {
                token = ListOfCM.substr(0, pos);
                DCAClusterObj->CMList.push_front(token);
                ListOfCM.erase(0, pos + delimiter.length());
            }
            for(auto it = DCAClusterObj->CMList.begin(); it != DCAClusterObj->CMList.end(); it++) {
                LogsObj->LogDebug("Cluster Member " + *it);
            }
        }

	/* When the cluster state of a car changes to "CM" or "INIT", it uploads any residual data left with it to AP */
	if ((ResponseMsg.cluster_state() == "INIT") || (ResponseMsg.cluster_state() == "CM")) {
            std::pair<std::string, long unsigned int> FileInfo = AggregateData();
            if(!FileInfo.first.empty()) {
                SplitDataFile(CHGeneratedDataFilePath, FileInfo.first);
                TransferDataToAP(FileInfo.first, FileInfo.second);
            }
        }
        DCAClusterObj->ClusterState = ResponseMsg.cluster_state();
	StatsObj->ClusterSequenceNumber = ResponseMsg.cluster_seq_num();
    }
}
