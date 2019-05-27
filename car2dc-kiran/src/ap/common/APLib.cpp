
//
// Created by kiran on 9/22/18.


/*********************************************************************************
	Clustering service module in AP
	Collects vehicles local information
	Periodically creates clusters and distributes cluster information to cars

*********************************************************************************/

#include "../../header/ap/AP.h"
#include "../../../lib/chilkat/include/CkFileAccess.h"
#include "../../../lib/pugixml/pugixml.cpp"
#include "../../../lib/zmq/zhelpers.hpp"
#include <algorithm>

/* CT_Inuse is used to check if other threads are using cluster table (CT) */
bool CT_InUse = false;


/**
	Periodically publishes cluster status to subscribed applications
*/

void T_ClusterChangeInfoPublisher(ACCESSPOINT *APObj) {
    zmq::context_t Context(1);
    zmq::socket_t Publisher(Context, ZMQ_PUB);
    std::cout << "IP = =============== " << APObj->APIPInfo->IP << " Port =================== " <<
              std::to_string(std::stoi(APObj->APIPInfo->Port) + 3) << "\n";
    Publisher.bind("tcp://" + APObj->APIPInfo->IP + ":" + std::to_string(std::stoi(APObj->APIPInfo->Port) + 3));
    std::string Data;
    while (!APObj->NoMoreCars) {
        if(APObj->StatsObj->GetPublishClusterInfo()) {
            APtoApps ClusterChangeInfoObj;
            ClusterChangeInfoObj.set_clusterchange(true);
            ClusterChangeInfoObj.SerializeToString(&Data);
            s_sendmore(Publisher, "AP_TO_APPS");
            s_send(Publisher, Data);
        }
        sleep(1);
    }
}

/**
	Publish vehicles and cluster status information to visual component
	Vehicles information (id, location, speed, etc.) is published every second
	Cluster status information is published when changed
*/
	
