
//
// Created by kiran on 9/22/18.

#include "../../header/ap/AP.h"
#include "../../../lib/chilkat/include/CkFileAccess.h"
#include "../../../lib/pugixml/pugixml.cpp"
#include "../../../lib/zmq/zhelpers.hpp"


extern zmq::socket_t* ClusterInfoSocket;

void T_InfoPublisher(ACCESSPOINT *APObj) {
    zmq::context_t Context(1);
    zmq::socket_t Publisher(Context, ZMQ_PUB);
    std::cout << "Ip = =============== " << APObj->APIPInfo->IP << " Porttttttttttttttttttt = " <<
              std::to_string(std::stoi(APObj->APIPInfo->Port) + 1) << "\n";
    Publisher.bind("tcp://"+APObj->APIPInfo->IP+":"+std::to_string(std::stoi(APObj->APIPInfo->Port) + 1));
    std::string DataToSend, PrevCH;

    while(!APObj->NoMoreCars) {
        if(!APObj->StatsObj->GetPublishClusterInfo())
            continue;
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
                        ListOfCM.append(" ");
                    } else {
                        if (MC_ID == ((APObj->StatsObj->VehWhichMC)[*bit])) {
                            ListOfCM.append(*bit);
                            ListOfCM.append(" ");
                        }
                    }
                }

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
                s_sendmore(Publisher, "APP");
                s_send(Publisher, DataToSend);

            }
        }
        APObj->StatsObj->SetPublishClusterInfo(false);
    }
}
void T_CreateWorkerThreads(std::string);
void T_MessageHandler(ACCESSPOINT* APObj) {
    //std::thread CreateWorkerThreads(T_CreateWorkerThreads, "AP");
    APObj->CommObj->ReceiveMessage((void*)APObj, APObj->GetID(), (char*)"B", "AP");
    //CreateWorkerThreads.join();
}


void T_ProcessMessage(ACCESSPOINT *APObj, std::string MessageReceived) {

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
            APObj->CTObj->AddToClusterTable(&VehLocalInfo, APObj);
        } else if(DataType == "CHDATA") {
		std::cout << " I am hereeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n";
            APObj->DCObj->LogsObj->LogDebug("Received Data from " + Message.src_id());
            APObj->DCObj->LogsObj->LogInfo("Received Data from " + Message.src_id());
	    if(APObj->StatsObj->ClusterSequenceNumber >= 10) {
            	APObj->StatsObj->TotalDataReceived += DataString.data().size();
            	APObj->LogsObj->LogStats("Total Data Received = " + std::to_string(APObj->StatsObj->TotalDataReceived));
	    }
            APObj->DCObj->StoreData(&DataString);
        }
    } else if(SourceType == "RSU") {

    } else if(SourceType == "BS") {

    }
    APObj->LogsObj->LogDebug(std::string("Messaged received is from source type " + Message.src_type()));
    APObj->LogsObj->LogInfo(std::string("Recieved msg:: ") + DataLine1 + " " + DataLine2 + std::string(" And this message was sent by sender at ") + DataString.timestamp());
    APObj->LogsObj->LogDebug(std::string("Recieved msg:: ") + DataLine1 + " " + DataLine2 + std::string(" And this message was sent by sender at ") + DataString.timestamp());
}

/*!
 * Process UDM messages
 * @param message_received Message recieved from UDM
 */
/*void ACCESSPOINT::ProcessReceivedMsg(std::string MessageReceived) {
    std::thread ProcessMessage(T_ProcessMessage, this, MessageReceived);
    ProcessMessage.join();
}*/

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
        } else if(DataType == "CHDATA") {
                std::cout << " I am hereeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n";
                DCObj->LogsObj->LogDebug("Received Data from " + Message.src_id());
                DCObj->LogsObj->LogInfo("Received Data from " + Message.src_id());
            if(StatsObj->ClusterSequenceNumber >= 10) {
                StatsObj->TotalDataReceived += DataString.data().size();
                LogsObj->LogStats("Total Data Received = " + std::to_string(StatsObj->TotalDataReceived));
            }
            DCObj->StoreData(&DataString);
        }
    } else if(SourceType == "RSU") {

    } else if(SourceType == "BS") {

    }
    LogsObj->LogDebug(std::string("Messaged received is from source type " + Message.src_type()));
    LogsObj->LogInfo(std::string("Recieved msg:: ") + DataLine1 + " " + DataLine2 + std::string(" And this message was sent by sender at ") + DataString.timestamp());
    LogsObj->LogDebug(std::string("Recieved msg:: ") + DataLine1 + " " + DataLine2 + std::string(" And this message was sent by sender at ") + DataString.timestamp());
}


