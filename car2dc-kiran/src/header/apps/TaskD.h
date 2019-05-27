//
// Created by kiran.narayanaswamy on 1/3/19.
//

#ifndef VMCPI_TASKD_H
#define VMCPI_TASKD_H

#include "../../../lib/chilkat/include/CkFileAccess.h"

#include "../../utilities/Stats.h"
#include "../../utilities/Communication.h"
#include "../../utilities/Logging.h"
#include "../../../lib/pugixml/pugixml.hpp"
#include "../../packets/header/CARClusterInfoReq.pb.h"
#include "../../packets/header/CARClusterInfoResponse.pb.h"
#include "../../packets/header/CountData.pb.h"
#include "../../header/common/Task.h"
#include "../../header/common/Visuals.h"
#include "../car/CarHeader.h"
#include <list>

class TASK_CLUSTERINFO;

class TASK_DISTRIBUTION {
    std::string VID;
	/* Count of number of tasks so far assigned to Cluster Members */
    unsigned int TaskSeqNum;
	/* Maximum time within which Cluster Members should compute task is chosen as a random number 
		between TaskMaxTime and TaskMinTime */
    __useconds_t TaskMaxTime;
    __useconds_t TaskMinTime;
	/* Interval with which subtasks are assigned to Cluster Members */
    __useconds_t TaskInterval;

public:
    std::list<std::string> PrevCMList;
	/* <VID, <0|1>> mapping 1 -> Subtask results from that VID is received 
		0 -> Subtasks results not received */
    std::map<std::string, bool> ResultsCollectionChart;
    std::string GetVehicleID() {
        return VID;
    }
    TASK_DISTRIBUTION(std::string, LOGGING_UTILITY*, LOCAL_INFO*, TASK_CLUSTERINFO*, COMMUNICATION*, STATISTICS*);
    TASK_CLUSTERINFO *ClusterObj;
    bool CarExists;
    STATISTICS *StatsObj;
    LOGGING_UTILITY *LogsObj;
    COMMUNICATION *CommObj;
    LOCAL_INFO *LocalInfoObj;
    struct Visuals_IP_Info *VisualsIPInfo;
    void ProcessReceivedMsg(std::string);
    void CollectClusterInfo();
    void TaskDistributionService();
    void DistributeTasks();
    __useconds_t CreateTasksAndSendToCM();
    void CollectResults();
    void CompleteTaskAndSendResults(ALLDATA *);
    void ProcessTaskResults(ALLDATA *);
    void MessageToVisuals();
};

/**
        Class used to maintain cluster information received from Cluster service module
*/

class TASK_CLUSTERINFO {
public:
    bool AM_I_CH;
    std::string ClusterState;
    std::string CH;
    std::list<std::string> CMList;
    std::string ClusterManager;
    std::string MC_ID;
    std::string DefaultClusterManager;
};

#endif 