void T_InfoPublisher(ACCESSPOINT *APObj) {
    zmq::context_t Context(1);
    zmq::socket_t Publisher(Context, ZMQ_PUB);
    std::cout << "IP = =============== " << APObj->APIPInfo->IP << " Port =================== " <<
              std::to_string(std::stoi(APObj->APIPInfo->Port) + 1) << "\n";
    Publisher.bind("tcp://"+APObj->APIPInfo->IP+":"+std::to_string(std::stoi(APObj->APIPInfo->Port) + 1));
    std::string DataToSend, PrevCH, ListOfAllCM, ListOfAllCH, ListOfAllCars;

    while(!APObj->NoMoreCars) {
        ListOfAllCars.clear();
        std::map<std::string, struct CTABLEENTRIES*> CT = APObj->CTObj->GetClusterTable();

        for (auto it = CT.begin(); it != CT.end(); it++) {
            struct CTABLEENTRIES* Veh = it->second;
            ListOfAllCars.append(it->first+",");
            ListOfAllCars.append(std::to_string(Veh->X) +",");
            ListOfAllCars.append(std::to_string(Veh->Y) +",");
            ListOfAllCars.append(std::to_string(Veh->Speed) +",");
            ListOfAllCars.append(std::to_string(Veh->Dir) +",");

            std::size_t found1 = ListOfAllCH.find("," + it->first+",");
            std::size_t found2 = ListOfAllCM.find("," + it->first+",");

            if(found1 != std::string::npos)
                ListOfAllCars.append("CH");
            else if(found2 != std::string::npos)
                ListOfAllCars.append("CM");
            else
                ListOfAllCars.append("INIT");
            ListOfAllCars.append(",");
        }

        CLUSTERINFO ClusterInfo;
        ClusterInfo.set_datatype("CARINFO");
        ClusterInfo.set_cmlist(ListOfAllCars);
        ClusterInfo.set_dataline1("Hello, This is Cars Information message");
        ClusterInfo.set_dataline2("From " + APObj->GetID());
        ClusterInfo.set_src_id(APObj->GetID());
        ClusterInfo.set_cluster_seq_num(APObj->StatsObj->ClusterSequenceNumber);

        ClusterInfo.SerializeToString(&DataToSend);
        s_sendmore(Publisher, "AP_INFO");
        s_send(Publisher, DataToSend);

	/* Checks if cluster status is changed. If yes publish it */
        if(!APObj->StatsObj->GetPublishClusterInfo()) {
            sleep(1);
            continue;
        }
        ListOfAllCM.clear();
        ListOfAllCH.clear();
        ListOfAllCH.append(",");
        ListOfAllCM.append(",");
        for (auto it = APObj->StatsObj->ListOfClusters.begin(); it != APObj->StatsObj->ListOfClusters.end(); it++) {
            std::string MC_ID = it->first;

            for (auto ait = it->second.begin(); ait != it->second.end(); ait++) {
                std::string CH = ait->first;

                if(APObj->StatsObj->ClusterSequenceNumber >= 10) {
                    if(APObj->StatsObj->PrevCH.empty()) {
                        APObj->StatsObj->CurrentCHWithCount[CH] = 1;
                        APObj->StatsObj->CountOfConsecutiveInterval[1] += 1;
                        APObj->StatsObj->PrevCH[MC_ID] = CH;
                    } else {
                        PrevCH = APObj->StatsObj->PrevCH[MC_ID];
                        if(CH == PrevCH) {
                            APObj->StatsObj->CountOfConsecutiveInterval[APObj->StatsObj->CurrentCHWithCount[CH]] -= 1;
                            APObj->StatsObj->CurrentCHWithCount[CH] += 1;
                            APObj->StatsObj->CountOfConsecutiveInterval[APObj->StatsObj->CurrentCHWithCount[CH]] += 1;
                        } else {
                            APObj->StatsObj->CurrentCHWithCount.erase(PrevCH);
                            APObj->StatsObj->CurrentCHWithCount[CH] = 1;
                            APObj->StatsObj->PrevCH[MC_ID] = CH;
                            APObj->StatsObj->CountOfConsecutiveInterval[1] +=1;
                        }
                    }
                }

                std::string ListOfCM = "";
                for (auto bit = ait->second.begin(); bit != ait->second.end(); bit++) {
                    auto duplicate = APObj->StatsObj->VehWhichMC.find(*bit);
                    if (duplicate == APObj->StatsObj->VehWhichMC.end()) {
                        ListOfCM.append(*bit);
                        ListOfCM.append(",");
                    } else {
                        if (MC_ID == ((APObj->StatsObj->VehWhichMC)[*bit])) {
                            ListOfCM.append(*bit);
                            ListOfCM.append(",");
                        }
                    }
                }

                ListOfAllCM.append(ListOfCM);
                ListOfAllCH.append(CH + ",");

                CLUSTERINFO ClusterInfo;
                ClusterInfo.set_ch(CH);
                ClusterInfo.set_datatype("CLUSTERINFO");
                ClusterInfo.set_clusterid(MC_ID);
                ClusterInfo.set_cmlist(ListOfCM);
                ClusterInfo.set_dataline1("Hello, This is a Cluster Info Information message");
                ClusterInfo.set_dataline2("From " + APObj->GetID());
                ClusterInfo.set_src_id(APObj->GetID());
                ClusterInfo.set_cluster_seq_num(APObj->StatsObj->ClusterSequenceNumber);

                std::cout << "CH List = " << ClusterInfo.ch() << "\n" << "CM List = " << ClusterInfo.cmlist() << "\n";

                ClusterInfo.SerializeToString(&DataToSend);
                s_sendmore(Publisher, "AP_INFO");
                s_send(Publisher, DataToSend);

            }
        }
        APObj->StatsObj->SetPublishClusterInfo(false);
        sleep(1);
    }
}

/** 
        Receiver created in Communication module to receive messages 
*/

void T_MessageHandler(ACCESSPOINT* APObj) {
    APObj->CommObj->ReceiveMessage((void*)APObj, APObj->GetID(), (char*)"B", "AP");
}