void DATA_COLLECTION::StoreData(ALLDATA *DataString) {
    unsigned long long Start;
    //std::cout << "Filename " << DataString->filename() << std::endl;
    //std::cout << "ChunkNum " << DataString->chunknum() << std::endl;
    //std::cout << "LastPKT ?? " << DataString->lastpkt() << std::endl;
    //std::cout << "Data " << DataString->data().size() << std::endl;
    /*if(DataString->chunknum() == 1) {
        //std::chrono::time_point<std::chrono::system_clock> Start, End;
        Start = std::stoull(DataString->start_time());
        struct timeval tv;
        gettimeofday(&tv, NULL);
        unsigned long long millisecondsSinceEpoch =
                (unsigned long long)(tv.tv_sec) * 1000 +
                (unsigned long long)(tv.tv_usec) / 1000;
        unsigned long long End = millisecondsSinceEpoch;
        auto ElapsedTime = End - Start;
        //std::cout << "Elapsedddddddd timeeeeee = " << ElapsedTime << "\n";
        //LogsObj->LogStats("Time taken for first packet to travel " + std::to_string(ElapsedTime));
    }*/
    std::string OutputFilename = APDataFilePath + DataString->filename() + std::to_string(DataString->chunknum())+".part";
    std::ofstream Out(OutputFilename);
    std::string Data = DataString->data();
    Out << Data;
    Out.close();
    if(DataString->lastpkt()) {
        /*struct timeval tv;
        gettimeofday(&tv, NULL);
        unsigned long long millisecondsSinceEpoch =
                (unsigned long long)(tv.tv_sec) * 1000 +
                (unsigned long long)(tv.tv_usec) / 1000;
        unsigned long long End = millisecondsSinceEpoch;
        auto ElapsedTime = End - Start;
        std::cout << "Elapsedddddddd timeeeeee = " << ElapsedTime << "\n";
        LogsObj->LogDebug("Time taken for complete packet to travel " + std::to_string(ElapsedTime));*/


        CkFileAccess fac;
        std::string reassembledFilename = APDataFilePath + DataString->filename();
        const char *partPrefix = DataString->filename().c_str();
        const char *partExtension = "part";

        int Success = fac.ReassembleFile(APDataFilePath.c_str(),partPrefix,partExtension,reassembledFilename.c_str());
        if (Success) {
            std::cout << "Success.................................................... AP" << "\r\n";
        }
        else {
            std::cout << fac.lastErrorText() << "\r\n";
            LogsObj->LogError(fac.lastErrorText());
        }
        int Count = 1;
        while(Count <= DataString->chunknum()) {
            std::string RemoveFile = APDataFilePath + DataString->filename() + std::to_string(Count) + ".part";
            remove(RemoveFile.c_str());
            Count++;
        }
    }
}



void T_CreateClusters(ACCESSPOINT* APObj) {
    std::string ClusterAlgo = APObj->GetClusterAlgo();
    std::map<std::string, std::string> VehWhichMC;
    BASECLUSTER *BaseClusterObj;
    useconds_t interval;
    if(ClusterAlgo == "MapBased") {
        BaseClusterObj = new MAPBASED_CLUSTER(APObj);
        interval = (useconds_t )BaseClusterObj->GetClusterInterval();
    }

    while(!APObj->MCInfoMap.empty() && !APObj->NoMoreCars) {
        std::map<std::string, struct CTABLEENTRIES*> CopyOfClusterTable = APObj->CTObj->GetClusterTable();
        clock_t t = clock();
        BaseClusterObj->FormCluster(APObj, CopyOfClusterTable);
        BaseClusterObj->TieBreakBetweenMultipleMC(APObj, &VehWhichMC, CopyOfClusterTable);
        t = clock() - t;
	if(APObj->StatsObj->ClusterSequenceNumber >= 10) {
        	APObj->StatsObj->TotalClusterFormationTime += ((float)t)/CLOCKS_PER_SEC;
	}
        BaseClusterObj->DisseminateClusterInfo(APObj, &VehWhichMC);
        BaseClusterObj->ListOfClusters.clear();
        VehWhichMC.clear();
        usleep(interval);
    }
}


void T_CTEntryTimer(ACCESSPOINT *APObj, useconds_t interval) {
    while(!APObj->NoMoreCars) {
        APObj->CTObj->ValidateCTEntries(APObj, interval);
        usleep(50000);
    }
}

