//
// Created by kiran on 10/27/18.
//

#include "../../header/car/CarHeader.h"
#include "../../header/car/CarCluster.h"
#include "../../../lib/zmq/zhelpers.hpp"
#include "../../header/apps/DCA.h"

/*********************************************************************************
	Clustering service module in car

**********************************************************************************/


/**     
        Thread that receives Car exit status 
*/

void T_CarExitStatus(VEHICLE *VehObj) {
    zmq::message_t MessageReceived;
    zmq::message_t Address;
    zmq::context_t *Context;
    zmq::socket_t *Subscriber;
    COMMUNICATION *CommObj;
    CommObj = new COMMUNICATION(VehObj->LogsObj);

    if(true) {
        Context = new zmq::context_t(1);
        Subscriber = new zmq::socket_t(*Context, ZMQ_SUB);
        Subscriber->connect("tcp://" + CommObj->GetParserIP() + ":" + (std::to_string(std::stoi(CommObj->GetUDMPort()) + 3)));
	    std::string ID = VehObj->GetVehicleID() + "EXITID";
        Subscriber->setsockopt(ZMQ_SUBSCRIBE, ID.c_str(), std::strlen(ID.c_str()));
	    Subscriber->recv(&Address);
        Subscriber->recv(&MessageReceived);
	    CommObj->KillReceiverSocket();
	    VehObj->CarExists = false;
    }
    delete CommObj;
    Subscriber->close();
    Context->close();
	
}


/** 
        Receiver created in Communication module to receive new messages
*/

void T_MessageHandler(VEHICLE *VehObj) {
    VehObj->CommObj->ReceiveMessage((void*)VehObj, VehObj->GetVehicleID(), (char*)"A", "CAR");
}


void T_ClusterService(VEHICLE *VehObj) {
    VehObj->ClusterObj->ClusterServer(VehObj);
}

/**
	Function that publishes cluster information to subscribed applications
	as and when there is a state of change in cluster status
*/

void CAR_CLUSTER::ClusterServer(VEHICLE *VehObj) {
    zmq::context_t Context(1);
    zmq::socket_t Publisher(Context, ZMQ_PUB);
    Publisher.bind("tcp://"+VehObj->LocalInfoObj->GetCarServerIP()+":"+VehObj->LocalInfoObj->GetCarClusterPort());
    std::string VID = VehObj->GetVehicleID(), CMList, DataToSend;

    while(VehObj->CarExists) {
        if(PublishClusterInfo) {
            CARCLUSTERINFORESPONSE ResponseMsg;
            if (!VehObj->CarExists) {
            } else {
                if (ClusterState != "INIT") {
                    if (ClusterHead == VID) {
                        ResponseMsg.set_is_cluster_head(true);
                        ResponseMsg.set_cluster_manager(CurrentClusterManager);
                    } else {
                        ResponseMsg.set_is_cluster_head(false);
                        ResponseMsg.set_ch(ClusterHead);
                    }
                    if (!ClusterMembers.empty()) {
                        for (auto it = ClusterMembers.begin(); it != ClusterMembers.end(); it++) {
                            CMList.append(*it);
                            CMList.append(" ");
                        }
                    }
                    ResponseMsg.set_cm_list(CMList);
                    ResponseMsg.set_cluster_id(MicrocloudID);
                    ResponseMsg.set_cluster_state(ClusterState);
                } else {
                    ResponseMsg.set_cluster_state(ClusterState);
                }
                ResponseMsg.set_default_manager(DefaultClusterManager);
		        ResponseMsg.set_cluster_seq_num(ClusterSeqNum);
            }
            ResponseMsg.SerializeToString(&DataToSend);
            s_sendmore(Publisher, "APP");
            s_send(Publisher, DataToSend);
            CMList.clear();
            DataToSend.clear();
            PublishClusterInfo = false;
            LogsObj->LogDebug("Cluster Information is published to all the APP subscribers");
        }
	sleep(1);
    }
    DataToSend.clear();
    s_sendmore(Publisher, "APP");
    s_send(Publisher, DataToSend);
    LogsObj->LogDebug("Closing the Publisher 'Publish cluster info to apps' socket");
    Publisher.close();
}