/**
	Process message received
	@param MessageReceived New message that is received
*/


void ACCESSPOINT::ProcessReceivedMsg(std::string MessageReceived) {
    MESSAGEFROMUDM Message;
    Message.ParseFromString(MessageReceived);

    std::string DataLine1, DataLine2, DataType, _ID, _Lane, SourceType;
    SourceType = Message.src_type();

    ALLDATA DataString = Message.data();
    DataLine1 = DataString.dataline1();
    DataLine2 = DataString.dataline2();
    DataType = DataString.datatype();

    /*! Checks the source type and data type and process it accordingly */
    if(SourceType == "CAR") {
        /*! If DataType is CLUSTERINFO, stores the details of vehicle id, its position and speed into a hashmap */
        if (DataType == "CLUSTERINFO") {
            UPDATE_LOCALINFO VehLocalInfo;
            VehLocalInfo.X = DataString.x();
            VehLocalInfo.Y = DataString.y();
            VehLocalInfo.Speed = DataString.speed();
            VehLocalInfo.Direction = DataString.dir();
            VehLocalInfo.SrcID = DataString.id();
            VehLocalInfo.Timestamp = DataString.timestamp();
            VehLocalInfo.Datatype = DataString.datatype();
            CTObj->AddToClusterTable(&VehLocalInfo, this);
        } 
    } else if(SourceType == "RSU") {

    } else if(SourceType == "BS") {

    }
    LogsObj->LogDebug(std::string("Messaged received is from source type " + Message.src_type()));
    LogsObj->LogInfo(std::string("Recieved msg:: ") + DataLine1 + " " + DataLine2 + std::string(" And this message was sent by sender at ") + DataString.timestamp());
    LogsObj->LogDebug(std::string("Recieved msg:: ") + DataLine1 + " " + DataLine2 + std::string(" And this message was sent by sender at ") + DataString.timestamp());
}


/**
	Periodically compute clusters and sends cluster information to cars
*/

void T_CreateClusters(ACCESSPOINT* APObj) {
    std::string ClusterAlgo = APObj->GetClusterAlgo();
	/*VehWhichMC is a map of <Vehicle ID, Micro cloud ID> created to resolve in case of a car belonging to multiple clusters */
    std::map<std::string, std::string> VehWhichMC;
    BASECLUSTER *BaseClusterObj;
    useconds_t interval;
    if(true) {
        BaseClusterObj = new MAPBASED_CLUSTER(APObj);
        interval = (useconds_t )BaseClusterObj->GetClusterInterval();
    }

    while(!APObj->MCInfoMap.empty() && !APObj->NoMoreCars) {
        int NumberOfTimesWait = 0;
	/* Same cluster table(CT) is accessed by different threads. Hence requires check if CT is currently used by any other thread */
        if(CT_InUse) {
            while (CT_InUse) {
                NumberOfTimesWait++;
                usleep(10000);
            }
        }
        CT_InUse = true;
        std::map<std::string, struct CTABLEENTRIES*> ClusterTable = APObj->CTObj->GetClusterTable();
        auto StartTime = std::chrono::high_resolution_clock::now();
	/* Computes clusters */

        BaseClusterObj->FormCluster(APObj, ClusterTable);
        auto EndTime = std::chrono::high_resolution_clock::now();
        if(APObj->StatsObj->ClusterSequenceNumber >= 10) {
            std::ofstream outputFile;
            outputFile.open("../results/logs/AvgClusterFormationTime.csv", std::fstream::app);
            outputFile << APObj->StatsObj->ClusterSequenceNumber << "," <<  ((std::chrono::duration_cast<std::chrono::microseconds>(EndTime - StartTime).count()) ) << std::endl;
            outputFile.close();
        }
	/* TieBreakBetweenMultipleMC is tested only with two Micro clouds hence not included in thesis */
        //BaseClusterObj->TieBreakBetweenMultipleMC(APObj, &VehWhichMC, ClusterTable);

	/* Send computed cluster information to cars */
        BaseClusterObj->DisseminateClusterInfo(APObj, &VehWhichMC);

        BaseClusterObj->ListOfClusters.clear();
        VehWhichMC.clear();
        APObj->StatsObj->SetPublishClusterInfo(true);
        CT_InUse = false;
        usleep(interval - (NumberOfTimesWait * 10000));
    }
}

