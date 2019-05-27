//
// Created by kiran.narayanaswamy on 11/30/18.
//

#ifndef VMCPI_DC_H
#define VMCPI_DC_H

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <list>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <dirent.h>
#include <chrono>
#include <list>


#include "../../../lib/chilkat/include/CkFileAccess.h"
#include "../../utilities/Stats.h"
#include "../../utilities/Communication.h"
#include "../../utilities/Logging.h"
#include "../../../lib/pugixml/pugixml.hpp"
#include "../../packets/header/CARClusterInfoReq.pb.h"
#include "../../packets/header/CARClusterInfoResponse.pb.h"
#include "../../header/common/DATA.h"
#include "../../header/common/Visuals.h"
#include "../../packets/header/MessageForwardFromUDM.pb.h"
#include "../../packets/header/CountData.pb.h"
#include "../../packets/header/APtoApps.pb.h"


/**
	Structure IP_PORT_INFO is used to store IP and Port address to subscribe to 
	clutering service module to receive cluster status change notification
*/
	
struct IP_PORT_INFO {
	std::string IP;
	std::string Port;
};


/**
	Class for Data collection in AP
*/

class AP_DATA_COLLECTION {
private:
	/* Path where data sent from CH is stored in files */
    std::string APDataFilePath;
    std::string ID;

public:
    std::string GetID() {
        return ID;
    }
    bool APExists;
    AP_DATA_COLLECTION(std::string);
    LOGGING_UTILITY *LogsObj;
    COMMUNICATION *CommObj;
    STATISTICS *StatsObj;
    void DataCollectionService();
    void StoreData(ALLDATA*);
    void ProcessMessage(std::string);
    void MessageToVisuals();
    struct IP_PORT_INFO *IP_Port_Info;
	/* Contains IP address and port number details to push application data to visual component */
    struct Visuals_IP_Info *VisualsIPInfo;

};

#endif //VMCPI_DC_H
