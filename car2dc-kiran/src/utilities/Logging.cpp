//
// Created by kiran.narayanaswamy on 8/17/18.
//

#include "Logging.h"
#include "../../lib/pugixml/pugixml.hpp"


LOGGING_UTILITY::LOGGING_UTILITY(std::string _ID, std::string Module, std::string SubModule) {
    pugi::xml_document doc;
    if (!doc.load_file("../config/logging_utility/config.xml")) {
        fprintf(stderr,"ERROR:: LOGGING_UTILITY Config File Missing !!\n") ;
    }

    pugi::xml_node tools = doc.child("VMCPI").child("config");
    std::string name, value;

    for (pugi::xml_node tool = tools.first_child(); tool; tool = tool.next_sibling())
    {
        //std::cout << "Tool:";
        for (pugi::xml_attribute attr = tool.first_attribute(); attr; attr = attr.next_attribute())
        {
            name = attr.name();
            value = attr.value();
            //std::cout << "name = " << name << " value = " << value << std::endl;
            if (name == "LogInfoEnable")
                (value == "TRUE") ? LogInfoEnable = true: LogInfoEnable = false;
            else if (name == "LogInfoFile")
                (value == "TRUE") ? LogInfoFile = true: LogInfoFile = false;
            else if (name == "LogInfoStdOutput")
                (value == "TRUE") ? LogInfoStdOutput = true: LogInfoStdOutput = false;
            else if (name == "LogDebugEnable")
                (value == "TRUE") ? LogDebugEnable = true: LogDebugEnable = false;
            else if (name == "LogDebugFile")
                (value == "TRUE") ? LogDebugFile = true: LogDebugFile = false;
            else if (name == "LogDebugStdOutput")
                (value == "TRUE") ? LogDebugStdOutput = true: LogDebugStdOutput = false;
            else if (name == "LogStatsEnable")
                (value == "TRUE") ? LogStatsEnable = true: LogStatsEnable = false;
            else if (name == "LogStatsFILE")
                (value == "TRUE") ? LogStatsFILE = true: LogStatsFILE = false;
            else if (name == "LogStatsStdOutput")
                (value == "TRUE") ? LogStatsStdOutput = true: LogStatsStdOutput = false;
            else if (name == "LogErrorEnable")
                (value == "TRUE") ? LogErrorEnable = true: LogErrorEnable = false;
            else if (name == "LogErrorFILE")
                (value == "TRUE") ? LogErrorFILE = true: LogErrorFILE = false;
            else if (name == "LogErrorStdOutput")
                (value == "TRUE") ? LogErrorStdOutput = true: LogErrorStdOutput = false;
            else if (name == "LogFilePath")
                LogFilePath = value;
            else if(name == "ExperimentNumber")
                ExperimentNumber = value;
        }

        std::cout << std::endl;
    }
    ModuleName = Module;
    SubModuleName = SubModule;
    ID = _ID;
    LogFilePath = LogFilePath + "[" + ExperimentNumber + "][" + ModuleName + "][" + ID + "].txt";
    Log.open(LogFilePath, std::fstream::in | std::fstream::out | std::fstream::app);
    /*std::ofstream file(LogFilePath, std::ios_base::app | std::ios_base::out);
    log = file;*/
    //std::cout << LogInfoEnable << std::endl << LogInfoFile << std::endl << LogInfoStdOutput << std::endl << LogDebugEnable << std::endl\
        << LogDebugFile << std::endl << LogDebugStdOutput << std::endl << LogStatsEnable << std::endl << LogStatsFILE << std::endl\
        << LogStatsStdOutput << std::endl << LogErrorEnable << std::endl << LogErrorFILE << std::endl << LogErrorStdOutput << std::endl;

}

std::string LOGGING_UTILITY::GetMsgString(std::string Message) {
    if(SubModuleName == "")
        return "[" + GetCurrentTime() + "][" + ModuleName + "][" + ID + "] [" + Message + "]" + "\n";
    else
        return "[" + GetCurrentTime() + "][" + ModuleName + "][" + ID + "][" + SubModuleName +"] [" + Message + "]" + "\n";
}

void LOGGING_UTILITY::LogInfo(std::string msg) {
    const char* Message = msg.c_str();
    if (LogInfoEnable) {
        if (LogInfoFile) {
            Log << GetMsgString(Message) << std::endl;
        }
        if (LogInfoStdOutput) {
            std::cout <<  GetMsgString(Message) << std::endl;
        }
    }
    //delete Message;
}

void LOGGING_UTILITY::LogDebug(std::string msg) {
    const char* Message = msg.c_str();
    if (LogDebugEnable) {
        if (LogDebugFile) {
            Log << GetMsgString(Message) << std::endl;
        }
        if (LogDebugStdOutput) {
            std::cout <<  GetMsgString(Message) << std::endl;
        }

    }
    //delete Message;
}

void LOGGING_UTILITY::LogStats(std::string msg) {
    const char* Message = msg.c_str();
    if (LogStatsEnable) {
        if (LogStatsFILE) {
            Log << GetMsgString(Message) << std::endl;
        }
        if (LogStatsStdOutput) {
            std::cout <<  GetMsgString(Message) << std::endl;

        }
    }
    //delete Message;
}

void LOGGING_UTILITY::LogError(std::string msg) {
    const char* Message = msg.c_str();
    if (LogErrorEnable) {
        if (LogErrorFILE) {
            Log << GetMsgString(Message) << std::endl;
        }
        if (LogErrorStdOutput) {
            std::cout <<  GetMsgString(Message) << std::endl;
        }
    }
    //delete Message;
}

std::string LOGGING_UTILITY::GetCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long int millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
    if (millisec>=1000) { // Allow for rounding up to nearest second
        millisec -=1000;
        tv.tv_sec++;
    }

    tm *ltm = localtime(&tv.tv_sec);
    std::string time = std::to_string(ltm->tm_hour) + ":" + std::to_string(ltm->tm_min) + ":" + std::to_string(ltm->tm_sec) + "." + std::to_string(millisec);
    return time;
}