/**
	Reads Speed info from Speed module periodically (1s)
*/

void T_ProbeSpeedInfo(VEHICLE *VehObj) {

    zmq::context_t SpeedContext;
    zmq::socket_t SpeedReadSocket(SpeedContext, ZMQ_REQ);

    VehObj->LogsObj->LogDebug("Connecting to in-car Speed Module...");
    VehObj->LogsObj->LogInfo("Connecting to in-car Speed Module...");
    SpeedReadSocket.connect("tcp://"+VehObj->LocalInfoObj->GetCarServerIP()+":"+VehObj->LocalInfoObj->GetCarSpeedPort());

    bool S_CarExistsInNet;

    CARSPEEDREQUESTFROMINCAR RequestStr;
    RequestStr.set_req("REQ");
    std::string RequestBuffer;
    RequestStr.SerializeToString(&RequestBuffer);

    CARSPEEDRESPONSETOINCAR ResponseStr;
    std::string ResponseBuffer;

    double _Speed;
    VehObj->LogsObj->LogDebug("Succesfully Connected !! Periodically every seconnd Location, Speed"
                                      " information is collected from in-car Modules...");

    zmq::message_t request(RequestBuffer.length());
    auto requestInfo = RequestBuffer.c_str();
    memcpy (request.data(), requestInfo, RequestBuffer.length());
    zmq::message_t reply;
    char *ReplyMsg;

    while (VehObj->CarExists) {
        SpeedReadSocket.send(request);
        SpeedReadSocket.recv(&reply);
        ReplyMsg = new char [reply.size()];
        memcpy (ReplyMsg, reply.data(), reply.size());
        ResponseBuffer = ReplyMsg;
        ResponseStr.ParseFromString(ResponseBuffer);

        S_CarExistsInNet = ResponseStr.carexists();
        if(S_CarExistsInNet) {
            _Speed = ResponseStr.speed();
            VehObj->LogsObj->LogDebug("Collected updated Speed details from in-car Speed Module, Speed = " + std::to_string(_Speed));
        } else {
            VehObj->CarExists = false;
            VehObj->LogsObj->LogDebug("Car do not exist anymore");
            VehObj->LogsObj->LogInfo("Car do not exist anymore");
        }
        VehObj->SetSpeedInfo(_Speed);
        delete []ReplyMsg;
        sleep(1);
    }
    std::string TerminateBuffer = "";
    zmq::message_t TerminateReq(0);
    auto TerminaterequestInfo = TerminateBuffer.c_str();
    memcpy (TerminateReq.data(), TerminaterequestInfo, TerminateBuffer.length());
    SpeedReadSocket.send(TerminateReq);


    VehObj->LogsObj->LogDebug("Closing Speed Requesting Sockets");
    VehObj->LogsObj->LogInfo("Closing Speed Requesting Sockets");
    SpeedReadSocket.close();
}

/**
	Reads GPS info from GPS module periodically (1s)
*/

