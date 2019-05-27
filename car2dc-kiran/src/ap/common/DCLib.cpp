//
// Created by kiran.narayanaswamy on 11/30/18.
//
#include "../../header/ap/DC.h"
unsigned int DeserializationCount = 0;
float CumulativeDeserializeTime = 0.0;


/**
	Data collection application is listening to cluster change status notification from clustering service module
	Currently it is used to write stats of data collected at each interval
*/

void T_ListenToAP(AP_DATA_COLLECTION*DCObj){
    auto ClusterInfoCollectorContext = new zmq::context_t(1);
    auto ClusterInfoCollectorSubscriber = new zmq::socket_t(*ClusterInfoCollectorContext, ZMQ_SUB);
    ClusterInfoCollectorSubscriber->connect("tcp://" + DCObj->IP_Port_Info->IP +":" +std::to_string( std::stoi(DCObj->IP_Port_Info->Port) + 3));
    ClusterInfoCollectorSubscriber->setsockopt( ZMQ_SUBSCRIBE, "AP_TO_APPS", 10);
    APtoApps PublishedMsg;
    zmq::message_t MessageReceived;
    zmq::message_t Address;
    while (DCObj->APExists) {
        try {
            ClusterInfoCollectorSubscriber->recv(&Address);
            ClusterInfoCollectorSubscriber->recv(&MessageReceived);
        } catch(zmq::error_t& e) {
            std::cout << "Exception raised\n";
            break;
        }
        std::string RequestBuffer(static_cast<char *>(MessageReceived.data()), MessageReceived.size());
        PublishedMsg.ParseFromString(RequestBuffer);
        DCObj->StatsObj->SetPublishClusterInfo(PublishedMsg.clusterchange());
	/* Write to a file amount of Data collected at previous interval */
        std::ofstream outputFile;
        outputFile.open("../results/logs/DataCollectedAtEveryInterval.csv", std::fstream::app);
        outputFile << DCObj->StatsObj->ClusterSequenceNumber++ << "," << DCObj->StatsObj->DataCollectedAtMCAtEachInterval << std::endl;
        outputFile.close();
	/* Set to zero after each cluster status change notification*/
        DCObj->StatsObj->DataCollectedAtMCAtEachInterval = 0;
        sleep(1);
    }
}

/** 
        Constructor that reads XML Config file to populate required variables 
*/

AP_DATA_COLLECTION::AP_DATA_COLLECTION(std::string _ID) {
    ID = _ID;
    pugi::xml_document doc;
    if (!doc.load_file("../config/dca/config.xml")) {
        LogsObj->LogError("ERROR:: Config File Missing ../config/apps/config.xml !!\n");
    } else {
        pugi::xml_node tools = doc.child("VMCPI").child("config");
        tools = doc.child("VMCPI").child("config");
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                if(name == "APDataFilePath")
                    APDataFilePath = ait->value();
            }
        }
    }

    auto _VisualsIPInfo = new struct Visuals_IP_Info();
    VisualsIPInfo = _VisualsIPInfo;

    pugi::xml_node tools = doc.child("VMCPI").child("config");
    if (!doc.load_file("../config/visuals/config.xml")) {
        std::cout << "ERROR:: Config File Missing ../config/visuals/config.xml !!\n";
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

    APExists = true;
}

void T_SendDataCountToVisuals(AP_DATA_COLLECTION *APDC) {
    APDC->MessageToVisuals();
}

/** 
	Periodically send information (Total data so far collected at micro cloud) to visual component
	
*/

void AP_DATA_COLLECTION::MessageToVisuals() {
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_PUSH);

    socket.connect("tcp://"+VisualsIPInfo->VisualsIP+":"+VisualsIPInfo->VisualsPort);

    while(APExists) {
        VISUALSDATA DataCountInfo;
        std::string MCDataCount = "";
        for (auto it = StatsObj->DataCollectedAtMC.begin(); it != StatsObj->DataCollectedAtMC.end(); it++) {
            DataCountInfo.set_messagetype("DATA");
            DataCountInfo.set_datacount(it->second);
            DataCountInfo.set_srcid(it->first);
            DataCountInfo.set_srctype("AP");
            std::string DataToSend;
            DataCountInfo.SerializeToString(&DataToSend);

            zmq::message_t Packet(DataToSend.size());
            memcpy ((void*)Packet.data(), (void*)DataToSend.data(), DataToSend.length());
            socket.send(Packet);
        }
        sleep(4);
    }

}

