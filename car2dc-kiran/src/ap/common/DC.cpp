//
// Created by kiran.narayanaswamy on 11/30/18.
//


#include "../../header/ap/DC.h"

void T_ListenToAP(AP_DATA_COLLECTION*);

int main(int argc, char* argv[]) {
    auto _LogsObj = new LOGGING_UTILITY(std::string(argv[1]), "BASESTATION", "     DC");
    auto CommObj = new COMMUNICATION(_LogsObj);
    auto StatsObj = new STATISTICS;
    auto IP_Port_Info = new struct IP_PORT_INFO();
    IP_Port_Info->IP = argv[2];
    IP_Port_Info->Port = argv[3];
    auto DCObj = new AP_DATA_COLLECTION(argv[1]);
    DCObj->LogsObj = _LogsObj;
    DCObj->CommObj = CommObj;
    DCObj->StatsObj = StatsObj;
    DCObj->IP_Port_Info = IP_Port_Info;
	/* Collects cluster information from clustering service in AP */
    std::thread ListenToAP(T_ListenToAP, DCObj);
    DCObj->DataCollectionService();

    delete _LogsObj;
    delete CommObj;
    delete StatsObj;
    delete DCObj;
    delete IP_Port_Info;
    return 0;
}