void T_ProbeGPSInfo(VEHICLE *VehObj) {

    zmq::context_t GPSContext;
    zmq::socket_t GPSReadSocket(GPSContext, ZMQ_REQ);

    VehObj->LogsObj->LogDebug("Connecting to in-car Position Module...");
    VehObj->LogsObj->LogInfo("Connecting to in-car Position Module...");
    GPSReadSocket.connect("tcp://"+VehObj->LocalInfoObj->GetCarServerIP()+":"+VehObj->LocalInfoObj->GetCarPosPort());

    bool P_CarExistsInNet;

    CARGPSREQUESTFROMINCAR RequestStr;
    RequestStr.set_req("REQ");
    std::string RequestBuffer;
    RequestStr.SerializeToString(&RequestBuffer);

    CARGPSRESPONSETOINCAR ResponseStr;

    char *ReplyMsg;
    double PosX, PosY, _Direction;
    VehObj->LogsObj->LogDebug("Succesfully Connected !! Periodically every seconnd Location, Position"
                                      " information is collected from in-car Modules...");

    zmq::message_t request(RequestBuffer.length());
    auto requestInfo = RequestBuffer.c_str();
    memcpy (request.data(), requestInfo, RequestBuffer.length());
    zmq::message_t reply;

    while (VehObj->CarExists) {
        GPSReadSocket.send(request);
        GPSReadSocket.recv(&reply);
        ReplyMsg = new char [reply.size()];
        memcpy (ReplyMsg, reply.data(), reply.size());
        std::string ResponseBuffer(static_cast<char*>(reply.data()), reply.size());
        ResponseStr.ParseFromString(ResponseBuffer);

        P_CarExistsInNet = ResponseStr.carexists();
        if(P_CarExistsInNet) {
            PosX = ResponseStr.x();
            PosY = ResponseStr.y();
            _Direction = ResponseStr.dir();
            VehObj->LogsObj->LogDebug(
                    "Collected updated Position details from in-car Position Module, Poition = (" +
                    std::to_string(PosX) + "," + std::to_string(PosY) + ")");
        } else {
            VehObj->CarExists = false;
            VehObj->LogsObj->LogDebug("Car does not exist anymore");
            VehObj->LogsObj->LogInfo("Car does not exist anymore");
        }

        VehObj->SetGPSInfo(PosX,PosY,_Direction);
        ResponseBuffer.clear();
        sleep(1);
        delete []ReplyMsg;
    }
    VehObj->LogsObj->LogDebug("Closing Position Requesting Sockets");
    GPSReadSocket.close();
}

/**
	Periodically sends beacons for neigbor table maintenance
*/

void T_BroadcastBeacon(VEHICLE *VehObj){
    VehObj->LogsObj->LogDebug("Periodically sends Beacons(Hello)");
    VehObj->LogsObj->LogInfo("Periodically sends Beacons(Hello)");
    useconds_t BeaconInterval = (useconds_t )VehObj->NTObj->GetBeaconInterval() * 1000000;
    while(VehObj->CarExists) {
        VehObj->LogsObj->LogDebug("Broadcasting Hello Packet...");
        VehObj->NTObj->SendHelloPackets(VehObj);
        usleep(BeaconInterval);
    }
}

/**
	Periodically updates car's local info to AP
*/

void T_SendLocalInfotoAP(VEHICLE *VehObj){
    VehObj->LogsObj->LogDebug("Periodically sends Local Info to AP");
    VehObj->LogsObj->LogInfo("Periodically sends Local Info to AP");
    useconds_t SendInterval = (useconds_t )VehObj->ClusterObj->GetLocalInfoInterval() * 1000000;
    BASE_COMMUNICATION *BaseCommObj;
    if(true)
        BaseCommObj = new COMMUNICATION("DEFAULT", VehObj->LogsObj);
    while(VehObj->CarExists) {
        VehObj->LogsObj->LogDebug("Sending Local Info to AP... " );
        VehObj->ClusterObj->SendVehUpdatedInfo(VehObj, BaseCommObj);
        usleep(SendInterval);
    }
    delete BaseCommObj;

}

void CAR_CLUSTER::SendVehUpdatedInfo(VEHICLE *VehObj, BASE_COMMUNICATION *BaseCommObj) {
    std::string DestID = CurrentClusterManager;
    if (DestID == "")
	    return;
    std::string DataToSend;
    double X, Y, Direction, Speed;
    VehObj->GetGPSInfo(&X, &Y, &Direction);
    VehObj->GetSpeedInfo(&Speed);
    std::string SrcID = VehObj->GetVehicleID();

    UPDATE_LOCALINFO LocalInfoPKT;
    LocalInfoPKT.Dataline1 = "Hello, This is a Updated vehicle information UNICAST message";
    LocalInfoPKT.Dataline2 = "From " + SrcID;
    LocalInfoPKT.SrcID = SrcID;
    LocalInfoPKT.X = X;
    LocalInfoPKT.Y = Y;
    LocalInfoPKT.Speed = Speed;
    LocalInfoPKT.Direction = Direction;
    LocalInfoPKT.Datatype = "CLUSTERINFO";
    LocalInfoPKT.Timestamp = LogsObj->GetCurrentTime();
    BaseCommObj->SendUnicastMsg(SrcID, DestID, &LocalInfoPKT, (char*)"B", "LOCALINFO");
}