void AP_DATA_COLLECTION::DataCollectionService() {
    std::thread SendDataCountToVisuals(T_SendDataCountToVisuals, this);

    /* Receiver created in Communication module to receive new messages */
    this->CommObj->ReceiveMessage((void*)this, ID + "DC", (char*)"C", "AP_DC");
}

void AP_DATA_COLLECTION::ProcessMessage(std::string MessageReceived) {
    MESSAGEFROMUDM Message;
    clock_t t = clock();
    Message.ParseFromString(MessageReceived);
    ALLDATA DataString = Message.data();

	/* Stats to find out deserialization time */
    t = clock() - t;
    CumulativeDeserializeTime += (float)t/CLOCKS_PER_SEC;
    DeserializationCount += 1;
    //LogsObj->LogStats("Average time taken to deserialize = " + std::to_string(CumulativeDeserializeTime/(float)DeserializationCount));

    std::string DataLine1, DataLine2, DataType, _ID, _Lane, SourceType;
    SourceType = Message.src_type();

    DataLine1 = DataString.dataline1();
    DataLine2 = DataString.dataline2();
    DataType = DataString.datatype();

    /*! Checks the source type and data type and process it accordingly */
    if(SourceType == "CAR") {
        if (DataType == "CLUSTERINFO") {
        } else if(DataType == "CHDATA") {
            LogsObj->LogDebug("Received Data from " + Message.src_id());
            LogsObj->LogInfo("Received Data from " + Message.src_id());
            StoreData(&DataString);
        }
    } else if(SourceType == "RSU") {

    } else if(SourceType == "BS") {

    }
    LogsObj->LogDebug(std::string("Messaged received is from source type " + Message.src_type()));
    LogsObj->LogInfo(std::string("Recieved msg:: ") + DataLine1 + " " + DataLine2 + std::string(" And this message was sent by sender at ") + DataString.timestamp());
    LogsObj->LogDebug(std::string("Recieved msg:: ") + DataLine1 + " " + DataLine2 + std::string(" And this message was sent by sender at ") + DataString.timestamp());
}

/** 
	Store data sent by Cluster Heads
*/

void AP_DATA_COLLECTION::StoreData(ALLDATA *DataString) {
    unsigned long long Start;
    StatsObj->DataCollectedAtMC[DataString->clusterid()] += DataString->data().size();
    //if(DataString->cluster_seq_num() >= 10) {
        StatsObj->TotalDataReceived += DataString->data().size();
        LogsObj->LogStats("Total Data Received = " + std::to_string(StatsObj->TotalDataReceived));
    //}
    StatsObj->DataCollectedAtMCAtEachInterval += DataString->data().size();
    std::string OutputFilename = APDataFilePath + DataString->filename() + std::to_string(DataString->chunknum())+".part";
    std::ofstream Out(OutputFilename);
    std::string Data = DataString->data();
    Out << Data;
    Out.close();
	/* When the last fragment of data is arrived then reassemble all the fragments into original file */
    if(DataString->lastpkt()) {
	/* Measures delay, but cannot rely on it as system time is not synchronised between the systems */
	if(DataString->chunknum() > 200) {
        	struct timeval tv;
        	gettimeofday(&tv, NULL);
        	unsigned long long millisecondsSinceEpoch =
                	(unsigned long long)(tv.tv_sec) * 1000 +
                	(unsigned long long)(tv.tv_usec) / 1000;
        	unsigned long long End = millisecondsSinceEpoch;
		    Start = std::stoull(DataString->start_time());
        	auto ElapsedTime = End - Start;
        	LogsObj->LogStats("Time taken for last data fragment to travel " + std::to_string(ElapsedTime));
	}
	
        CkFileAccess fac;
	/* Reassemble fragments */
        std::string reassembledFilename = APDataFilePath + DataString->filename();
        const char *partPrefix = DataString->filename().c_str();
        const char *partExtension = "part";

        int Success = fac.ReassembleFile(APDataFilePath.c_str(),partPrefix,partExtension,reassembledFilename.c_str());
        if (Success) {
            std::cout << "Success.................................................... OK AP" << "\r\n";
        }
        else {
        }
        int Count = 1;

	/*After fragments are reassembled the fragments are removed from the directory */
        while(Count <= DataString->chunknum()) {
            std::string RemoveFile = APDataFilePath + DataString->filename() + std::to_string(Count) + ".part";
            remove(RemoveFile.c_str());
            Count++;
        }
    }
}

