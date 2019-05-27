//
// Created by kiran on 11/5/18.
//

#ifndef VMCPI_STATS_H
#define VMCPI_STATS_H

#include <string>
#include <map>
#include <list>
#include "Logging.h"
#include <fstream>
#include <iostream>


class ACCESSPOINT;
class VEHICLE;
class AP_DATA_COLLECTION;

class STATISTICS {
private:
    bool PublishClusterInfo;

public:
    unsigned long long ClusterSequenceNumber;
    unsigned long long TotalDataReceived;
    unsigned long long TotalDataReceivedFromCM;
    unsigned long long TotalDataSentToAP;
    unsigned long long TotalDataReceivedCHInterval;
    unsigned long long TotalDataSentToCH;
    unsigned long long TotalDataCHGenerated;
    unsigned long long TotalTasksAssigned;
    unsigned long long TotalSubTasksAssigned;
    unsigned long long TotalSubTasksResultsReceived;
    unsigned long long TotalSubTasksCompleted;


    float TotalClusterFormationTime;
    STATISTICS() {
        PublishClusterInfo = false;
        ClusterSequenceNumber = 0;
        TotalDataReceived = 0;
        TotalClusterFormationTime = 0.0;
        TotalDataReceivedFromCM = 0;
        TotalDataSentToAP = 0;
        TotalDataReceivedCHInterval = 0;
        TotalDataSentToCH = 0;
        TotalDataCHGenerated = 0;
        TotalTasksAssigned = 0;
        TotalSubTasksAssigned = 0;
        TotalSubTasksResultsReceived = 0;
        TotalSubTasksCompleted = 0;
        DataCollectedAtMCAtEachInterval = 0;

    }
    std::map<std::string, std::map<int,int>> NumberOfVehInMultipleClusters;
    std::map<int,int> NumberOfVehInClusters;
    std::map<std::string, unsigned long long> DataCollectedAtMC;
    std::map<unsigned int, unsigned long long> TotalDataCollectedAtMCAtEachInterval;
    unsigned long long DataCollectedAtMCAtEachInterval;
    std::map<std::string, std::map<std::string, std::list<std::string>>> ListOfClusters;
    std::map<std::string, std::string> VehWhichMC;
    std::map<std::string, int> CurrentCHWithCount;
    std::map<std::string, std::string> PrevCH;
    int CountOfConsecutiveInterval[25] = {0};

    void SetPublishClusterInfo(bool Value) {
        this->PublishClusterInfo = Value;
    }
    bool GetPublishClusterInfo() {
        return this->PublishClusterInfo;
    }
    void PrintAPStats(ACCESSPOINT *);
    void PrintDCStats(AP_DATA_COLLECTION*);
    void PrintVehStats(LOGGING_UTILITY *);
    void PublishClusterInfoToStats(std::map<std::string, std::map<std::string, std::list<std::string>>>
                                   _ListOfClusters, std::map<std::string, std::string>* _VehWhichMC) {
        this->ListOfClusters = _ListOfClusters;
        this->VehWhichMC = *_VehWhichMC;
        this->PublishClusterInfo = true;
    }

};

#endif //VMCPI_STATS_H
