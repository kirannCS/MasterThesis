//
// Created by kiran on 8/1/18.
//


/********************************************************************
        Reads vehicles Speed information from In-car sensors
        Provide updaated Speed information to requesting in-car Modules

*********************************************************************/


#include "../../header/car/CarHeader.h"
#include "../../../lib/pugixml/pugixml.hpp"
#include "../../../lib/pugixml/pugixml.cpp"


class SPEEDMODULE {
private:
    double Speed;
    std::string ParserPort;
    std::string ParserIP;
    std::string UDMPort;
    std::string CarServerIP;
public:
    std::string GetParserIP() {
	return ParserIP;
    }

    std::string GetUDMPort() {
	return UDMPort;
    }
    SPEEDMODULE(LOGGING_UTILITY *);
    LOCAL_INFO *LocalInfoObj;
    void SetSpeed(double S) {
        Speed = S;
    }
    void GetSpeed(double *S) {
        *S = Speed;
    }
    std::string GetCarServerIP() {
        return CarServerIP;
    }
    void SetCarsServerIP(std::string IP) {
        CarServerIP = IP;
    }
    void ReadSpeedInfoFromSensor(char *[], LOGGING_UTILITY *);
    void SpeedInfoServer(LOGGING_UTILITY*, SPEEDMODULE*);
    bool CarExists;
};


/** 
        Constructor that reads XML Config file to populate required variables 
*/

SPEEDMODULE::SPEEDMODULE(LOGGING_UTILITY *LogsObj) {
    Speed = -1;
    CarExists = true;
    pugi::xml_document doc;
    pugi::xml_node tools = doc.child("VMCPI").child("config");
    if (!doc.load_file("../config/common/config.xml")) {
        std::cout << "ERROR:: Config File Missing ../config/common/config.xml !!\n";
    }
    else {
        tools = doc.child("VMCPI").child("config");

        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                if (name == "ParserPort")
                    ParserPort = ait->value();
                else if (name == "ParserIP")
                    ParserIP = ait->value();
                else if (name == "UDMPort")
                    UDMPort = ait->value();
            }
        }
    }

    if (!doc.load_file("../config/car/config.xml")) {
        LogsObj->LogError("ERROR:: Config File Missing ../config/car/config.xml !!\n") ;
    }

    else {
        tools = doc.child("VMCPI").child("config");
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                if (name == "CarServerIP")
                    CarServerIP = ait->value();
            }
        }
    }
    LogsObj->LogDebug("Mobility Trace server IP = " + ParserIP + " Port = " + ParserPort);
}


void SpeedModuleServer(LOGGING_UTILITY *LogsObj, SPEEDMODULE *DummySpeedModule) {
    DummySpeedModule->SpeedInfoServer(LogsObj, DummySpeedModule);
}

/*!
 * 'SpeedInfoServer' awaits requests from in-car modules for the Speed information
 */

void SPEEDMODULE::SpeedInfoServer(LOGGING_UTILITY *LogsObj, SPEEDMODULE *DummySpeedModule) {
    std::string ResponseBuffer;
    LogsObj->LogDebug("Creating 'ZMQ_REP' socket in 'Speed Module Server' in-car module");

    zmq::context_t Context(1);
    zmq::socket_t Socket(Context, ZMQ_REP);
    std::string IP = CarServerIP;
    std::string Port = LocalInfoObj->GetCarSpeedPort();
    Socket.bind("tcp://" + IP + ":" + Port);

    CARSPEEDRESPONSETOINCAR ResponseStr;
    std::string ResponseMsg;
    zmq::message_t Request;

    while (CarExists) {
        Socket.recv(&Request);
        LogsObj->LogDebug("Received new request for Speed data from an in-car module");
        if(CarExists) {
            DummySpeedModule->GetSpeed(&Speed);
            ResponseStr.set_carexists(true);
            ResponseStr.set_speed(Speed);
            LogsObj->LogDebug("Sending response to in-car module with updated Speed data, Speed = " + std::to_string(Speed));
        } else {
            ResponseStr.set_carexists(false);
            ResponseStr.set_speed(-1.0);
            LogsObj->LogDebug("Sending response to in-car module that car do not exist anymore in the network");
        }

        ResponseStr.SerializeToString(&ResponseMsg);
        zmq::message_t request (ResponseMsg.size());
        memcpy ((void *) request.data (), ResponseMsg.c_str(), ResponseMsg.size());
        Socket.send (request);

        ResponseMsg.clear();
    }
    Socket.close();
}

/** 
        Thread that receives Car exit status 
*/