/**
	Validates Cluster Table entries 
*/
void T_CTEntryTimer(ACCESSPOINT *APObj, useconds_t interval) {
    while(!APObj->NoMoreCars) {
        APObj->CTObj->ValidateCTEntries(APObj, interval);
        sleep(1);
    }
}

void CLUSTERTABLE::DeleteFromClusterTable(std::string VID, ACCESSPOINT * APObj) {
    if (CT_InUse) {
        while (CT_InUse) {
            usleep(10000);
        }
    }
    CT_InUse = true;
    delete APObj->CTObj->ClusterTable[VID];
    APObj->CTObj->ClusterTable.erase(VID);
    CT_InUse = false;
}

void CLUSTERTABLE::ValidateCTEntries(ACCESSPOINT *APObj, useconds_t interval) {
    std::map<std::string, struct CTABLEENTRIES*> CopyClusterMap;
    double ElapsedTime;
    time_t CurTime;
    CopyClusterMap = APObj->CTObj->GetClusterTable();
    time(&CurTime);
    for (auto it = CopyClusterMap.begin(); it != CopyClusterMap.end(); it++) {
        auto _CTObj = it->second;
        time_t EntryTime = _CTObj->Timestamp;
        ElapsedTime = difftime(CurTime, EntryTime);
        if (ElapsedTime > (double)interval) {
            APObj->LogsObj->LogDebug(std::string("Timer expired for Vehicle " + it->first +
                                                 " in the Cluster Table, Last Message received before = " +
                                                 std::to_string(ElapsedTime)));
            APObj->LogsObj->LogInfo(std::string("Timer expired for Vehicle " + it->first +
                                                " in the Cluster Table, Last Message received before = " +
                                                std::to_string(ElapsedTime)));
            APObj->CTObj->DeleteFromClusterTable(it->first, APObj);
        }
    }

}

double CalEuclidDistance(double X1, double Y1, double X2, double Y2) {
    return sqrt((X1 - X2) * (X1 - X2) + (Y1 - Y2) * (Y1 - Y2));
}

bool VehicleInRange(double X1, double Y1, double X2, double Y2, double Radius) {
    double distance = CalEuclidDistance(X1, Y1, X2, Y2);
    if(distance < Radius) {
        return true;
    }
    return false;
}

/**
	Computes clusters using MapBased clustering approach
*/

void MAPBASED_CLUSTER::FormCluster(ACCESSPOINT *APObj, std::map<std::string, struct CTABLEENTRIES*> CopyOfClusterTable) {

    double PrevEuclidDistance, CurEuclidDistance;
    std::string CH, MC_ID;

    for(auto it = APObj->MCInfoMap.begin(); it != APObj->MCInfoMap.end(); it++) {
        std::map<std::string, std::list<std::string>> Cluster;
        std::list<std::string> ClusterVehicles;
        MC_ID = it->first;
        PrevEuclidDistance = 9999999.9;
        std::cout << "Total entries in cluster table = " << CopyOfClusterTable.size() << std::endl;
        for (auto it = CopyOfClusterTable.begin(); it != CopyOfClusterTable.end(); it++) {
            struct CTABLEENTRIES *VehInfo = it->second;
            CurEuclidDistance = CalEuclidDistance(VehInfo->X, VehInfo->Y, APObj->MCInfoMap[MC_ID].MCPosX,
                                                  APObj->MCInfoMap[MC_ID].MCPosY);
            if (CurEuclidDistance <= APObj->MCInfoMap[MC_ID].MCRadius) {
                ClusterVehicles.push_front(VehInfo->VID);
                if (CurEuclidDistance < PrevEuclidDistance) {
                    PrevEuclidDistance = CurEuclidDistance;
                    CH = it->first;
                }
            }
        }
        if(!CH.empty()) {
            Cluster.insert(std::pair<std::string, std::list<std::string>>(CH, ClusterVehicles));
            ListOfClusters[MC_ID] = Cluster;
        }
        CH.clear();
        Cluster.clear();
        ClusterVehicles.clear();
    }
}

