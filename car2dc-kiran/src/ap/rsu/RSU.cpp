//
// Created by kiran.narayanaswamy on 8/13/18.
//

#include "../../header/ap/AP.h"

void T_MessageHandler(ACCESSPOINT* APObj);
void T_CTEntryTimer(ACCESSPOINT *, useconds_t);
void T_CreateClusters(ACCESSPOINT*);
void T_InfoPublisher(ACCESSPOINT *);
void T_ClusterChangeInfoPublisher(ACCESSPOINT *);


extern zmq::socket_t* RSUBeaconSocket;
extern zmq::socket_t* ClusterInfoSocket;

/* Creates socket used to send RSU beacons */
void CreateRSUBeaconsSocket(std::string IP, std::string Port) {
    zmq::context_t* Context = new zmq::context_t(1);
    RSUBeaconSocket = new zmq::socket_t(*Context, ZMQ_PUSH);
    RSUBeaconSocket->connect("tcp://" + IP + ":" + Port);
}

/* Creates socket used to send Cluster Info to cars */
void CreateClusterSocket(std::string IP, std::string Port) {
    zmq::context_t* Context = new zmq::context_t(1);
    ClusterInfoSocket = new zmq::socket_t(*Context, ZMQ_PUSH);
    ClusterInfoSocket->connect("tcp://" + IP + ":" + Port);
}

/**
	Periodically send RSU beacons
*/

void T_SendBeacons(ACCESSPOINT* APObj) {
    BASE_COMMUNICATION *BaseCommObj;

    RSUBEACON RSUBeaconPkt;
    RSUBeaconPkt.Dataline1 = "Hello, This is a RSU Beacon";
    RSUBeaconPkt.Dataline2 = "From "  + APObj->GetID();
    RSUBeaconPkt.SrcID = APObj->GetID();
    RSUBeaconPkt.X = APObj->GetX();
    RSUBeaconPkt.Y = APObj->GetY();
    RSUBeaconPkt.Datatype = "RSUBEACON";
    if(true)
        BaseCommObj = new COMMUNICATION("DEFAULT", APObj->LogsObj);

    while(!APObj->MCInfoMap.empty() && !APObj->NoMoreCars) {

        BaseCommObj->SendBroadcastMsg(APObj->GetID(), &RSUBeaconPkt, (char*)"A", "RSUBEACON");
        sleep(1);
    }
    delete BaseCommObj;
}


/*!
 * 'main()' function of the base station entity
 * @param argc 1
 * @param argv ID of the RSU
 * @return 0 on success, -1 on error
 */
int main(int argc, char *argv[]) {

    auto _LogsObj = new LOGGING_UTILITY(std::string(argv[1]), "ROADSIDEUNIT");
    auto _CTObj = new CLUSTERTABLE(_LogsObj);
    auto _StatsIntervalObj = new STATISTICS();
    _StatsIntervalObj->SetPublishClusterInfo(false);
    ACCESSPOINT APObj("RSU", argv[1], _LogsObj);

    APObj.LogsObj = _LogsObj;
    APObj.CTObj = _CTObj;
    APObj.StatsObj = _StatsIntervalObj;
    auto APCommObj = new COMMUNICATION(_LogsObj);
    std::string DCAppExec = "../bin/APDC ";
    DCAppExec += argv[1];
    DCAppExec += " ";
    DCAppExec += APObj.APIPInfo->IP + " ";
    DCAppExec += APObj.APIPInfo->Port;
    DCAppExec += " &";
    system(DCAppExec.c_str());
    CreateRSUBeaconsSocket(APCommObj->GetParserIP(), APCommObj->GetUDMPort());
    CreateClusterSocket(APCommObj->GetParserIP(), APCommObj->GetUDMPort());

    APObj.CommObj = APCommObj;
    APObj.LogsObj->LogInfo(std::string("Inside RoadSideUnit Module:: ID = ")+ argv[1]);
    APObj.LogsObj->LogDebug(std::string("Inside RoadSideUnit Module:: ID = ") + argv[1]);

    APObj.LogsObj->LogDebug(std::string("Creating RoadSideUnit server thread"));
    std::thread RSUServerService(T_MessageHandler, &APObj);

    APObj.LogsObj->LogDebug("Creating RSU Cluster Formation thread");
    std::thread MCServiceThread (T_CreateClusters, &APObj);

    APObj.LogsObj->LogDebug("Creating Cluster Table Expiry Timer thread");
    std::thread CTEntryExpireTimer(T_CTEntryTimer, &APObj, _CTObj->GetCTEntryTimer());

    APObj.LogsObj->LogDebug("Creating RSUBeacons thread");
    std::thread RSUBeacons(T_SendBeacons, &APObj);

	/* Publishes required information to visual component */
    APObj.LogsObj->LogDebug("Creating 'RSU Information Publisher' thread");
    std::thread InfoPublisher(T_InfoPublisher, &APObj);

	/* Publishes Cluster status info to applications (Data collection APP) */
    APObj.LogsObj->LogDebug("Creating 'RSU Cluster info Publisher' thread");
    std::thread APtoAppsInfoPublish(T_ClusterChangeInfoPublisher, &APObj);

    RSUServerService.join();
    MCServiceThread.join();
    CTEntryExpireTimer.join();
    RSUBeacons.join();
    InfoPublisher.join();
    APtoAppsInfoPublish.join();

    APObj.StatsObj->PrintAPStats(&APObj);


    std::cout << "------------------------------------------------------------------------\n";
    std::cout << "------------ ROADSIDEUNIT  " << APObj.GetID() << "  exited from the network -----------" << std::endl;
    std::cout << "------------------------------------------------------------------------\n";

    delete APObj.LogsObj;
    delete APObj.CommObj;
    delete APObj.CTObj;
    delete APObj.StatsObj;
    delete APObj.APIPInfo;

    return 0;
}
