//
// Created by kiran.narayanaswamy on 8/15/18.
//

#include "Communication.h"
#include "../../lib/pugixml/pugixml.hpp"
#include "../header/car/CarHeader.h"
#include "../header/ap/AP.h"
#include "../header/apps/DCA.h"
#include "../header/ap/DC.h"
#include "../header/apps/TaskD.h"
#include "Stats.h"


class LOGGING_UTILITY;

/* Open sockets used to send different types of messages */
zmq::socket_t* SendHelloPacketSocket;
zmq::socket_t* SendLocalInfoPacketSocket;
zmq::socket_t* RSUBeaconSocket;
zmq::socket_t* ClusterInfoSocket;
zmq::socket_t* TransferDataToCHSocket;
zmq::socket_t* TransferDataToAPSocket;
zmq::socket_t* DistributeTasksToCMSocket;
zmq::socket_t* SendResultsToCHSocket;


/** 
        Constructor that reads XML Config file to populate required variables 
*/

COMMUNICATION::COMMUNICATION(LOGGING_UTILITY *_LogsObj) {
    LogsObj = _LogsObj;
    pugi::xml_document doc;
    pugi::xml_node tools = doc.child("VMCPI").child("config");
    if (!doc.load_file("../config/common/config.xml")) {
        std::cout << "ERROR:: Config File Missing ../config/common/config.xml !!\n";
    }
    else {
        tools = doc.child("VMCPI").child("config");

        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                if (name == "ParserPort")
                    ParserPort = ait->value();
                else if (name == "ParserIP")
                    ParserIP = ait->value();
                else if (name == "UDMPort")
                    UDMPort = ait->value();
            }
        }
    }
    LogsObj->LogDebug("CarServerIP = " + CarServerIP + " ParserIP = " + ParserIP + " ParserPort = " + ParserPort + " UDMPort = " + UDMPort);
}

/* Used to measure average serialization time */
/* SerializationCount - number of times serialization performed */
unsigned int SerializationCount = 0;
/* CumulativeSerializeTime - Total time spent on serialization */
float CumulativeSerializeTime = 0.0;


/*!
 * Compose and sends Unicast Message
 * @param Dest_id Destination ID
 * @param data 'data' is data needs to be sent through protobuf
 */