/*
void MAPBASED_CLUSTER::TieBreakBetweenMultipleMC(ACCESSPOINT *APObj, std::map<std::string, std::string>*VehWhichMC,
                                                 std::map<std::string, struct CTABLEENTRIES*> CopyOfClusterTable) {
    std::string MC_ID, RepeatedVehID, CH, VID;
    double PrevDistance, distance;
    double CHX, CHY, VehX, VehY;
    std::map<std::string, std::list<std::string>> Cluster;
    int NoOfMCVehBelongsTo;
    auto *Obj = new MICROCLOUD;
    std::map<std::string, std::string> AllMCVehBelongsTo;

    if(APObj->MCInfoMap.size() > 1) {
        for (auto it = ListOfClusters.begin(); it != ListOfClusters.end(); it++) {
            for (auto ait = it->second.begin(); ait != it->second.end(); ait++) {
                for (auto bit = ait->second.begin(); bit != ait->second.end(); bit++) {
                    std::string VID = *bit;
                    struct CTABLEENTRIES *VehInfo = CopyOfClusterTable[VID];
                    NoOfMCVehBelongsTo = 0;
                    for (auto cit = APObj->MCInfoMap.begin(); cit != APObj->MCInfoMap.end(); cit++) {
                        *Obj = cit->second;
                        if (VehicleInRange(VehInfo->X, VehInfo->Y, Obj->MCPosX, Obj->MCPosY,
                                           APObj->MCInfoMap[it->first].MCRadius)) {
                            AllMCVehBelongsTo[cit->first] = VehInfo->VID;
                            NoOfMCVehBelongsTo++;
                        }
                    }
                    //std::cout << VehInfo["ID"].asString() << " Belongs to " << NoOfMCVehBelongsTo << std::endl;
                    if (NoOfMCVehBelongsTo > 1) {
                        PrevDistance = 99999999.9;
                        for (auto cit = AllMCVehBelongsTo.begin(); cit != AllMCVehBelongsTo.end(); cit++) {
                            RepeatedVehID = cit->second;
                            MC_ID = cit->first;

                            for (auto it = ListOfClusters.begin(); it != ListOfClusters.end(); it++) {
                                if (it->first == MC_ID) {
                                    for (auto ait = it->second.begin(); ait != it->second.end(); ait++) {
                                        CH = ait->first;
                                        for (auto bit = ait->second.begin(); bit != ait->second.end(); bit++) {
                                            VID = *bit;
                                            struct CTABLEENTRIES *VehInfo = CopyOfClusterTable[VID];
                                            if (VehInfo->VID == RepeatedVehID) {
                                                VehX = VehInfo->X;
                                                VehY = VehInfo->Y;
                                            }
                                            if (VehInfo->VID == CH) {
                                                CHX = VehInfo->X;
                                                CHY = VehInfo->Y;
                                            }
                                        }
                                        distance = CalEuclidDistance(CHX, CHY, VehX, VehY);
                                        if (distance < PrevDistance) {
                                            PrevDistance = distance;
                                            VehWhichMC->insert(
                                                    std::pair<std::string, std::string>(RepeatedVehID, MC_ID));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    delete Obj;
    AllMCVehBelongsTo.clear();
}

*/

/**
	Sends Cluster information to all vehicles 
*/

