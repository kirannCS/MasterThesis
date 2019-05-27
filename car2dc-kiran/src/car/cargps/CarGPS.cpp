//
// Created by kiran on 8/1/18.
//

/********************************************************************
	Reads vehicles GPS information from In-car sensors
	Provide updaated GPS information to requesting in-car Modules

*********************************************************************/


#include "../../header/car/CarHeader.h"
#include "../../../lib/pugixml/pugixml.hpp"
#include "../../../lib/pugixml/pugixml.cpp"


class CARGPSMODULE {
private:
    double PositionX;
    double PositionY;
    double Direction;
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

    CARGPSMODULE(LOGGING_UTILITY *);
    LOCAL_INFO *LocalInfoObj;
    bool CarExists;
    void SetGPSInfo(double X, double Y, double Dir) {
        PositionX = X;
        PositionY = Y;
        Direction = Dir;
    }
    void GetGPSInfo(double *X, double *Y, double *Dir) {
        *X = PositionX;
        *Y = PositionY;
        *Dir = Direction;
    }
    std::string GetCarServerIP() {
        return CarServerIP;
    }
    void SetCarsServerIP(std::string IP) {
        CarServerIP = IP;
    }
    void ReadGPSInfoFromSensor(char *[], LOGGING_UTILITY *);
    void GPSInfoServer(LOGGING_UTILITY *, CARGPSMODULE *);
};

/** 
	Constructor that reads XML Config file to populate required variables 
*/
CARGPSMODULE::CARGPSMODULE(LOGGING_UTILITY *LogsObj) {
    PositionX = -1;
    PositionY = -1;
    Direction = -1;
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

    LogsObj->LogDebug("Mobility Trace server IP = " + ParserIP + " Port = " + ParserPort);
}



void T_PositionModuleServer(LOGGING_UTILITY *LogsObj, CARGPSMODULE *DummyGPSModule) {
    DummyGPSModule->GPSInfoServer(LogsObj, DummyGPSModule);
}

/*!
 * 'GPSInfoServer' awaits requests from in-car modules for the GPS information
 */