void COMMUNICATION::SendUnicastMsg(std::string SrcID, std::string DestID, void* Data, char* PacketTypeRef...) {
    va_list args;
    va_start(args, PacketTypeRef);
    std::string PacketType = va_arg(args, char *);
    std::string DataToSend;

    ALLDATA *InfoPkt = new ALLDATA;

    if(PacketType == "LOCALINFO") {
        UPDATE_LOCALINFO *LocalInfo;
        LocalInfo = (UPDATE_LOCALINFO*)Data;
        InfoPkt->set_dataline1(LocalInfo->Dataline1);
        InfoPkt->set_dataline2(LocalInfo->Dataline2);
        InfoPkt->set_id(LocalInfo->SrcID);
        InfoPkt->set_x(LocalInfo->X);
        InfoPkt->set_y(LocalInfo->Y);
        InfoPkt->set_dir(LocalInfo->Direction);
        InfoPkt->set_speed(LocalInfo->Speed);
        InfoPkt->set_timestamp(LocalInfo->Timestamp);
        InfoPkt->set_datatype(LocalInfo->Datatype);
    } else if(PacketType == "CLUSTERINFO") {
        CLUSTER_INFO *ClusterInfo;
        ClusterInfo = (CLUSTER_INFO *)Data;
        InfoPkt->set_dataline1(ClusterInfo->Dataline1);
        InfoPkt->set_dataline2(ClusterInfo->Dataline2);
        InfoPkt->set_ch(ClusterInfo->CH);
        InfoPkt->set_cmlist(ClusterInfo->ListOfCM);
        InfoPkt->set_clusterid(ClusterInfo->MC_ID);
        InfoPkt->set_datatype(ClusterInfo->Datatype);
        InfoPkt->set_timestamp(ClusterInfo->Timestamp);
	    InfoPkt->set_cluster_seq_num(ClusterInfo->ClusterSeqNum);
    } else if(PacketType == "DATA" || PacketType == "CHDATA") {
        DATA *DataInfo;
        DataInfo = (DATA *)Data;
        InfoPkt->set_dataline1(DataInfo->Dataline1);
        InfoPkt->set_dataline2(DataInfo->Dataline2);
        InfoPkt->set_timestamp(DataInfo->Timestamp);
        InfoPkt->set_datatype(DataInfo->Datatype);
        InfoPkt->set_data(DataInfo->Data);
        InfoPkt->set_chunknum(DataInfo->ChunkNum);
        InfoPkt->set_filename(DataInfo->Filename);
        InfoPkt->set_lastpkt(DataInfo->LastPkt);
        InfoPkt->set_clusterid(DataInfo->ClusterID);
	    InfoPkt->set_cluster_seq_num(DataInfo->ClusterSeqNum);
        InfoPkt->set_start_time(std::to_string(DataInfo->StartTime));
    } else if(PacketType == "TASK_ASSIGN") {
        TASK *TaskInfo;
        TaskInfo = (TASK*) Data;
        InfoPkt->set_datatype(TaskInfo->DatatType);
        InfoPkt->set_id(TaskInfo->SrcID);
        InfoPkt->set_timestamp(TaskInfo->Timestamp);
        InfoPkt->set_taskseqnum(TaskInfo->TaskSeqNum);
        InfoPkt->set_taskmaxtime(TaskInfo->MaxTime);
    } else if(PacketType == "TASK_RESULTS") {
        TASK_RESULTS *TaskResultsInfo;
        TaskResultsInfo = (TASK_RESULTS*) Data;
        InfoPkt->set_datatype(TaskResultsInfo->DatatType);
        InfoPkt->set_id(TaskResultsInfo->SrcID);
        InfoPkt->set_timestamp(TaskResultsInfo->Timestamp);
        InfoPkt->set_finishtime(TaskResultsInfo->FinishTime);
        InfoPkt->set_result(TaskResultsInfo->Result);
    }

    MSGFROMNODETOUDM UnicastPkt;
    UnicastPkt.set_mtype("UNI");
    UnicastPkt.set_src_id(SrcID);
    UnicastPkt.set_dest_id(DestID);
    if(PacketType == "DATA")
        UnicastPkt.set_sub_dest_id(DestID+"DCA");
    else if(PacketType == "CHDATA")
        UnicastPkt.set_sub_dest_id(DestID+"DC");
    else if(PacketType == "TASK_ASSIGN" || PacketType == "TASK_RESULTS")
        UnicastPkt.set_sub_dest_id(DestID + "TASK");
    UnicastPkt.set_allocated_data(InfoPkt);

	/* Code snippet used to measure average serialization time, uncomment when required */
    /*clock_t t;
    if(PacketType == "CHDATA") {
	t = clock();
    }	*/
    UnicastPkt.SerializeToString(&DataToSend);
    /*if(PacketType == "CHDATA") {
	t = clock() - t;
	CumulativeSerializeTime += (float)t/CLOCKS_PER_SEC;
   	SerializationCount += 1;
    	LogsObj->LogStats("Average time taken to deserialize = " + std::to_string(CumulativeSerializeTime/(float)SerializationCount));
    }*/

    UnicastPkt.release_data();

    LogsObj->LogDebug("Sending Unicast Message from " + SrcID +" to " + DestID + " ...");

    zmq::message_t Packet(DataToSend.size());
    memcpy ((void*)Packet.data(), (void*)DataToSend.data(), DataToSend.length());

    if(PacketType == "LOCALINFO") {
        SendLocalInfoPacketSocket->send(Packet);
    } else if (PacketType == "CLUSTERINFO") {
        ClusterInfoSocket->send(Packet);
    } else if(PacketType == "DATA") {
        TransferDataToCHSocket->send(Packet);
    } else if(PacketType == "CHDATA") {
        TransferDataToAPSocket->send(Packet);
    } else if(PacketType == "TASK_ASSIGN") {
        DistributeTasksToCMSocket->send(Packet);
    } else if(PacketType == "TASK_RESULTS") {
        SendResultsToCHSocket->send(Packet);
    }
    va_end(args);
    delete InfoPkt;
}

/*!
 * Compose and Sends broadcast message
 * @param data 'data is data needs to be sent through protobuf
 */