void T_CarExitStatus(SPEEDMODULE *DummySpeedModule, std::string VehID) {
    zmq::message_t MessageReceived;
    zmq::message_t Address;
    zmq::context_t *Context;
    zmq::socket_t *Subscriber;

    if(true) {
        Context = new zmq::context_t(1);
        Subscriber = new zmq::socket_t(*Context, ZMQ_SUB);
        Subscriber->connect("tcp://" + DummySpeedModule->GetParserIP() + ":" + (std::to_string(std::stoi(DummySpeedModule->GetUDMPort()) + 3)));
        std::string ID = VehID + "EXITSPEEDID";
        Subscriber->setsockopt(ZMQ_SUBSCRIBE, ID.c_str(), std::strlen(ID.c_str()));
        Subscriber->recv(&Address);
        Subscriber->recv(&MessageReceived);

        DummySpeedModule->CarExists = false;
        sleep(5);
        Subscriber->close();
        Context->close();
        delete Subscriber;
	delete Context;
            exit(0);
    }
}


/*!
 * 'main()' function of the base station entity
 * @param argc 2
 * @param argv argv[0] has vehcile ID and argv[1] has the starting Port number(used to assign port numbers
 * to different servers in the in-car modules
 * @return 0 on success, -1 on error
 */
int main(int argc, char *argv[]) {
    LOGGING_UTILITY LogsObj(std::string(argv[1]), "CAR", "  SPEED");

    LogsObj.LogDebug("Inside 'Car Speed' Module ");
    LogsObj.LogInfo("Inside 'Car Speed' Module ");

    LOCAL_INFO LocalInfoObj;
	/* Assigns ports */
    LocalInfoObj.AssignPorts(argv[2], &LogsObj);

    SPEEDMODULE DummySpeedModule(&LogsObj);
    DummySpeedModule.SetCarsServerIP(argv[3]);
    LocalInfoObj.SetCarServerIP(DummySpeedModule.GetCarServerIP());
    DummySpeedModule.LocalInfoObj = &LocalInfoObj;


    /*! starts a thread for speed module server*/
    LogsObj.LogDebug("Starting 'Speed Module Server' thread");
    LogsObj.LogInfo("Starting 'Speed Module Server' thread");
    std::thread SpeedService(SpeedModuleServer, &LogsObj, &DummySpeedModule);

    std::string VehicleID = argv[1];
    std::thread ExitStatus(T_CarExitStatus, &DummySpeedModule, VehicleID);

    DummySpeedModule.ReadSpeedInfoFromSensor(argv, &LogsObj);
    SpeedService.join();
    return 0;
}

/** 
        Function to fetch updated Speed info from in-car sensors
*/


void SPEEDMODULE::ReadSpeedInfoFromSensor(char *argv[], LOGGING_UTILITY *LogsObj) {
    /*! Creates and Connects a socket with in-car sensors to fetch the speed information */
    LogsObj->LogDebug("Creating 'ZMQ_REQ' socket that connects to SUMO Data Parser");
    zmq::context_t Context(1);
    zmq::socket_t Socket(Context, ZMQ_REQ);
    std::string IP = ParserIP;
    std::string Port = ParserPort;
    Socket.connect("tcp://" + IP + ":" + Port);

    CARREQUESTTOMT RequestStr;
    RequestStr.set_vid(std::string(argv[1]));
    RequestStr.set_req("SPE");
    std::string RequestBuffer;

    MTSPEEDRESPONSE ResponseStr;

    /*! Periodically(every second) send a request for the speed details */
    while (CarExists) {
        RequestStr.SerializeToString(&RequestBuffer);
        zmq::message_t request(RequestBuffer.size()), Reply;
        memcpy ((void*)request.data(), RequestBuffer.c_str(), RequestBuffer.size());
        LogsObj->LogDebug("Sending message to SUMO Data Parser for updated information of Speed");
        Socket.send(request);

        Socket.recv(&Reply);

        std::string ResponseBuffer(static_cast<char*>(Reply.data()), Reply.size());
        ResponseStr.ParseFromString(ResponseBuffer);
        LogsObj->LogDebug("Received message from SUMO Data Parser contains updated information of Speed");

        CarExists = ResponseStr.info_exists();
        if(CarExists) {
            SetSpeed(ResponseStr.speed());
            LogsObj->LogDebug("Updated Speed = " + std::to_string(ResponseStr.speed()));
        } else {
            LogsObj->LogDebug("Car Existed from the network");
        }

        /*! One second break */
        sleep(1);
    }
    Socket.close();
    LogsObj->LogDebug("Halting Car Speed Module as car do not exist in the network anymore");
}