void CARGPSMODULE::GPSInfoServer(LOGGING_UTILITY *LogsObj, CARGPSMODULE *DummyGPSModule) {
    std::string ResponseBuffer;
    LogsObj->LogDebug("Creating 'ZMQ_REP' socket in 'Position Module Server' in-car module");

    zmq::context_t Context(1);
    zmq::socket_t Socket(Context, ZMQ_REP);
    std::string IP = CarServerIP;
    std::string Port = LocalInfoObj->GetCarPosPort();
    Socket.bind("tcp://" + IP + ":" + Port);


    CARGPSRESPONSETOINCAR ResponseStr;
    std::string ResponseMsg;
    zmq::message_t Request;

    while (CarExists) {
        Socket.recv(&Request);
        LogsObj->LogDebug("Received new request for Position data from an in-car module");
        if (CarExists) {
            ResponseStr.set_carexists(true);
            ResponseStr.set_x(PositionX);
            ResponseStr.set_y(PositionY);
            ResponseStr.set_dir(Direction);
            LogsObj->LogDebug("Sending response to in-car module with updated Position data = (" + std::to_string(PositionX) + "," + std::to_string(PositionY) + ")");
        } else {
            ResponseStr.set_carexists(false);
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

void T_CarExitStatus(CARGPSMODULE *DummyGPSModule, std::string VehID) {
    zmq::message_t MessageReceived;
    zmq::message_t Address;
    zmq::context_t *Context;
    zmq::socket_t *Subscriber;

    if(true) {
        Context = new zmq::context_t(1);
        Subscriber = new zmq::socket_t(*Context, ZMQ_SUB);
        Subscriber->connect("tcp://" + DummyGPSModule->GetParserIP() + ":" + (std::to_string(std::stoi(DummyGPSModule->GetUDMPort()) + 3)));
        std::string ID = VehID + "EXITGPSID";
        Subscriber->setsockopt(ZMQ_SUBSCRIBE, ID.c_str(), std::strlen(ID.c_str()));
        Subscriber->recv(&Address);
        Subscriber->recv(&MessageReceived);
            
        DummyGPSModule->CarExists = false;
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
    LOGGING_UTILITY LogsObj(std::string(argv[1]), "CAR", "    GPS");
    LogsObj.LogDebug("Inside 'Car GPS' Module ");
    LogsObj.LogInfo("Inside 'Car GPS' Module ");

    LOCAL_INFO LocalInfoObj;

	/* Assigns ports */
    LocalInfoObj.AssignPorts(argv[2], &LogsObj);

    CARGPSMODULE DummyGPSModule(&LogsObj);
    DummyGPSModule.SetCarsServerIP(argv[3]);
    LocalInfoObj.SetCarServerIP(DummyGPSModule.GetCarServerIP());
    DummyGPSModule.LocalInfoObj = &LocalInfoObj;

    /*! starts a thread for position module server*/
    LogsObj.LogDebug("Starting 'Position Module Server' thread");
    LogsObj.LogInfo("Starting 'Position Module Server' thread");
    std::thread PositionService(T_PositionModuleServer, &LogsObj, &DummyGPSModule);


    std::string VehicleID = argv[1];
    std::thread ExitStatus(T_CarExitStatus, &DummyGPSModule, VehicleID);

    DummyGPSModule.ReadGPSInfoFromSensor(argv, &LogsObj);
    PositionService.join();
    ExitStatus.join();
    return 0;
}

/** 
	Function to fetch updated GPS info from in-car sensors
*/

void CARGPSMODULE::ReadGPSInfoFromSensor(char *argv[], LOGGING_UTILITY *LogsObj) {

    /*! Creates and Connects a socket with in-car sensors to fetch the position information */
    LogsObj->LogDebug("Creating 'ZMQ_REQ' socket that connects to SUMO Data Parser");

    zmq::context_t Context(1);
    zmq::socket_t Socket(Context, ZMQ_REQ);
    std::string IP = ParserIP;
    std::string Port = ParserPort;
    Socket.connect("tcp://" + IP + ":" + Port);

    CARREQUESTTOMT RequestStr;
    RequestStr.set_vid(std::string(argv[1]));
    RequestStr.set_req("POS");
    std::string RequestBuffer;

    double TempX, TempY, TempDir;
    std::string TempLane;

    MTGPSRESPONSE ResponseStr;

    /*! Periodically(every second) send a request for the position details */
    while (CarExists) {
        RequestStr.SerializeToString(&RequestBuffer);
        zmq::message_t request(RequestBuffer.size()), Reply;
        memcpy (request.data(), RequestBuffer.c_str(), RequestBuffer.size());
        LogsObj->LogDebug("Sending message to SUMO Data Parser for updated information of Position");
        Socket.send(request);

        Socket.recv(&Reply);

        std::string ResponseBuffer(static_cast<char*>(Reply.data()), Reply.size());
        ResponseStr.ParseFromString(ResponseBuffer);

        LogsObj->LogDebug("Received message from SUMO Data Parser contains updated information of Position");

        CarExists = ResponseStr.info_exists();
        if(CarExists) {
            TempX = ResponseStr.x();
            TempY = ResponseStr.y();
            TempDir = ResponseStr.dir();
            SetGPSInfo(TempX,TempY, TempDir);
            LogsObj->LogDebug("Updated Position X = " + std::to_string(TempX) + " Y = " + std::to_string(TempY) + " Dir = " + std::to_string(TempDir));
        } else {
            LogsObj->LogDebug("Car exited from the network");
        }

        /*! One second break */
        sleep(1);
    }
    LogsObj->LogDebug("Halting Car Position Module as car do not exist in the network anymore");
    Socket.close();
}
