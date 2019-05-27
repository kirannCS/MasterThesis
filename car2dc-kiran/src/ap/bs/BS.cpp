//
// Created by kiran.narayanaswamy on 8/9/18.
//

#include "../../header/ap/AP.h"

void T_MessageHandler(ACCESSPOINT* APObj);
void T_CTEntryTimer(ACCESSPOINT *, useconds_t);
void T_CreateClusters(ACCESSPOINT*);
void T_InfoPublisher(ACCESSPOINT *);
void T_ClusterChangeInfoPublisher(ACCESSPOINT *);

extern zmq::socket_t* ClusterInfoSocket;

/* Creates socket used to send Cluster Info to cars */
void CreateClusterSocket(std::string IP, std::string Port) {
    zmq::context_t* Context = new zmq::context_t(1);
    ClusterInfoSocket = new zmq::socket_t(*Context, ZMQ_PUSH);
    ClusterInfoSocket->connect("tcp://" + IP + ":" + Port);
}


/*!
 * 'main()' function of the base station entity
 * @param argc 1
 * @param argv ID of the Base Station
 * @return 0 on success, -1 on error
 */
int main(int argc, char *argv[]) {

    auto _LogsObj = new LOGGING_UTILITY(std::string(argv[1]), "BASESTATION");
    auto _CTObj = new CLUSTERTABLE(_LogsObj);
    //auto _DCObj = new DATA_COLLECTION(std::string(argv[1]), "BASESTATION");
    auto _StatsIntervalObj = new STATISTICS();
    _StatsIntervalObj->SetPublishClusterInfo(false);
    ACCESSPOINT APObj("BS", argv[1], _LogsObj);

    //APObj.DCObj = _DCObj;
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
    CreateClusterSocket(APCommObj->GetParserIP(), APCommObj->GetUDMPort());
    APObj.CommObj = APCommObj;


    APObj.LogsObj->LogInfo(std::string("Inside Cellular Base Station Module:: ID = ")+ argv[1]);
    APObj.LogsObj->LogDebug(std::string("Inside Cellular Base Station Module:: ID = ") + argv[1]);

    APObj.LogsObj->LogDebug(std::string("Creating Base Station server thread"));
    std::thread BSServerService(T_MessageHandler, &APObj);

    APObj.LogsObj->LogDebug("Creating Base Station Cluster Formation thread");
    std::thread MCServiceThread(T_CreateClusters, &APObj);

    APObj.LogsObj->LogDebug("Creating Cluster Table Expiry Timer thread " + std::to_string((useconds_t)_CTObj->GetCTEntryTimer()));
    std::thread CTEntryExpireTimer(T_CTEntryTimer, &APObj, (useconds_t)_CTObj->GetCTEntryTimer());

	/* Publishes required information to visual component */
    APObj.LogsObj->LogDebug("Creating 'Base Station Information Publisher' thread");
    std::thread InfoPublisher(T_InfoPublisher, &APObj);

	/* Publishes Cluster status info to applications (Data collection APP) */
    std::thread APtoAppsInfoPublish(T_ClusterChangeInfoPublisher, &APObj);

    BSServerService.join();
    MCServiceThread.join();
    CTEntryExpireTimer.join();
    InfoPublisher.join();
    APtoAppsInfoPublish.join();

	APObj.StatsObj->PrintAPStats(&APObj);

    std::cout << "------------------------------------------------------------------------\n";
    std::cout << "------------ Basestation  " << APObj.GetID() << "  exited from the network -----------" << std::endl;
    std::cout << "------------------------------------------------------------------------\n";

    delete APObj.LogsObj;
    delete APObj.CommObj;
    delete APObj.CTObj;
    //delete APObj.DCObj;
    delete APObj.StatsObj;
    delete APObj.APIPInfo;


    return 0;
}