void NEIGHBORTABLE::SendHelloPackets(VEHICLE *VehObj) {
    BASE_COMMUNICATION *BaseCommObj;
    std::string DataToSend;
    double X, Y, Direction, Speed;
    VehObj->GetGPSInfo(&X, &Y, &Direction);
    VehObj->GetSpeedInfo(&Speed);

    HELLO_PACKET HelloPkt;
    HelloPkt.SrcID = VehObj->GetVehicleID();
    HelloPkt.Dataline1 = "Hello, This is a Hello Broadcast message";
    HelloPkt.Dataline2 = "From " + VehObj->GetVehicleID();
    HelloPkt.X = X;
    HelloPkt.Y = Y;
    HelloPkt.Speed = Speed;
    HelloPkt.Direction = Direction;
    HelloPkt.Timestamp = VehObj->LogsObj->GetCurrentTime();
    HelloPkt.Datatype = "HELLO";

    if(true)
        BaseCommObj = new COMMUNICATION("DEFAULT", LogsObj);
    BaseCommObj->SendBroadcastMsg(VehObj->GetVehicleID(), &HelloPkt, (char*)"A", "HELLO");


    delete BaseCommObj;

}

/**
	Process incoming Hello packets
*/

void NEIGHBORTABLE::ProcessHelloPackets(HELLO_PACKET *HelloPkt) {
    auto NewObj = new NEIGHBORTABLE;
    NewObj->VID = HelloPkt->SrcID;
    NewObj->X = HelloPkt->X;
    NewObj->Y = HelloPkt->Y;
    NewObj->Speed = HelloPkt->Speed;
    NewObj->Direction = HelloPkt->Direction;
    time(&NewObj->Timestamp);
    AddToNeighborTable(NewObj->VID, NewObj);
}

/**
	Populates neighbor table
*/

void NEIGHBORTABLE::AddToNeighborTable(std::string ID, NEIGHBORTABLE *NewNTObj) {

    NeighborTable[ID] = NewNTObj;
    std::map<std::string, NEIGHBORTABLE*> CopyOfNeighborTable = GetCopyNeighborTable();

    LogsObj->LogDebug("The Neighbor Table has the following data");
    LogsObj->LogDebug("-------------------------------------------------------------------------------------");
    for(auto it = CopyOfNeighborTable.begin(); it != CopyOfNeighborTable.end(); it++) {
        auto Obj = it->second;
        LogsObj->LogDebug("ID = " + it->first + " X = " + std::to_string(Obj->X) + " Y = " + std::to_string(Obj->Y) + " Speed = " + std::to_string(Obj->Speed) + " Direction = " + std::to_string(Obj->Direction));
    }
    LogsObj->LogDebug("--------------------------------------------------------------------------------------");
}


/**
	Updates vehicles cluster status
*/

void CAR_CLUSTER::StoreClusterInfo(CLUSTER_INFO *ClusterData, std::string VehID) {
    ClusterMembers.clear();
    ClusterHead = ClusterData->CH;
    MicrocloudID = ClusterData->MC_ID;
    ClusterSeqNum = ClusterData->ClusterSeqNum;
    LogsObj->LogDebug("Recieved and processed cluster info from " + MicrocloudID);
    LogsObj->LogDebug("Cluster Head = " + ClusterHead);
    std::string ListOfCM = ClusterData->ListOfCM;
    size_t pos = 0;
    std::string token, delimiter = " ";
    while ((pos = ListOfCM.find(delimiter)) != std::string::npos) {
        token = ListOfCM.substr(0, pos);
        if(token != ClusterHead)
            ClusterMembers.push_front(token);
        ListOfCM.erase(0, pos + delimiter.length());
    }
    for(auto it = ClusterMembers.begin(); it != ClusterMembers.end(); it++) {
        LogsObj->LogDebug("Cluster Member " + *it);
    }
    if(VehID == ClusterHead)
        ClusterState = "CH";
    else
        ClusterState = "CM";
    time(&ClusterStateInitialTimer);
}