void COMMUNICATION::SendBroadcastMsg(std::string SrcID, void* Data, char* PacketTypeRef...) {
    va_list args;
    va_start(args, PacketTypeRef);
    std::string PacketType = va_arg(args, char*);
    std::string DataToSend;
    ALLDATA *HelloPacket = new ALLDATA;
    HELLO_PACKET *HelloPkt;
    RSUBEACON *RSUBeaconPkt;

    if(PacketType == "HELLO") {
        HelloPkt = (HELLO_PACKET*) Data;
        HelloPacket->set_dataline1(HelloPkt->Dataline1);
        HelloPacket->set_dataline2(HelloPkt->Dataline2);
        HelloPacket->set_id(HelloPkt->SrcID);
        HelloPacket->set_x(HelloPkt->X);
        HelloPacket->set_y(HelloPkt->Y);
        HelloPacket->set_speed(HelloPkt->Speed);
        HelloPacket->set_dir(HelloPkt->Direction);
        HelloPacket->set_timestamp(HelloPkt->Timestamp);
        HelloPacket->set_datatype(HelloPkt->Datatype);
    } else if (PacketType == "RSUBEACON") {
        RSUBeaconPkt = (RSUBEACON *)Data;
        HelloPacket->set_dataline1(RSUBeaconPkt->Dataline1);
        HelloPacket->set_dataline2(RSUBeaconPkt->Dataline2);
        HelloPacket->set_id(RSUBeaconPkt->SrcID);
        HelloPacket->set_datatype(RSUBeaconPkt->Datatype);
        HelloPacket->set_x(RSUBeaconPkt->X);
        HelloPacket->set_y(RSUBeaconPkt->Y);
    }


    MSGFROMNODETOUDM BroadcastPkt;
    BroadcastPkt.set_mtype("BROAD");
    BroadcastPkt.set_src_id(SrcID);

    BroadcastPkt.set_allocated_data(HelloPacket);
    BroadcastPkt.SerializeToString(&DataToSend);
    BroadcastPkt.release_data();

    LogsObj->LogDebug("Sending Broadcast Message ...");

    zmq::message_t Packet(DataToSend.size());
    memcpy ((void*)Packet.data(), DataToSend.c_str(), DataToSend.size());

    if(PacketType == "HELLO") {
        SendHelloPacketSocket->send(Packet);
    } else if (PacketType == "RSUBEACON") {
        RSUBeaconSocket->send(Packet);
    }
    va_end(args);
    delete HelloPacket;
}

/* On setting to true, worker threads will exit */
bool WorkerThreadExit = false;
AP_DATA_COLLECTION *DCObj;
ACCESSPOINT *APObj;

/** 
	Routine or procedure executed by worker threads 
*/

void *WorkerRoutine (void *arg)
{
    struct Worker_Routine *WRObj = (struct Worker_Routine *)arg;
    auto *context = WRObj->WR_Context;
    zmq::socket_t socket (*context, ZMQ_PULL);
    socket.connect ("inproc://workers");
    while (!WorkerThreadExit) {
        //  Wait for next request from client
        zmq::message_t request;
        try {
            socket.recv(&request);
        }
        catch(zmq::error_t& e) {
            break;
        }
	    std::string NewMessageBuffer(static_cast<char *>(request.data()), request.size());
        if (WRObj->ReceiverType == "AP_DC")
            DCObj->ProcessMessage(NewMessageBuffer);
        else if (WRObj->ReceiverType == "AP")
            APObj->ProcessReceivedMsg(NewMessageBuffer);
	
    }
    return (NULL);
}

zmq::context_t *Context;
zmq::socket_t *Subscriber;
zmq::context_t *WT_Context;
zmq::socket_t *WT_Clients;

/** 
	Creates multiple worker threads - ZMQ simple mad box pattern
*/

void T_CreateWorkerThreads(std::string Type, std::string Port) {
    WT_Context = new zmq::context_t(1);
    WT_Clients = new zmq::socket_t(*WT_Context, ZMQ_PULL);
    struct Worker_Routine WRObj;
    WRObj.WR_Context = WT_Context;
    WRObj.ReceiverType = Type;
    std::string PortNum;
	if (Type == "AP_DC")
		PortNum = Port;
	else if (Type == "AP")
		PortNum = Port;
    WT_Clients->bind ("tcp://*:"+PortNum);
    zmq::socket_t workers (*WT_Context, ZMQ_DEALER);
    workers.bind ("inproc://workers");
    //  Launch pool of worker threads
    for (int thread_nbr = 0; thread_nbr != 5; thread_nbr++) {
        pthread_t worker;
        pthread_create(&worker, NULL, WorkerRoutine, (void *) &WRObj);
    }
    //  Connect work threads to client threads via a queue;
    try {
        zmq::proxy((void *) *WT_Clients, (void *) workers, (void *) NULL);
    } catch(zmq::error_t& e) {
        std::cout << "Terminating worker threads\n";
        return;
    }
}


/** 
	Explicitly kills receiver sockets which are in blocking mode 
*/

