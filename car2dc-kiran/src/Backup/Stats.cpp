//
// Created by kiran on 11/5/18.
//

#include "Stats.h"
#include "../header/ap/AP.h"

void STATISTICS::PrintAPStats(ACCESSPOINT *APObj) {
    APObj->LogsObj->LogStats("Consecutive intervals = ");
    std::string _Count = "Count{", _Intervals = "Intervals{";
    for(int i = 0; i < 25; i++) {
        _Intervals += std::to_string(i) + ",";
        _Count += std::to_string(APObj->StatsObj->CountOfConsecutiveInterval[i]) + ",";
    }
    _Count += "}Count";
    _Intervals += "}Intervals";
    APObj->LogsObj->LogStats(_Intervals);
    APObj->LogsObj->LogStats(_Count);

    for (int i = 0; i < APObj->StatsObj->ClusterSequenceNumber; i++) {
        APObj->LogsObj->LogStats("At step " + std::to_string(i) + " " +
                                         std::to_string(APObj->StatsObj->NumberOfVehInClusters[i])
                                 + " car(s) are part of the cluster");
    }

    APObj->LogsObj->LogStats("Total Data Received = " + std::to_string(APObj->StatsObj->TotalDataReceived));

    float TotatlTime = APObj->StatsObj->AvgClusterFormationTime / APObj->StatsObj->ClusterSequenceNumber;
    APObj->LogsObj->LogStats("Average Cluster Formation time = " + std::to_string(TotatlTime));
    APObj->LogsObj->LogStats("Total Times cluster formed = " + std::to_string(APObj->StatsObj->ClusterSequenceNumber));
}


void STATISTICS::PrintVehStats(LOGGING_UTILITY *LogsObj) {
    LogsObj->LogStats("Total Data I generated = " + std::to_string(this->TotalDataCHGenerated));
    LogsObj->LogStats("Total Data Sent to CH = " + std::to_string(this->TotalDataSentToCH));
    LogsObj->LogStats("Total Data Received from CM = " + std::to_string(this->TotalDataReceivedFromCM));
    LogsObj->LogStats("Total Data Sent to AP = " + std::to_string(this->TotalDataSentToAP));

}