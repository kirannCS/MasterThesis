//
// Created by kiran on 11/5/18.
//

#include "Stats.h"
#include "../header/ap/AP.h"
#include "../header/ap/DC.h"

using namespace std;

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

    ofstream outputFile;
    std::string filename = "../results/logs/ConsecutiveCH.csv";
    outputFile.open(filename);
    outputFile << "Interval" << "," << "Count" << std::endl;
    for(int i = 0; i < 15; i++) {
	    outputFile << i << "," << APObj->StatsObj->CountOfConsecutiveInterval[i] << std::endl;
    }
    outputFile.close();
    outputFile.open("../results/logs/NoOfVehiclesInCluster.csv");
    outputFile << "StepCount" << "," << "Number Of Cars" << std::endl;
    for (int i = 1; i <= APObj->StatsObj->ClusterSequenceNumber; i++) {
        APObj->LogsObj->LogStats("At step " + std::to_string(i) + " " +
                                         std::to_string(APObj->StatsObj->NumberOfVehInClusters[i])
                                 + " car(s) are part of the cluster");
		outputFile << i << "," << APObj->StatsObj->NumberOfVehInClusters[i] << std::endl;
    }
    outputFile.close();


    for( auto it = APObj->StatsObj->NumberOfVehInMultipleClusters.begin(); it != APObj->StatsObj->NumberOfVehInMultipleClusters.end(); it++) {
        std::string Filename = "../results/logs/NoOfVehiclesInCluster_" + it->first + ".csv";
        outputFile.open(Filename.c_str());
        outputFile << "StepCount" << "," << "Number Of Cars" << std::endl;
        for (int i = 1; i <= APObj->StatsObj->ClusterSequenceNumber; i++) {
            APObj->LogsObj->LogStats("At step " + std::to_string(i) + " " +
                                     std::to_string(APObj->StatsObj->NumberOfVehInMultipleClusters[it->first][i])
                                     + " car(s) are part of the cluster");
            outputFile << i << "," << APObj->StatsObj->NumberOfVehInMultipleClusters[it->first][i] << std::endl;
        }
        outputFile.close();
    }

    std::string Filename = "../results/logs/DataInMultipleCluster.csv";
    outputFile.open(Filename.c_str());
    outputFile << "MC_ID" << "," << "Data" << std::endl;
    for( auto it = APObj->StatsObj->DataCollectedAtMC.begin(); it != APObj->StatsObj->DataCollectedAtMC.end(); it++) {
            outputFile << it->first << "," << APObj->StatsObj->DataCollectedAtMC[it->first] << std::endl;
    }
    outputFile.close();

    //APObj->LogsObj->LogStats("Total Data Received = " + std::to_string(APObj->StatsObj->TotalDataReceived));

   /* float TotalTime = (APObj->StatsObj->TotalClusterFormationTime / (APObj->StatsObj->ClusterSequenceNumber - 9)) * 1000;
    APObj->LogsObj->LogStats("Average Cluster Formation time = " + std::to_string(TotalTime));
    APObj->LogsObj->LogStats("Total Times cluster formed = " + std::to_string(APObj->StatsObj->ClusterSequenceNumber));*/


    /*filename = "../results/logs/AvgClusterFormationTime.csv";
    outputFile.open(filename);
    outputFile << "Avg Time" << "," <<  TotalTime << std::endl;
    outputFile.close();*/
}

void STATISTICS::PrintDCStats(AP_DATA_COLLECTION *DCObj) {

    std::string Filename = "../results/logs/DataInMultipleCluster_" + DCObj->GetID() + ".csv";
    ofstream outputFile;
    outputFile.open(Filename.c_str());
    outputFile << "MC_ID" << "," << "Data" << std::endl;
    for( auto it = DCObj->StatsObj->DataCollectedAtMC.begin(); it != DCObj->StatsObj->DataCollectedAtMC.end(); it++) {
        outputFile << it->first << "," << it->second << std::endl;
        std::cout << "hereeeee " << it->first << " " << it->second << std::endl;
    }
    outputFile.close();
}

void STATISTICS::PrintVehStats(LOGGING_UTILITY *LogsObj) {
    LogsObj->LogStats("Total Data I generated = " + std::to_string(this->TotalDataCHGenerated));
    LogsObj->LogStats("Total Data Sent to CH = " + std::to_string(this->TotalDataSentToCH));
    LogsObj->LogStats("Total Data Received from CM = " + std::to_string(this->TotalDataReceivedFromCM));
    LogsObj->LogStats("Total Data Sent to AP = " + std::to_string(this->TotalDataSentToAP));

}