/**
	Periodically vValidates Neighbor Table entries
*/
void T_NTEntryExpireTimer(VEHICLE *VehObj) {
    while(VehObj->CarExists) {
        VehObj->NTObj->ValidateNTEntries();
        sleep(1);
    }
}


void NEIGHBORTABLE::ValidateNTEntries() {
    auto TimerInterval = (useconds_t )NTExpiryTimer;
    std::map<std::string, NEIGHBORTABLE*> CopyOfNeighborTable = GetCopyNeighborTable();
    time_t CurTime;
    time(&CurTime);
    NEIGHBORTABLE *IterObj;
    double ElapsedTime;
    for (auto it = CopyOfNeighborTable.begin(); it != CopyOfNeighborTable.end(); it++) {
        IterObj = it->second;
        ElapsedTime = difftime(CurTime, IterObj->Timestamp);
        if(ElapsedTime > TimerInterval) {
            LogsObj->LogDebug("Time Expired for Vehicle " + it->first + " in the Neighbor Table, Last Message received before = " + std::to_string(ElapsedTime));
            LogsObj->LogInfo("Time Expired for Vehicle " + it->first + " in the Neighbor Table, Last Message received before = " + std::to_string(ElapsedTime));
            delete it->second;
            DelFromNeighborTable(it->first);
        }
    }
}
void NEIGHBORTABLE::DelFromNeighborTable(std::string VID) {
    NeighborTable.erase(VID);
}

/**
	Checks if Cluster manager timer(Missing 2 RSU beacons) has been expired
	If yes, sets Cluster manager back to default cluster manager
*/

void T_ClusterManagerTimerExpier(VEHICLE *VehObj) {
    auto TimerInterval = (useconds_t )VehObj->ClusterObj->ClusterManagerExpiryTimer;
    double ElapsedTime;
    while(VehObj->CarExists) {
        while(VehObj->ClusterObj->CurrentClusterManager != VehObj->ClusterObj->DefaultClusterManager) {
            time_t CurTime;
            time(&CurTime);
            ElapsedTime = difftime(CurTime, VehObj->ClusterObj->ClusterManagerInitTimer);
            if (ElapsedTime > TimerInterval) {
                VehObj->LogsObj->LogDebug(
                        "Time Expired for Current Cluster Manager " + VehObj->ClusterObj->CurrentClusterManager);
                VehObj->LogsObj->LogInfo(
                        "Time Expired for Current Cluster Manager " + VehObj->ClusterObj->CurrentClusterManager);
                VehObj->ClusterObj->CurrentClusterManager = VehObj->ClusterObj->DefaultClusterManager;
                VehObj->ClusterObj->SetPublishClusterInfo(true);
            }
            sleep(1);
        }
        sleep(1);
    }
}

/**
	Checks if Cluster state ('CM', 'CH') has expired
	If yes, sets state to 'INIT'
*/

void T_ClusterStateExpireTimer(VEHICLE *VehObj) {
    time_t _ClusterStateInitTime, CurTime;
    double _TimerInterval, ElapsedTime;
    std::string _ClusterState;
    while(VehObj->CarExists) {
        while(VehObj->ClusterObj->GetClusterState() != "INIT") {
            std::tie(_ClusterStateInitTime , _TimerInterval, _ClusterState) = VehObj->ClusterObj->GetClusterStateAndTimers();
            time(&CurTime);
            ElapsedTime = difftime(CurTime, _ClusterStateInitTime);
            if (ElapsedTime >= _TimerInterval) {
                VehObj->LogsObj->LogDebug(
                        "Time Expired for Cluster State " + _ClusterState);
                VehObj->LogsObj->LogInfo(
                        "Time Expired for Current State " + _ClusterState);
                VehObj->ClusterObj->SetClusterState("INIT");
                VehObj->ClusterObj->SetPublishClusterInfo(true);

            }
            sleep(1);
        }
        sleep(1);
    }
}

