//
// Created by kiran on 8/2/18.
//

/**
	Constructors regarding to Car Node is defined in this .cpp file
*/

#include "../../header/car/CarHeader.h"
#include "../../header/car/CarCluster.h"
#include "../../../lib/pugixml/pugixml.hpp"

/** 
        Constructor that reads XML Config file to populate required variables related to car
*/

VEHICLE::VEHICLE(LOGGING_UTILITY *_LogsObj, NEIGHBORTABLE *_NTObj, CAR_CLUSTER *_ClusterObj) {
    LogsObj = _LogsObj;
    NTObj = _NTObj;
    ClusterObj = _ClusterObj;
    pugi::xml_document doc;
    if (!doc.load_file("../config/car/config.xml")) {
        LogsObj->LogError("ERROR:: Config File Missing ../config/car/config.xml !!\n") ;
    }

    else {
        pugi::xml_node tools = doc.child("VMCPI").child("config");
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                if (name == "MessageSendService")
                    MessageSendService = ait->value();
                else if (name == "NeighborTableEntryTimer")
                    NTObj->SetNTExpiryTimer(std::stod(ait->value()));
                else if (name == "BeaconInterval")
                    NTObj->SetBeaconInterval(std::stod(ait->value()));
                else if (name == "UpdateToAPInterval")
                    ClusterObj->SetLocalInfoInterval(std::stod(ait->value()));
            }
        }
    }
    CarExists = true;
}

/** 
        Constructor that reads XML Config file to populate required variables related to car cluster service
*/

CAR_CLUSTER::CAR_CLUSTER(LOGGING_UTILITY *_LogsObj, std::string VID) {
    pugi::xml_document doc;
    ClusterSeqNum = 0;
    if (!doc.load_file("../config/car/config.xml")) {
        _LogsObj->LogError("ERROR:: Config File Missing ../config/car/config.xml !!\n");
    } else {
        pugi::xml_node tools = doc.child("VMCPI").child("config");

        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                if (name == "DefaultClusterManager") {
                    CurrentClusterManager = ait->value();
                    DefaultClusterManager = ait->value();
                }
                else if (name == "ClusterManagerExpiryTimer")
                    ClusterManagerExpiryTimer = std::stod(ait->value());
                else if (name == "ClusterStateExpireTimer")
                    ClusterStateExpireTimer = std::stod(ait->value());
            }
        }
    }
    ID = VID;
    ClusterState = "INIT";
    if(true) {
        _LogsObj->LogDebug(
                "VID = " + ID + " ClusterManager = " + CurrentClusterManager + " ClusterManagerExpiryTimer = " +
                std::to_string(ClusterManagerExpiryTimer) + " ClusterStateExpireTimer = " +
                std::to_string(ClusterStateExpireTimer) + " ClusterState = " + ClusterState);
    }
}


/**
	Assigns port numbers to different in-car modules (uses start portnumber and a fixed offset) 
*/
void LOCAL_INFO::AssignPorts(char *StartingPortNum, LOGGING_UTILITY *LogsObj){
    int PortNum = atoi(StartingPortNum);
    CarSpeedPort = std::to_string(PortNum + 1);
    CarPosPort = std::to_string(PortNum + 2);
    CarClusterPort = std::to_string(PortNum);
    LogsObj->LogDebug("Port Assignment:: Car Cluster Port = " + std::to_string(PortNum) + " Port for Car Speed Module = " + CarSpeedPort +  " Port for Car Position Module = \
               " + CarPosPort);
}