void MAPBASED_CLUSTER::DisseminateClusterInfo (ACCESSPOINT *APObj, std::map<std::string, std::string>*VehWhichMC) {
    APObj->StatsObj->PublishClusterInfoToStats(ListOfClusters, VehWhichMC);
    std::string VID;
    BASE_COMMUNICATION *BaseCommObj;
    if(true)
        BaseCommObj = new COMMUNICATION("DEFAULT", APObj->LogsObj);
    for(auto it = ListOfClusters.begin(); it != ListOfClusters.end(); it++) {
        std::cout << "MC ID = " << it->first << std::endl;
        APObj->LogsObj->LogDebug("Micro cloud(cluster) ID = " + it->first);
        APObj->LogsObj->LogInfo("Micro cloud(cluster) ID = " + it->first);

        for (auto ait = it->second.begin(); ait != it->second.end(); ait++) {
            std::cout << "Cluster Head is " << ait->first << std::endl;
            APObj->LogsObj->LogDebug("Cluster Head = " + ait->first);
            APObj->LogsObj->LogInfo("Cluster Head = " + ait->first);
            APObj->StatsObj->NumberOfVehInClusters[APObj->StatsObj->ClusterSequenceNumber] += ait->second.size();
            APObj->StatsObj->NumberOfVehInMultipleClusters[it->first][APObj->StatsObj->ClusterSequenceNumber] += ait->second.size();

            for (auto bit = ait->second.begin(); bit != ait->second.end(); bit++) {
                std::string VID = *bit;
                struct CTABLEENTRIES *VehInfo = APObj->CTObj->GetClusterTable()[VID];
                APObj->LogsObj->LogDebug("The list has " + VehInfo->VID);
                APObj->LogsObj->LogInfo("The list has " + VehInfo->VID);
            }
            std::cout << std::endl;
        }
    }

    APObj->StatsObj->ClusterSequenceNumber++;

    //APObj->LogsObj->LogDebug("Vehicle repetitions in more than one MicroClouds:: ");
    //APObj->LogsObj->LogInfo("Vehicle repetitions in more than one MicroClouds:: ");

    /*for(auto it = VehWhichMC->begin(); it != VehWhichMC->end(); it++) {
        std::cout << " Vehid " << it->first << " belongs to " << it->second << std::endl;
        APObj->LogsObj->LogDebug("VehID = " + it->first + " belongs to " + it->second);
        //APObj->LogsObj->LogInfo("VehID = " + it->first + " belongs to " + it->second);

    }*/


    for(auto it = ListOfClusters.begin(); it != ListOfClusters.end(); it++) {
        std::string MC_ID = it->first;
        for (auto ait = it->second.begin(); ait != it->second.end(); ait++) {
            std::string CH = ait->first;
            std::string ListOfCM = "";
            for (auto bit = ait->second.begin(); bit != ait->second.end(); bit++) {
                if(APObj->MCInfoMap.size() > 1) {
                    auto it = VehWhichMC->find(*bit);
                    if (it == VehWhichMC->end()) {
                        ListOfCM.append(*bit);
                        ListOfCM.append(" ");
                    } else {
                        if (MC_ID == ((*VehWhichMC)[*bit])) {
                            ListOfCM.append(*bit);
                            ListOfCM.append(" ");
                        }
                    }
                } else {
                    ListOfCM.append(*bit);
                    ListOfCM.append(" ");
                }
            }
            CLUSTER_INFO ClusterData;
            ClusterData.Dataline1 = "Hello, This is a Cluster Info Information UNICAST message";
            ClusterData.Dataline2 = "From " + APObj->GetID();
            ClusterData.CH = CH;
            ClusterData.ListOfCM = ListOfCM;
            ClusterData.MC_ID = MC_ID;
            ClusterData.Datatype = "CLUSTERINFO";
            ClusterData.Timestamp = APObj->LogsObj->GetCurrentTime();
	        ClusterData.ClusterSeqNum = APObj->StatsObj->ClusterSequenceNumber;

            for (auto bit = ait->second.begin(); bit != ait->second.end(); bit++) {
                VID = *bit;
                auto ct = VehWhichMC->find(VID);
                if( ct == VehWhichMC->end()) {
                    BaseCommObj->SendUnicastMsg(APObj->GetID(), VID, &ClusterData, (char*)"B", "CLUSTERINFO");
                    APObj->LogsObj->LogDebug("Sent Cluster Info:: Micro cloud ID = " + MC_ID + " VID = " + VID);
                } else {
                    if(MC_ID == ct->second) {
                        BaseCommObj->SendUnicastMsg(APObj->GetID(), VID, &ClusterData, (char*)"B", "CLUSTERINFO");
                        APObj->LogsObj->LogDebug("Sent Cluster Info:: Micro cloud ID = " + MC_ID + " VID = " + VID);
                    }
                }
            }
        }
    }
    delete BaseCommObj;
}


