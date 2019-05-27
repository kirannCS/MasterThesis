//
// Created by kiran.narayanaswamy on 8/17/18.
//

#ifndef VMCPI_RSU_H
#define VMCPI_RSU_H

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <unistd.h>
#include "../../utilities/Logging.h"
#include "../../utilities/Communication.h"

class ROADSIDEUNIT {
private:
    std::string ID;
    std::string IP;
    std::string Port;
    std::string X;
    std::string Y;
    std::string ParserIP;
    std::string UDMPort;
    std::string ParserPort;

public:
    ROADSIDEUNIT(char *, LOGGING_UTILITY *);
    std::string GetIP(){
        return IP;
    }
    std::string GetPort() {
        return Port;
    }
    void SendBroadMsg(std::string);
    void SendUnicastMsg(std::string, std::string);
    void SendMsg(std::string);
    void ProcessReceivedMsg(char*);
    void RSUProc();
    void TestSendMsg();
    LOGGING_UTILITY *LogsObj;
};

void RSUServer(ROADSIDEUNIT *);

#endif //VMCPI_RSU_H