void COMMUNICATION::KillReceiverSocket() {
    WorkerThreadExit = true;
    Subscriber->close();
    Context->close();
}


/*!
	* Each service in car and AP calls this function to have sockets opened 
	* and subscribed to UDM to recieve all messages destined to it
 * @param data 'data is data needs to be sent through protobuf
 */

void COMMUNICATION::ReceiveMessage(void *ModuleObj, std::string _ID, char* ModuleRef...) {
    va_list args;
    va_start(args, ModuleRef);
    std::string ModuleType = va_arg(args, char*);

    VEHICLE *VehObj;
    DCA_DATACOLLECTION *DCAObj;
    TASK_DISTRIBUTION *TASKObj;
    COMMUNICATION *CommObj;

    if(ModuleType == "CAR") {
        VehObj = (VEHICLE *)ModuleObj;
        CommObj = new COMMUNICATION(VehObj->LogsObj);
    } else if(ModuleType == "AP") {
        APObj = (ACCESSPOINT *)ModuleObj;
        CommObj = new COMMUNICATION(APObj->LogsObj);
    } else if(ModuleType == "DCA") {
        DCAObj = (DCA_DATACOLLECTION *)ModuleObj;
        CommObj = new COMMUNICATION(DCAObj->LogsObj);
    } else if(ModuleType == "AP_DC") {
        DCObj = (AP_DATA_COLLECTION *)ModuleObj;
        CommObj = new COMMUNICATION(DCObj->LogsObj);
    } else if(ModuleType == "TASKD") {
        TASKObj = (TASK_DISTRIBUTION *)ModuleObj;
        CommObj = new COMMUNICATION(TASKObj->LogsObj);
    }

    std::string ID = _ID + "ID";
    zmq::message_t MessageReceived;
    zmq::message_t Address;
    
    if(true) {
        Context = new zmq::context_t(1);
        Subscriber = new zmq::socket_t(*Context, ZMQ_SUB);
        Subscriber->connect("tcp://" + CommObj->GetParserIP() + ":" + (std::to_string(std::stoi(CommObj->GetUDMPort()) + 1)));
        Subscriber->setsockopt(ZMQ_SUBSCRIBE, ID.c_str(), std::strlen(ID.c_str()));
    }
		

    if(ModuleType == "CAR") {
        while (VehObj->CarExists) {
            try {
                Subscriber->recv(&Address);
                Subscriber->recv(&MessageReceived);
            }
            catch(zmq::error_t& e) {
                VehObj->LogsObj->LogDebug("Vehicle Cluster Process Receiver Sockets Terminated");
                break;
            }
            VehObj->LogsObj->LogDebug(std::string("New Message Received"));
            std::string NewMessageBuffer(static_cast<char *>(MessageReceived.data()), MessageReceived.size());
            VehObj->ProcessReceivedMsg(NewMessageBuffer);
        }
    } else if(ModuleType == "AP") {
	/* Create multiple worker threads */
        std::thread CreateWorkerThreads(T_CreateWorkerThreads, "AP", APObj->APIPInfo->Port);
	/* Connect worker threads to worker routines by creating push sockets */
        auto WT_Push_Context = new zmq::context_t(1);
        auto WT_Push_Socket = new zmq::socket_t(*WT_Push_Context, ZMQ_PUSH);
        std::string IP = "127.0.0.1";
        std::string Port = APObj->APIPInfo->Port;
        WT_Push_Socket->connect("tcp://" + IP  + ":" + Port);
        
	while(!APObj->NoMoreCars) {
            Subscriber->recv(&Address);
            Subscriber->recv(&MessageReceived);
            APObj->LogsObj->LogDebug(std::string("New Message Received"));
            std::string ResponseBuffer(static_cast<char *>(MessageReceived.data()), MessageReceived.size());
            MESSAGEFROMUDM UDMPkt;
            UDMPkt.ParseFromString(ResponseBuffer);
            if (UDMPkt.exit()) {
                APObj->NoMoreCars = true;
	        WorkerThreadExit = true;
                WT_Clients->close();
                WT_Context->close();
                break;
            }
		/* Push message received to worker threads */
            WT_Push_Socket->send(MessageReceived);
        }
        WT_Push_Socket->close();
        WT_Push_Context->close();
        KillReceiverSocket();
        CreateWorkerThreads.join();
    } else if(ModuleType == "DCA") {
        while (true) {
            try {
                Subscriber->recv(&Address);
                Subscriber->recv(&MessageReceived);
            }
            catch(zmq::error_t& e) {
                VehObj->LogsObj->LogDebug("Vehicle DCA process Receiver Sockets Terminated");
                std::cout << "Vehicle DCA Process Receiver Sockets Terminated\n";
                break;
            }
            DCAObj->LogsObj->LogDebug(std::string("New Message Received"));
            std::string NewMessageBuffer(static_cast<char *>(MessageReceived.data()), MessageReceived.size());
            DCAObj->ProcessReceivedMsg(NewMessageBuffer);
        }
    } else if(ModuleType == "AP_DC") {
	/* Create multiple worker threads */
        std::thread CreateWorkerThreads(T_CreateWorkerThreads, "AP_DC", std::to_string(std::stoi(DCObj->IP_Port_Info->Port) + 2));
	/* Connect worker threads to worker routines by creating push sockets */
        auto WT_Push_Context = new zmq::context_t(1);
        auto WT_Push_Socket = new zmq::socket_t(*WT_Push_Context, ZMQ_PUSH);
        std::string IP = "127.0.0.1";
        std::string Port = std::to_string(std::stoi(DCObj->IP_Port_Info->Port) + 2);
        WT_Push_Socket->connect("tcp://" + IP  + ":" + std::to_string(std::stoi(DCObj->IP_Port_Info->Port) + 2));

        while (DCObj->APExists) {
	   
            Subscriber->recv(&Address);
            Subscriber->recv(&MessageReceived);

            DCObj->LogsObj->LogDebug(std::string("New Message Received"));
            std::string NewMessageBuffer(static_cast<char *>(MessageReceived.data()), MessageReceived.size());
            MESSAGEFROMUDM UDMPkt;
            UDMPkt.ParseFromString(NewMessageBuffer);
            if (UDMPkt.exit()) {
                DCObj->StatsObj->PrintDCStats(DCObj);
                DCObj->APExists = false;
		WorkerThreadExit = true;
                WT_Clients->close();
                WT_Context->close();
                break;
            }
		/* Push message received to worker threads */
            WT_Push_Socket->send(MessageReceived);
        }
        WT_Push_Socket->close();
        WT_Push_Context->close();
        KillReceiverSocket();
        CreateWorkerThreads.join();
    } else if(ModuleType == "TASKD") {
        while (TASKObj->CarExists) {
            try {
                Subscriber->recv(&Address);
                Subscriber->recv(&MessageReceived);
            }
            catch(zmq::error_t& e) {
                VehObj->LogsObj->LogDebug("Vehicle TASKD process Receiver Sockets Terminated");
                std::cout << "Vehicle TASKD Process Receiver Sockets Terminated\n";
                break;
            }
            TASKObj->LogsObj->LogDebug(std::string("New Message Received"));
            std::string NewMessageBuffer(static_cast<char *>(MessageReceived.data()), MessageReceived.size());
            TASKObj->ProcessReceivedMsg(NewMessageBuffer);
        }
    }
    delete CommObj;
}


