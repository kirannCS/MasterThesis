//
// Created by kiran on 8/3/18.
//

#include "../../header/car/CarHeader.h"
#include "../../header/car/CarCluster.h"
#include "../../../lib/zmq/zhelpers.hpp"

extern zmq::socket_t* SendHelloPacketSocket;
extern zmq::socket_t* SendLocalInfoPacketSocket;

void T_MessageHandler(VEHICLE *) ;
void T_ProbeSpeedInfo(VEHICLE *);
void T_ProbeGPSInfo(VEHICLE *);
void T_BroadcastBeacon(VEHICLE *);
void T_SendLocalInfotoAP(VEHICLE *);
void T_NTEntryExpireTimer(VEHICLE *);
void T_ClusterManagerTimerExpier(VEHICLE *);
void T_ClusterStateExpireTimer(VEHICLE *);
void T_ClusterService(VEHICLE *);
void T_CarExitStatus(VEHICLE *);

/* Creates socket used to send Hello Packets */
void CreateSendHelloPacketSocket(std::string IP, std::string Port) {
    zmq::context_t* Context = new zmq::context_t(1);
    SendHelloPacketSocket = new zmq::socket_t(*Context, ZMQ_PUSH);
    SendHelloPacketSocket->connect("tcp://" + IP + ":" + Port);
}

/* Creates socket used to send Local Info Packets */
void CreateSendLocalInfoPacketSocket(std::string IP, std::string Port) {
    zmq::context_t* Context = new zmq::context_t(1);
    SendLocalInfoPacketSocket = new zmq::socket_t(*Context, ZMQ_PUSH);
    SendLocalInfoPacketSocket->connect("tcp://" + IP + ":" + Port);
}

/*!
 * 'main()' function of the base station entity
 * @param argc 2
 * @param argv argv[0] has vehcile ID and argv[1] has the starting Port number(used to assign port numbers
 * to different servers in the in-car modules)
 * @return 0 on success, -1 on error
 */
int main(int argc, char*argv[]) {
    auto NTObj = new NEIGHBORTABLE;
    auto LogsObj = new LOGGING_UTILITY(std::string(argv[1]), "CAR", "CLUSTER");
    auto ClusterObj = new CAR_CLUSTER(LogsObj, argv[1]);
    auto LocalInfoObj = new LOCAL_INFO;
    ClusterObj->LogsObj = LogsObj;
    auto VehObj = new VEHICLE(LogsObj, NTObj, ClusterObj);
    VehObj->SetVehicleID(std::string(argv[1]));
    VehObj->ClusterObj = ClusterObj;
    VehObj->SetGPSInfo(-1, -1, -1);
    VehObj->SetSpeedInfo(-1);
    VehObj->LocalInfoObj = LocalInfoObj;

    auto CARCommObj = new COMMUNICATION(LogsObj);
    VehObj->CommObj = CARCommObj;
    VehObj->LocalInfoObj->SetCarServerIP(std::string(argv[3]));

    VehObj->NTObj = NTObj;
    NTObj->LogsObj = LogsObj;
    VehObj->NTObj->LogsObj = LogsObj;
    VehObj->LocalInfoObj->AssignPorts(argv[2], LogsObj);

    //CreateSendHelloPacketSocket(CARCommObj->GetParserIP(), CARCommObj->GetUDMPort());
    CreateSendLocalInfoPacketSocket(CARCommObj->GetParserIP(), CARCommObj->GetUDMPort());

    LogsObj->LogDebug("Inside 'Car Cluster' Module My ID = " + VehObj->GetVehicleID());
    LogsObj->LogInfo("Inside 'Car Cluster' Module My ID = " + VehObj->GetVehicleID());
    CARCommObj->SendInit(VehObj->GetVehicleID());

    LogsObj->LogDebug("Starting 'Read from GPS module' thread");
    LogsObj->LogInfo("Starting 'Read from GPS module' thread");
    std::thread ProbeGPSInfo(T_ProbeGPSInfo, VehObj);

    LogsObj->LogDebug("Starting 'Read from Speed module' thread");
    LogsObj->LogInfo("Starting 'Read from Speed module' thread");
    std::thread ProbeSpeedInfo(T_ProbeSpeedInfo, VehObj);

    LogsObj->LogDebug("Starting 'Car Cluster info Server' thread");
    LogsObj->LogInfo("Starting 'Car Cluster info Server' thread");
    std::thread CarMsgReceiveService(T_MessageHandler, VehObj);

    //LogsObj->LogDebug("Starting 'Car Neighbor Tabe entry mainentance' thread");
    //LogsObj->LogInfo("Starting 'Car Neighbor Tabe entry mainentance' thread");
    //std::thread NTEntryMaintenance(T_NTEntryExpireTimer, VehObj);

    LogsObj->LogDebug("Starting 'Cluster Manager Timer Expire' thread");
    LogsObj->LogInfo("Starting 'Cluster Manager Timer Expire' thread");
    std::thread ClusterManagerTimerExp(T_ClusterManagerTimerExpier, VehObj);

    //LogsObj->LogDebug("Starting 'Car Broadcast Beaons' thread");
    //LogsObj->LogInfo("Starting 'Car Broadcast Beaons' thread");
    //std::thread BroadcastBeacon(T_BroadcastBeacon, VehObj);

    LogsObj->LogDebug("Starting 'Local Info to AP' thread");
    LogsObj->LogInfo("Starting 'Local Info to AP' thread");
    std::thread LocalInfotoAP(T_SendLocalInfotoAP, VehObj);

    LogsObj->LogDebug("Starting 'Cluster State Expire' thread");
    LogsObj->LogInfo("Starting 'Cluster State Expire' thread");
    std::thread ClusterStateExpireThread(T_ClusterStateExpireTimer, VehObj);

    LogsObj->LogDebug("Starting 'Cluster State Expire' thread");
    LogsObj->LogInfo("Starting 'Cluster State Expire' thread");
    std::thread ClusterService(T_ClusterService, VehObj);
    
    std::thread CarExitStatus(T_CarExitStatus, VehObj);

    //BroadcastBeacon.join();
    //NTEntryMaintenance.join();
    ClusterManagerTimerExp.join();
    CarMsgReceiveService.join();
    LocalInfotoAP.join();
    ClusterStateExpireThread.join();
    ProbeGPSInfo.join();
    ProbeSpeedInfo.join();
    ClusterService.join();
    CarExitStatus.join();

    std::cout << "------------------------------------------------------------------------\n";
    std::cout << "------------ Vehicle  " << VehObj->GetVehicleID() << "  exited from the network -----------" << std::endl;
    std::cout << "------------------------------------------------------------------------\n";

    delete NTObj;
    delete ClusterObj;
    delete LogsObj;
    delete LocalInfoObj;
    delete CARCommObj;
    delete VehObj;
    delete SendLocalInfoPacketSocket;
    delete SendHelloPacketSocket;

    return 0;
}
