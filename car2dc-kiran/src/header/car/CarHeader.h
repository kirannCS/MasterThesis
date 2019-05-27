//
// Created by kiran on 8/3/18.
//

#ifndef VMCPI_CARHEADER_H
#define VMCPI_CARHEADER_H

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <list>

#include "../../utilities/Communication.h"
#include "../../utilities/Logging.h"
#include "../../header/common/DATA.h"

#include "../../packets/header/CARSpeedRequestFromInCar.pb.h"
#include "../../packets/header/CARSpeedResponseToInCar.pb.h"
#include "../../packets/header/CARGPSRequestFromInCar.pb.h"
#include "../../packets/header/CARGPSResponseToInCar.pb.h"
#include "../../packets/header/CARRequestToMT.pb.h"
#include "../../packets/header/MTGPSResponse.pb.h"
#include "../../packets/header/MTSpeedResponse.pb.h"
#include "../../packets/header/MessageForwardFromUDM.pb.h"
#include "../../packets/header/BigData.pb.h"
#include "../../packets/header/MsgFromNodeToUDM.pb.h"



class CAR_CLUSTER;
class NEIGHBORTABLE;
class LOCAL_INFO;

/**
	Vehicle class that holds local information of the car
	Also has pointers to other required objects
*/

class VEHICLE {
private:
    std::string ID;
    double PosX;
    double PosY;
    double Direction;
    double Speed;
    /* To distinguish how messages are sent (through UDM or any other mechanism).
	A configurable parameter in XML but not used in the code(thesis) */
    std::string MessageSendService;

public:
    VEHICLE() {
        CarExists = true;
    }
    VEHICLE(LOGGING_UTILITY *, NEIGHBORTABLE*, CAR_CLUSTER *);

    void SetGPSInfo (double X, double Y, double Dir) {
        PosX = X;
        PosY = Y;
        Direction = Dir;
    }

    void SetSpeedInfo (double _Speed) {
        Speed = _Speed;
    }

    void GetGPSInfo (double *X, double *Y, double *Dir) {
        *X = PosX;
        *Y = PosY;
        *Dir = Direction;
    }

    void GetSpeedInfo (double *_Speed) {
        *_Speed = Speed;
    }

    void SetVehicleID(std::string _ID) {
        ID =_ID;
    }
    std::string GetVehicleID() {
        return ID;
    }
    void ProcessReceivedMsg(std::string);
    LOGGING_UTILITY *LogsObj;
    BASE_COMMUNICATION *CommObj;
    CAR_CLUSTER *ClusterObj;
    NEIGHBORTABLE *NTObj;
    LOCAL_INFO *LocalInfoObj;
    bool CarExists;
};



/**
	Holds information regarding to IP and port address of all the in-car modules

*/

class LOCAL_INFO {
private:
    std::string CarServerIP;
    std::string CarClusterPort;
    std::string CarSpeedPort;
    std::string CarPosPort;
public:
    void AssignPorts(char *, LOGGING_UTILITY *);
    std::string GetCarSpeedPort() {
        return CarSpeedPort;
    }
    std::string GetCarPosPort() {
        return CarPosPort;
    }
    void SetCarServerIP(std::string IP) {
        CarServerIP = IP;
    }
    std::string GetCarServerIP() {
        return CarServerIP;
    }
    std::string GetCarClusterPort() {
        return CarClusterPort;
    }
};


#endif //VMCPI_CARHEADER_H