/*!
 * Sends a INIT message to UDM (once when a module starts) contains vehicle ID(VehID), car server ip(CarServerIP)
 * Cluster module port number (CarClusterPort) and type of the source ("CAR")
 */

void COMMUNICATION::SendInit(std::string VehID) {
    MSGFROMNODETOUDM InitMsg;
    std::string DataToSend;
    InitMsg.set_src_id(VehID);
    InitMsg.set_src_type("CAR");
	/* UDM do not require ip and port address anymore, hence sending dummy ip and port address here */
    InitMsg.set_ip("192.168.0.1");
    InitMsg.set_port("1234");
    InitMsg.set_mtype("INIT");
    InitMsg.SerializeToString(&DataToSend);


    COMMUNICATION CommObj(LogsObj);
    zmq::context_t Context(1);
    zmq::socket_t Socket(Context, ZMQ_PUSH);
    std::string IP = CommObj.GetParserIP();
    std::string Port = CommObj.GetUDMPort();
    Socket.connect("tcp://" + IP + ":" + Port);
    LogsObj->LogDebug("Sending INIT Message IP = " + CommObj.GetParserIP() + " Port = " + CommObj.GetUDMPort());
    LogsObj->LogInfo("Sending INIT Message IP = " + CommObj.GetParserIP() + " Port = " + CommObj.GetUDMPort());
    zmq::message_t Message(DataToSend.size()), Reply;
    memcpy ((void*)Message.data(), DataToSend.c_str(), DataToSend.size());
    Socket.send(Message);
}
