//
// Created by kiran on 9/22/18.
//

#ifndef VMCPI_AP_H
#define VMCPI_AP_H

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <list>

#include "../../utilities/Logging.h"
#include "../../utilities/Stats.h"

#include "../../utilities/Communication.h"
#include "../../packets/header/BigData.pb.h"
#include "../../packets/header/MessageForwardFromUDM.pb.h"
#include "../../packets/header/MsgFromNodeToUDM.pb.h"
#include "../../packets/header/ClusterInfo.pb.h"
#include "../../packets/header/APtoApps.pb.h"
#include "../common/ClusterInfo.h"
#include "../common/LocalInfo.h"
#include "../common/RSUBeacon.h"
#include <chrono>


class CLUSTERTABLE;

class MAPBASED_CLUSTER;

class DATA_COLLECTION;

/* Micro cloud details are stored using this structure */
struct MICROCLOUD {

    std::string MID;

    double MCPosX;

    double MCPosY;

    double MCRadius;

};

/* AP's IP address and starting port address is stored in this class */
class AP_IP_INFO {
public:
    std::string IP;
    std::string Port;
};

/* AP's main class that holds the details of an AP */
class ACCESSPOINT {
private:
    std::string ID;
    double X;
    double Y;
	/* Clustering approach deployed */
    std::string ClusterAlgo;
	/* APType indicates 'BS" or 'RSU' */
    std::string APType;

public:
    ACCESSPOINT(std::string, char *, LOGGING_UTILITY *);
	/* Contains information of all the micro clouds that this AP is managing */
    std::map<std::string, MICROCLOUD> MCInfoMap;
	/* Pointer to cluster table, where information of all vehicles are stored */
    CLUSTERTABLE *CTObj;
    std::string GetAPType() {
        return APType;
    }
    std::string GetID(){
        return ID;
    }
    double GetX() {
        return X;
    }
    double GetY() {
        return Y;
    }
    std::string GetClusterAlgo() {
        return ClusterAlgo;
    }
    void ProcessReceivedMsg(std::string);
    LOGGING_UTILITY *LogsObj;
    COMMUNICATION *CommObj;
    DATA_COLLECTION *DCObj;
    STATISTICS *StatsObj;
    AP_IP_INFO *APIPInfo;
    bool NoMoreCars;

};

/* Base class of Clustering service that can be overriden to use different clustering approach mechanisms*/

class BASECLUSTER {

private:

	/* Cluster computation interval */
    double ClusterInterval;

public:

    double GetClusterInterval() {
        return ClusterInterval;
    }

    void SetClusterInterval(double interval) {
        ClusterInterval = interval;
    }

	/* Contains information of current clusters that this AP is managing 
		format <MC_ID, CH_ID, List_Of_cluster_members> */
    std::map<std::string, std::map<std::string, std::list<std::string>>> ListOfClusters;

	/* Computes clusters */
    virtual void FormCluster(ACCESSPOINT *,std::map<std::string, struct CTABLEENTRIES*>) = 0;

	/* Distributes Clusters */
    virtual void DisseminateClusterInfo (ACCESSPOINT *BSObj, std::map<std::string, std::string>*) = 0;

	/* It breaks tie if cars belongs to multiple clusters, not tested thoroughly in multiple 
		clusters situation hence not included as part of thesis */
    //virtual void TieBreakBetweenMultipleMC(ACCESSPOINT *, std::map<std::string, std::string>*, std::map<std::string, struct CTABLEENTRIES*>) = 0;
    ~BASECLUSTER() {
    }

};



/* Class for Map based approach */
class MAPBASED_CLUSTER : public BASECLUSTER {

public:

    explicit MAPBASED_CLUSTER(ACCESSPOINT *);

    void FormCluster(ACCESSPOINT *, std::map<std::string, struct CTABLEENTRIES*>) override ;

    void DisseminateClusterInfo (ACCESSPOINT *BSObj, std::map<std::string, std::string>*) override;

    //void TieBreakBetweenMultipleMC(ACCESSPOINT *, std::map<std::string, std::string>*,std::map<std::string, struct CTABLEENTRIES*>) override;
    virtual ~MAPBASED_CLUSTER() {
    }

};

/* The cluster table (that stores information of vehicles which have sent their GPS and Speed info) entries */

struct CTABLEENTRIES {

    std::string VID;

    std::string Lane;

    double X;

    double Y;

    double Speed;

    double Dir;

    time_t Timestamp;

};

/* Class to maintain cluster table 
	Add update deletes from the table */

class CLUSTERTABLE {

private:

    std::map<std::string, struct CTABLEENTRIES*> ClusterTable;

    clock_t CTEntryTimer;


public:

    CLUSTERTABLE(LOGGING_UTILITY *);

    clock_t GetCTEntryTimer() {
        return CTEntryTimer;
    }

    std::map<std::string, struct CTABLEENTRIES*> GetClusterTable() {
        return ClusterTable;
    }

    void AddToClusterTable(UPDATE_LOCALINFO*, ACCESSPOINT*);

    void DeleteFromClusterTable(std::string, ACCESSPOINT *);

    void ValidateCTEntries(ACCESSPOINT *, useconds_t);

    ~CLUSTERTABLE(){
        for (auto it = ClusterTable.begin(); it != ClusterTable.end(); it++) {
            delete it->second;
            ClusterTable.erase(it->first);
        }
    }
};


#endif //VMCPI_AP_H