/** 
        Constructor that reads XML Config file to populate required variables 
*/

CLUSTERTABLE::CLUSTERTABLE(LOGGING_UTILITY *LogsObj) {
    pugi::xml_document Doc;
    std::string TagName, AttrName, Attrvalue, Manager;
    if (!Doc.load_file("../config/ap/config.xml")) {
        LogsObj->LogError(std::string("ERROR:: Config File Missing ../config/ap/config.xml !!\n")) ;
    } else {
        pugi::xml_node tools = Doc.child("VMCPI").child("config");
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            TagName = it->name();
            std::cout << "Name, Value = " << TagName << std::endl;
            if (TagName == "CLUSTER_ALGORITHM") {
                for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                    AttrName = ait->name();
                    Attrvalue = ait->value();
                    if (AttrName == "entrytimer")
                        CTEntryTimer = (clock_t) std::stod(Attrvalue);
                }
            }
        }
    }
}


/*!
 * Updates required parameter values from the config xml file
 * @param ID 'ID' of the Base Station whose details to be extracted from the xml
 * @return true on success, false on failure
 */
ACCESSPOINT::ACCESSPOINT(std::string AP, char* _ID, LOGGING_UTILITY *LogsObj) {
    NoMoreCars = false;
    APType = AP;
    auto IPInfo = new AP_IP_INFO();
    APIPInfo = IPInfo;
    pugi::xml_document Doc;
    if (!Doc.load_file("../config/ap/config.xml")) {
        LogsObj->LogError(std::string("ERROR:: Config File Missing ../config/ap/config.xml !!\n")) ;
    }

    else {
        LogsObj->LogDebug(std::string("Parsing config file ..."));
        pugi::xml_node tools = Doc.child("VMCPI").child("config");
        bool Parse = false;
        std::string Fname, Sname, Value;
        std::string AttrName, Attrvalue, Manager;
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            Fname = it->name();
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                Sname = ait->name();
                Value = ait->value();
                if (Value == _ID || Parse)
                    Parse = true;
                if (Fname == AP && Parse) {
                    if (Sname == "id")
                        ID = Value;
                    else if (Sname == "ip")
                        APIPInfo->IP = Value;
                    else if (Sname == "port")
                        APIPInfo->Port = Value;
                    else if (Sname == "x")
                        X = std::stod(Value);
                    else if (Sname == "y") {
                        Y = std::stod(Value);
                        Parse = false;
                    }
                } else if (Fname == "CLUSTER_ALGORITHM") {
                    if (Sname == "clust_algo") {
                        ClusterAlgo = Value;
                    }
                } else if(Fname == "MICROCLOUD") {
                    MICROCLOUD NewObj;
                    for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                        AttrName = ait->name();
                        Attrvalue = ait->value();
                        std::cout << "Name, Value = " << AttrName << " " << Attrvalue << std::endl;
                        if (AttrName == "id")
                            NewObj.MID = Attrvalue;
                        else if (AttrName == "x")
                            NewObj.MCPosX = std::stod(Attrvalue);
                        else if (AttrName == "y")
                            NewObj.MCPosY = std::stod(Attrvalue);
                        else if (AttrName == "manager")
                            Manager = Attrvalue;
                        else if (AttrName == "mc_radius") {
                            NewObj.MCRadius = std::stod(Attrvalue);
                        }
                    }
                    if (Manager == _ID) {
                        MCInfoMap[NewObj.MID] = NewObj;
                    }
                }
            }
        }
    }
    LogsObj->LogDebug(std::string("ID = " + ID + " X = " + std::to_string(X) + " Y = " + std::to_string(Y)));
}