void CLUSTERTABLE::DeleteFromClusterTable(std::string VID, ACCESSPOINT * APObj) {
    delete APObj->CTObj->ClusterTable[VID];
    APObj->CTObj->ClusterTable.erase(VID);
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


void MAPBASED_CLUSTER::FormCluster(ACCESSPOINT *APObj, std::map<std::string, struct CTABLEENTRIES*> CopyOfClusterTable) {
    std::list<std::string> ClusterVehicles;
    double PrevEuclidDistance, CurEuclidDistance;
    std::string CH, MC_ID;
    std::map<std::string, std::list<std::string>> Cluster;
    for(auto it = APObj->MCInfoMap.begin(); it != APObj->MCInfoMap.end(); it++) {
        MC_ID = it->first;
        PrevEuclidDistance = 9999999.9;
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


void MAPBASED_CLUSTER::TieBreakBetweenMultipleMC(ACCESSPOINT *APObj, std::map<std::string, std::string>*VehWhichMC,
                                                 std::map<std::string, struct CTABLEENTRIES*> CopyOfClusterTable) {
    std::string MC_ID, RepeatedVehID, CH, VID;
    double PrevDistance, distance;
    double CHX, CHY, VehX, VehY;
    std::map<std::string, std::list<std::string>> Cluster;
    int NoOfMCVehBelongsTo;
    auto *Obj = new MICROCLOUD;
    std::map<std::string, std::string> AllMCVehBelongsTo;


    for(auto it = ListOfClusters.begin(); it != ListOfClusters.end(); it++) {
        for (auto ait = it->second.begin(); ait != it->second.end(); ait++) {
            for (auto bit = ait->second.begin(); bit != ait->second.end(); bit++) {
                std::string VID = *bit;
                struct CTABLEENTRIES *VehInfo = CopyOfClusterTable[VID];
                NoOfMCVehBelongsTo = 0;
                for (auto cit = APObj->MCInfoMap.begin(); cit != APObj->MCInfoMap.end(); cit++) {
                    *Obj = cit->second;
                    if (VehicleInRange(VehInfo->X, VehInfo->Y, Obj->MCPosX, Obj->MCPosY, APObj->MCInfoMap[it->first].MCRadius)) {
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
                                        VehWhichMC->insert(std::pair<std::string, std::string>(RepeatedVehID, MC_ID));
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



void MAPBASED_CLUSTER::DisseminateClusterInfo (ACCESSPOINT *APObj, std::map<std::string, std::string>*VehWhichMC) {
    APObj->StatsObj->PublishClusterInfoToStats(ListOfClusters, VehWhichMC);
    std::string VID;
    BASE_COMMUNICATION *BaseCommObj;
    for(auto it = ListOfClusters.begin(); it != ListOfClusters.end(); it++) {
        std::cout << "MC ID = " << it->first << std::endl;
        APObj->LogsObj->LogDebug("Micro cloud(cluster) ID = " + it->first);
        APObj->LogsObj->LogInfo("Micro cloud(cluster) ID = " + it->first);

        for (auto ait = it->second.begin(); ait != it->second.end(); ait++) {
            std::cout << "Cluster Head is " << ait->first << std::endl;
            APObj->LogsObj->LogDebug("Cluster Head = " + ait->first);
            APObj->LogsObj->LogInfo("Cluster Head = " + ait->first);
            APObj->StatsObj->NumberOfVehInClusters[APObj->StatsObj->ClusterSequenceNumber] += ait->second.size();

            for (auto bit = ait->second.begin(); bit != ait->second.end(); bit++) {
                std::string VID = *bit;
                struct CTABLEENTRIES *VehInfo = APObj->CTObj->GetClusterTable()[VID];
                //std::cout << "The list has " << VehInfo->VID << std::endl;
                APObj->LogsObj->LogDebug("The list has " + VehInfo->VID);
                APObj->LogsObj->LogInfo("The list has " + VehInfo->VID);

            }
            std::cout << std::endl;
        }
    }

    APObj->StatsObj->ClusterSequenceNumber++;

    std::cout << "Repetitions size = " << VehWhichMC->size() << " " << std::endl;
    APObj->LogsObj->LogDebug("Vehicle repetitions in more than one MicroClouds:: ");
    //APObj->LogsObj->LogInfo("Vehicle repetitions in more than one MicroClouds:: ");

    for(auto it = VehWhichMC->begin(); it != VehWhichMC->end(); it++) {
        std::cout << " Vehid " << it->first << " belongs to " << it->second << std::endl;
        APObj->LogsObj->LogDebug("VehID = " + it->first + " belongs to " + it->second);
        //APObj->LogsObj->LogInfo("VehID = " + it->first + " belongs to " + it->second);

    }


    for(auto it = ListOfClusters.begin(); it != ListOfClusters.end(); it++) {
        std::string MC_ID = it->first;
        for (auto ait = it->second.begin(); ait != it->second.end(); ait++) {
            std::string CH = ait->first;
            std::string ListOfCM = "";
            for (auto bit = ait->second.begin(); bit != ait->second.end(); bit++) {
                auto it = VehWhichMC->find(*bit);
                if( it == VehWhichMC->end()) {
                    ListOfCM.append(*bit);
                    ListOfCM.append(" ");
                } else{
                    if(MC_ID == ((*VehWhichMC)[*bit])) {
                        ListOfCM.append(*bit);
                        ListOfCM.append(" ");
                    }
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
                struct CTABLEENTRIES *VehInfo = APObj->CTObj->GetClusterTable()[VID];
                VID = VehInfo->VID;
                auto ct = VehWhichMC->find(VID);
                if( ct == VehWhichMC->end()) {
                    if(true)
                        BaseCommObj = new COMMUNICATION("DEFAULT", APObj->LogsObj);
                    BaseCommObj->SendUnicastMsg(APObj->GetID(), VID, &ClusterData, (char*)"B", "CLUSTERINFO");
                    delete BaseCommObj;
                    std::cout << "Sent ClusterInfo MC " << MC_ID << " VID " << VID << std::endl;
                    APObj->LogsObj->LogDebug("Sent Cluster Info:: Micro cloud ID = " + MC_ID + " VID = " + VID);
                } else {
                    if(MC_ID == ct->second) {
                        if(true)
                            BaseCommObj = new COMMUNICATION("DEFAULT", APObj->LogsObj);
                        BaseCommObj->SendUnicastMsg(APObj->GetID(), VID, &ClusterData, (char*)"B", "CLUSTERINFO");
                        delete BaseCommObj;
                        std::cout << "Sending ClusterInfo MC " << MC_ID << " VID " << VID << std::endl;
                        APObj->LogsObj->LogDebug("Sent Cluster Info:: Micro cloud ID = " + MC_ID + " VID = " + VID);
                    }
                }
            }
        }
    }
}


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

DATA_COLLECTION::DATA_COLLECTION(std::string ID, std::string Modulename) {
    LogsObj = new LOGGING_UTILITY(ID, Modulename, "DCA");
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
}






void CLUSTERTABLE::AddToClusterTable(UPDATE_LOCALINFO* DataOutput, ACCESSPOINT *APObj) {

    auto _CTObj = new struct CTABLEENTRIES;
    std::map<std::string, struct CTABLEENTRIES*> CopyOfCT;
    time_t CurTime;
    time(&CurTime);
    _CTObj->X = DataOutput->X;
    _CTObj->Y = DataOutput->Y;
    _CTObj->Speed = DataOutput->Speed;
    _CTObj->Dir = DataOutput->Direction;
    _CTObj->VID = DataOutput->SrcID;
    _CTObj->Timestamp = CurTime;
    if (!(_CTObj->X == -1.0 || _CTObj->Y == -1.0) ) {
        ClusterTable[_CTObj->VID] = _CTObj;
        CopyOfCT = ClusterTable;
        APObj->LogsObj->LogInfo(std::string("Updated Cluster Hash Map for Vehicle " + _CTObj->VID));
        APObj->LogsObj->LogDebug(std::string("Updated Cluster Hash Map for Vehicle " + _CTObj->VID));
        APObj->LogsObj->LogDebug(std::string("Updated Cluster Hash Table Contains following  data "));
        APObj->LogsObj->LogDebug(std::string(
                "-----------------------------------------------------------------------------------------------------"));
        for(auto it = CopyOfCT.begin(); it != CopyOfCT.end(); it++) {
            _CTObj = it->second;
            APObj->LogsObj->LogDebug(
                    "ID = " + it->first + ", x = " + std::to_string(_CTObj->X) + ", y = " + std::to_string(_CTObj->Y) +
                    " Speed = " + std::to_string(_CTObj->Speed) + " Dircetion = " + std::to_string(_CTObj->Dir));
            APObj->LogsObj->LogDebug(std::string(
                    "-----------------------------------------------------------------------------------------------------"));
        }
    }
}