/**
        Function is called when Clustering service module receives message 
        @param MessageReceived The new message that is received 
*/

void VEHICLE::ProcessReceivedMsg(std::string MessageReceived) {
    MESSAGEFROMUDM Message;
    Message.ParseFromString(MessageReceived);
    std::string DataLine1, DataLine2, DataType, SourceType;
    SourceType = Message.src_type();
    ALLDATA DataString = Message.data();
    DataLine1 = DataString.dataline1();
    DataLine2 = DataString.dataline2();
    DataType = DataString.datatype();
    if(SourceType == "CAR") {
        if(DataType == "HELLO") {
            HELLO_PACKET HelloPkt;
            HelloPkt.Dataline1 = DataString.dataline1();
            HelloPkt.Dataline2 = DataString.dataline2();
            HelloPkt.SrcID = DataString.id();
            HelloPkt.X = DataString.x();
            HelloPkt.Y = DataString.y();
            HelloPkt.Direction = DataString.dir();
            HelloPkt.Speed = DataString.speed();
            HelloPkt.Timestamp = DataString.timestamp();
            HelloPkt.Datatype = DataString.datatype();
            LogsObj->LogDebug("Received Hello Packet from " + DataString.id());
            NTObj->ProcessHelloPackets(&HelloPkt);
        }
    }
    if(SourceType == "RSU") {
        if(DataType == "RSUBEACON") {
            LogsObj->LogDebug("Recieved RSU BEACON and hence Current Cluster Manager is " + ClusterObj->CurrentClusterManager);
            ClusterObj->CurrentClusterManager = DataString.id();
            time(&(ClusterObj->ClusterManagerInitTimer));
        } else if(DataType == "CLUSTERINFO") {
            CLUSTER_INFO ClusterData;
            ClusterData.Dataline1 = DataString.dataline1();
            ClusterData.Dataline2 = DataString.dataline2();
            ClusterData.Datatype = DataString.datatype();
            ClusterData.CH = DataString.ch();
            ClusterData.ListOfCM = DataString.cmlist();
            ClusterData.MC_ID = DataString.clusterid();
            ClusterData.Timestamp = DataString.timestamp();
            ClusterData.Datatype = DataString.datatype();
	    ClusterData.ClusterSeqNum = DataString.cluster_seq_num();
            LogsObj->LogDebug("Cluster structure Message is received");
            ClusterObj->StoreClusterInfo(&ClusterData, GetVehicleID());
            ClusterObj->SetPublishClusterInfo(true);
        }

    }
    if(SourceType == "BS") {
        if(DataType == "CLUSTERINFO") {
            CLUSTER_INFO ClusterData;
            ClusterData.Dataline1 = DataString.dataline1();
            ClusterData.Dataline2 = DataString.dataline2();
            ClusterData.Datatype = DataString.datatype();
            ClusterData.CH = DataString.ch();
            ClusterData.ListOfCM = DataString.cmlist();
            ClusterData.MC_ID = DataString.clusterid();
            ClusterData.Timestamp = DataString.timestamp();
            ClusterData.Datatype = DataString.datatype();
	    ClusterData.ClusterSeqNum = DataString.cluster_seq_num();
            LogsObj->LogDebug("Cluster structure Message is received");
            ClusterObj->StoreClusterInfo(&ClusterData, GetVehicleID());
            ClusterObj->SetPublishClusterInfo(true);
        }
    }
    LogsObj->LogDebug("Messaged received is from source type " + SourceType);
    LogsObj->LogInfo(std::string("Recieved msg:: ") + DataLine1 + " " + DataLine2 + " And this message was sent by sender at " + DataString.timestamp());
    LogsObj->LogDebug(std::string("Recieved msg:: ") + DataLine1 + " " + DataLine2 + " And this message was sent by sender at " + DataString.timestamp());
}