/** 
        Constructor that reads XML Config file to populate required variables 
*/

MAPBASED_CLUSTER::MAPBASED_CLUSTER(ACCESSPOINT *APObj) {
    pugi::xml_document doc;
    std::string TagName, AttrName, Attrvalue, Manager;
    if (!doc.load_file("../config/ap/config.xml")) {
        APObj->LogsObj->LogError("Config file '../config/ap/config.xml' do not exist");
    } else {
        pugi::xml_node tools = doc.child("VMCPI").child("config");
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            TagName = it->name();
            std::cout << "Name, Value = " << TagName << std::endl;
            if(TagName == "CLUSTER_ALGORITHM") {
                for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                    AttrName = ait->name();
                    Attrvalue = ait->value();
                    if(AttrName == "clust_interval")
                        SetClusterInterval(std::stod(Attrvalue) * 1000000);
                }
            }
        }
    }
}



/**
	Update the detals of vehicles local info if vehicle ID already exists
	or add new vehicle's information as a new entry
*/


void CLUSTERTABLE::AddToClusterTable(UPDATE_LOCALINFO* DataOutput, ACCESSPOINT *APObj) {
	bool Alloc_Success = true;
    if (!(DataOutput->X == -1.0 || DataOutput->Y == -1.0) ) {
        CTABLEENTRIES *_CTObj;
	/* Same cluster table(CT) is accessed by different threads. Hence requires check if CT is currently used by any other thread */
        if (CT_InUse) {
            while (CT_InUse) {
                usleep(10000);
            }
        }
        CT_InUse = true;

	/* Adding new entry */
        if ( ClusterTable.find(DataOutput->SrcID) == ClusterTable.end() ) {

            try {
                    _CTObj = new struct CTABLEENTRIES;
            } catch (const std::bad_alloc& e) {
                    std::cout << "Allocation failed: " << e.what() << '\n';
                    Alloc_Success = false;
            } catch (std::exception &e) {
                    std::cout << "Catching exception: " << e.what() << std::endl;
                    Alloc_Success = false;
            }
        } else {
		/* Update existing */
            _CTObj = ClusterTable[DataOutput->SrcID];
        }

	/* If memory allocation is succesful */
        if (Alloc_Success) {
                time_t CurTime;
                time(&CurTime);
                _CTObj->X = DataOutput->X;
                _CTObj->Y = DataOutput->Y;
                _CTObj->Speed = DataOutput->Speed;
                _CTObj->Dir = DataOutput->Direction;
                _CTObj->VID = DataOutput->SrcID;
                _CTObj->Timestamp = CurTime;
                ClusterTable[_CTObj->VID] = _CTObj;
                APObj->LogsObj->LogInfo(std::string("Updated Cluster Hash Map for Vehicle " + _CTObj->VID));
                APObj->LogsObj->LogDebug(std::string("Updated Cluster Hash Map for Vehicle " + _CTObj->VID));
                APObj->LogsObj->LogDebug(std::string("Updated Cluster Hash Table Contains following  data "));
                APObj->LogsObj->LogDebug(std::string(
                    "-----------------------------------------------------------------------------------------------------"));
                for(auto it = ClusterTable.begin(); it != ClusterTable.end(); it++) {
                    _CTObj = it->second;
                    APObj->LogsObj->LogDebug(
                        "ID = " + it->first + ", x = " + std::to_string(_CTObj->X) + ", y = " + std::to_string(_CTObj->Y) +
                        " Speed = " + std::to_string(_CTObj->Speed) + " Dircetion = " + std::to_string(_CTObj->Dir));
                 APObj->LogsObj->LogDebug(std::string(
                        "-----------------------------------------------------------------------------------------------------"));
                }
            APObj->LogsObj->LogInfo("Total entries = " + std::to_string(ClusterTable.size()));
        }
        CT_InUse = false;
    }
}
