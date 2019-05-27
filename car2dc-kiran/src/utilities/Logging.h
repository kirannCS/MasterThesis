//
// Created by kiran.narayanaswamy on 8/17/18.
//

#ifndef VMCPI_LOGGING_H
#define VMCPI_LOGGING_H

#include <string>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <cmath>


class LOGGING_UTILITY {
private:
    std::string ModuleName;
    std::string SubModuleName;
    std::string ID;
    bool LogInfoEnable;
    bool LogInfoFile;
    bool LogInfoStdOutput;
    bool LogDebugEnable;
    bool LogDebugFile;
    bool LogDebugStdOutput;
    bool LogStatsEnable;
    bool LogStatsFILE;
    bool LogStatsStdOutput;
    bool LogErrorEnable;
    bool LogErrorFILE;
    bool LogErrorStdOutput;
    std::string LogFilePath;
    std::string ExperimentNumber;
    std::fstream Log;
public:
    LOGGING_UTILITY(std::string, std::string, std::string = "");
    void LogInfo(std::string);
    void LogDebug(std::string);
    void LogStats(std::string);
    void LogError(std::string);
    std::string GetCurrentTime();
    std::string GetMsgString(std::string);
};

#endif //VMCPI_LOGGING_H
