//
// Created by kiran on 10/27/18.
//

#ifndef VMCPI_DCA_H
#define VMCPI_DCA_H

#include <string>
#include <stdlib.h>
#include <fstream>
#include <dirent.h>
#include <chrono>

#include "../../../lib/chilkat/include/CkFileAccess.h"

#include "../../utilities/Stats.h"
#include "../../utilities/Communication.h"
#include "../../utilities/Logging.h"
#include "../../../lib/pugixml/pugixml.hpp"
#include "../../packets/header/CARClusterInfoReq.pb.h"
#include "../../packets/header/CARClusterInfoResponse.pb.h"
#include "../../packets/header/CountData.pb.h"
#include "../../header/common/DATA.h"
#include "../../header/common/Visuals.h"
#include "../car/CarHeader.h"
#include <fstream>
#include <sstream>
#include <list>

class DCA_CLUSTERINFO;

class DCA_BASE_DATACOLLECTION {
private:
    double CMDataUpdateInterval;

public:
    void SetCMDataUpdateInterval(double _Interval) {
        CMDataUpdateInterval = _Interval;
    }
    double GetCMDataUpdateInterval() {
        return CMDataUpdateInterval;
    }
    virtual void DataCollectionService()  = 0;
    virtual void CollectClusterInfo() = 0;
    virtual std::string CollectDataFromApp() = 0;
    virtual void SendDataToCH() = 0;
    virtual void SendDataToAP() = 0;
    virtual void CollectDataFromCM() = 0;
    virtual void ProcessReceivedMsg(std::string) = 0;
    virtual std::pair<std::string, long unsigned int> AggregateData() = 0;

};

class DCA_DATACOLLECTION : public DCA_BASE_DATACOLLECTION {
private:
    int DataSize;
    int ChunkSize;
    std::string VID;
	/* Path where Cluster Members generated data files are saved */
    std::string DataFilePath;
	/* Path where Cluster Head save received data */
    std::string CHReceivedDataFilePath;
	/* Path where CH generated data files are saved */
    std::string CHGeneratedDataFilePath;
	/* Counts packets(fragments) sent to Cluster Head */
    int CMSequenceNum;
	/* Counts packets(fragments) sent to Accesspoint */
    int CHSequenceNum;
    bool IsAggregationReq;
    double AggregationPercentage;
    int APDataUpdateInterval;


public:
    std::string GetVehicleID() {
	    return VID;
    }
    DCA_DATACOLLECTION() {}
    bool CarExists;
    LOGGING_UTILITY *LogsObj;
    LOCAL_INFO *LocalInfoObj;
    DCA_CLUSTERINFO *DCAClusterObj;
    COMMUNICATION *CommObj;
    DCA_DATACOLLECTION(std::string, LOGGING_UTILITY*, LOCAL_INFO*, DCA_CLUSTERINFO*, COMMUNICATION*, STATISTICS*);
    std::string GenerateData(bool);
    void SplitDataFile(std::string, std::string);
    void DataCollectionService() override ;
    void CollectClusterInfo() override ;
    std::string CollectDataFromApp() override ;
    void SendDataToCH() override ;
    void SendDataToAP() override ;
    void CollectDataFromCM() override ;
    void TransferDataTOCH(std::string);
    void TransferDataToAP(std::string, long unsigned int);
    void ProcessReceivedMsg(std::string) override ;
    void ProcessReceivedData(ALLDATA*);
    void MessageToVisuals();
    std::pair<std::string, long unsigned int> AggregateData() override ;
    STATISTICS *StatsObj;
    struct Visuals_IP_Info *VisualsIPInfo;

};

/**
	Class used to maintain cluster information received from Cluster service module
*/

class DCA_CLUSTERINFO {
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
