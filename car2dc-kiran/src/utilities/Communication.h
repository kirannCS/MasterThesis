//
// Created by kiran.narayanaswamy on 8/15/18.
//

#ifndef VMCPI_COMMUNICATION_H
#define VMCPI_COMMUNICATION_H

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <cstdarg>


#include "../utilities/Logging.h"
#include "../packets/header/MsgFromNodeToUDM.pb.h"
#include "../packets/header/BigData.pb.h"

#include "../header/common/LocalInfo.h"
#include "../header/common/ClusterInfo.h"
#include "../header/common/Hello.h"
#include "../header/common/RSUBeacon.h"
#include "../header/common/DATA.h"
#include "../header/common/Task.h"

#include <cassert>

/**
	Communication class is used to receive and send messages 
	It is a base class which can be overridden
*/
class BASE_COMMUNICATION {
public:
    virtual void SendUnicastMsg(std::string, std::string, void *, char*...) = 0;
    virtual void SendBroadcastMsg(std::string, void *, char*...) = 0;
    virtual void ReceiveMessage(void *, std::string, char*...) = 0;
};


/**
	Communication class used to send and receive messages in car and AP
*/

class COMMUNICATION: public BASE_COMMUNICATION{
private:
    std::string CarServerIP;
    std::string ParserPort;
    std::string ParserIP;
    std::string UDMPort;

public:
    COMMUNICATION(std::string Default, LOGGING_UTILITY *_LogsObj) {
        LogsObj = _LogsObj;
    }
    explicit COMMUNICATION(LOGGING_UTILITY *);
    LOGGING_UTILITY *LogsObj;
    std::string GetParserIP() {
        return ParserIP;
    }
    std::string GetParserPort() {
        return ParserPort;
    }
    std::string GetUDMPort() {
        return UDMPort;
    }
    std::string GetCarServerIP() {
        return CarServerIP;
    }
    void SendUnicastMsg(std::string, std::string, void *, char *...);
    void SendBroadcastMsg(std::string, void *, char *...);
    void ReceiveMessage(void *, std::string, char*...);
    void SendInit(std::string);
    void KillReceiverSocket();
    virtual ~COMMUNICATION() {
	}
};

/** 
	Worker routine struct used to pass it as a argument to worker routine method
*/

struct Worker_Routine {
    zmq::context_t *WR_Context;
    std::string ReceiverType;
};

#endif //VMCPI_COMMUNICATION_H
