#include "qtvisuals.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
extern class VISUALS *VisualObj;
#include "../../../../lib/pugixml/pugixml.hpp"
#include "../../../../lib/pugixml/pugixml.cpp"

/* Enables to refresh cars and APs listing widget */
bool EnableRefresh = false;
/* Handles Pause button */
bool ExternalRefreshQT = true;
/* Enables to refresh Microcloud details listing widget */
bool EnableClusterInfoDsiplay = false;

/** 
        Constructor that reads XML Config file to populate required variables 
*/
VISUALS::VISUALS() {
    pugi::xml_document doc;
    pugi::xml_node tools = doc.child("VMCPI").child("config");
	/* Reads Total cars, IP and Port details (at which nodes push application data) */
    if (!doc.load_file("../../../../config/visuals/config.xml")) {
        std::cout << "ERROR:: Config File Missing .../../../../config/visuals/config.xml !!\n";
    } else {
        tools = doc.child("VMCPI").child("config");

        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                if (name == "TotalCars")
                    TotalCars = std::stol(ait->value());
                else if (name == "IP")
                    ReceiverIP = ait->value();
                else if(name == "Port")
                    ReceiverPort = ait->value();

            }
        }
    }

	/* Reads all APs details (BS and RSU) */
    if (!doc.load_file("../../../../config/ap/config.xml")) {
        std::cout << "ERROR:: Config File Missing ../../../../config/ap/config.xml !!\n";
    } else {
        tools = doc.child("VMCPI").child("config");
        std::string Fname, Sname, Value;
        std::string AttrName, Attrvalue, Manager;
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            Fname = it->name();
            if (Fname == "BS" || Fname == "RSU") {
                auto Obj = new struct AP_INFO;
                for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                    Sname = ait->name();
                    Value = ait->value();
                    if (Sname == "id")
                        Obj->ID = Value;
                    else if (Sname == "ip")
                        Obj->IP = Value;
                    else if (Sname == "port")
                        Obj->Port = Value;
                    else if (Sname == "x")
                        Obj->PosX = std::stod(Value);
                    else if (Sname == "y")
                        Obj->PosY = std::stod(Value);
                }
                AP_Details[Obj->ID] = Obj;
                ListOfCars[Obj->ID] = new struct CAR_INFO[TotalCars + 5];
            } else if (Fname == "MICROCLOUD") {
                auto Obj = new struct MC_INFO;
                for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                    Sname = ait->name();
                    Value = ait->value();
                    if (Sname == "id")
                        Obj->ID = Value;
                    else if (Sname == "x")
                        Obj->PosX = std::stod(Value);
                    else if (Sname == "y")
                        Obj->PosY = std::stod(Value);
                    else if (Sname == "manager")
                        Obj->Manager = Value;
                    else if (Sname == "mc_radius")
                        Obj->Radius = std::stod(Value);
                }
                MC_Details[Obj->ID] = Obj;
            }
        }
        for(auto it = AP_Details.begin(); it != AP_Details.end(); it++) {
            auto Obj = it->second;
        }
        for(auto it = MC_Details.begin(); it != MC_Details.end(); it++) {
            auto Obj = it->second;
        }
    }
}

/**
	Receives messages from car application services such as Data collection and task distribution 
*/

void T_Receiver() {
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_PULL);
    socket.bind("tcp://"+VisualObj->ReceiverIP+":"+VisualObj->ReceiverPort);
    zmq::message_t newMessage;
    while(true) {
        socket.recv(&newMessage);
        std::string ReceivedMsg(static_cast<char *>(newMessage.data()), newMessage.size());
        VisualObj->ProcessReceivedMsg(ReceivedMsg);
    }
}

/**
	Process receive messages
	Extract details of the applications from the message and stores it
*/

void VISUALS::ProcessReceivedMsg(std::string ReceivedMsg){
    VISUALSDATA NewMessage;
    NewMessage.ParseFromString(ReceivedMsg);
    if(NewMessage.messagetype() == "DATA") {
        if(NewMessage.srctype() == "AP") { //Data collection details from AP perspective
            if(DataCount.find(NewMessage.srcid()) != DataCount.end()) {
                auto PrevDataInfo = DataCount[NewMessage.srcid()];
                auto PrevDataCount = DataCount[NewMessage.srcid()]->DataSent;
                PrevDataInfo->DataSent = NewMessage.datacount();
                PrevDataInfo->DataCountPreviousInterval = PrevDataInfo->DataSent - PrevDataCount;
                DataCount[NewMessage.srcid()] = PrevDataInfo;
            } else {

                auto CountDataInfo = new struct DATA_COUNT();
                CountDataInfo->DataSent = NewMessage.datacount();
                CountDataInfo->DataCountPreviousInterval = CountDataInfo->DataSent;
                DataCount[NewMessage.srcid()] = CountDataInfo;

            }
        } else if(NewMessage.srctype() == "CAR") { // Data collection details from car perspective
            if(DataCount.find(NewMessage.srcid()) != DataCount.end()) {
                auto CountPrevDataInfo = DataCount[NewMessage.srcid()];
                auto PrevDataCount = DataCount[NewMessage.srcid()]->DataSent;
                CountPrevDataInfo->DataSent = NewMessage.datacount();
                CountPrevDataInfo->DataCountPreviousInterval = CountPrevDataInfo->DataSent - PrevDataCount;
                DataCount[NewMessage.srcid()] = CountPrevDataInfo;

            } else {
                auto CountDataInfo = new struct DATA_COUNT();
                CountDataInfo->DataSent = NewMessage.datacount();
                CountDataInfo->DataCountPreviousInterval = CountDataInfo->DataSent;
                DataCount[NewMessage.srcid()] = CountDataInfo;

            }
        }
    } else if(NewMessage.messagetype() == "TASK") { //Task distribution application details
        if(TaskInfo.find(NewMessage.srcid()) != TaskInfo.end()) {
           auto PrevTaskInfo = TaskInfo[NewMessage.srcid()];
           PrevTaskInfo->TotalSubTasks = NewMessage.totalsubtasks();
           PrevTaskInfo->TotalSubTasksResults = NewMessage.totalsubtasksresults();
           PrevTaskInfo->TotalSubTasksComputed = NewMessage.totalsubtaskscomputed();
        } else {
            auto NewTaskInfo = new struct TASK_INFO();
            NewTaskInfo->TotalSubTasks = NewMessage.totalsubtasks();
            NewTaskInfo->TotalSubTasksResults = NewMessage.totalsubtasksresults();
            NewTaskInfo->TotalSubTasksComputed = NewMessage.totalsubtaskscomputed();
            TaskInfo[NewMessage.srcid()] = NewTaskInfo;
        }
    }
}

/**
	Subscribes to AP clustering service to receive cluster status details
	AP publish it every cluster computation interval
*/

void T_ReceiveMsgs(struct AP_INFO* APInfoObj) {
    auto Context = new zmq::context_t(1);
    auto Subscriber = new zmq::socket_t(*Context, ZMQ_SUB);
    std::string IP_Port = APInfoObj->IP + ":" + std::to_string((std::stoi(APInfoObj->Port) + 1));
    qDebug() << " IP " << IP_Port.c_str() << "\n";
    Subscriber->connect("tcp://"+IP_Port);
    Subscriber->setsockopt(ZMQ_SUBSCRIBE, "AP_INFO", 7);
    zmq::message_t MessageReceived, Address;
    while (true) {
        Subscriber->recv(&Address);
        Subscriber->recv(&MessageReceived);
        std::string ReceivedMsg(static_cast<char *>(MessageReceived.data()), MessageReceived.size());
        CLUSTERINFO ClusterInfo;
        ClusterInfo.ParseFromString(ReceivedMsg);
        if(ClusterInfo.datatype() == "CARINFO") { //When AP publishes cars details(ID, position speed etc.)
            VisualObj->StoreCarsInfo(&ClusterInfo);
        } else if(ClusterInfo.datatype() == "CLUSTERINFO") { //When AP publishes cluster status details
            EnableClusterInfoDsiplay = true;
             VisualObj->CarVsMCDetails(&ClusterInfo);
        }
    }
}

/**
	Extract from cluster status message received from AP and 
	stores details in map <car ID, cluster ID it belongs to> 
*/
void VISUALS::CarVsMCDetails(CLUSTERINFO *CarsInfo) {
    std::string CarsInfoStr = CarsInfo->cmlist();
    std::stringstream ss(CarsInfoStr);
    while( ss.good() ) {
        std::string substr;
        getline(ss, substr, ',');
        if (!substr.empty()) {
            VisualObj->Car_vs_MC[substr] = CarsInfo->clusterid();
        }
    }
}


/**
	Extract from car details message received from AP and 
	stores the details of cars
*/

void VISUALS::StoreCarsInfo(CLUSTERINFO *CarsInfo) {
    std::string CarsInfoStr = CarsInfo->cmlist();
    std::string SrcID = CarsInfo->src_id();
    std::stringstream ss(CarsInfoStr);
    int i = 0, j = 0;
    int NoOfValues = 6;
    auto CarsList = ListOfCars[CarsInfo->src_id()];
    while( ss.good() ) {
        std::string substr;
        getline( ss, substr, ',' );
        if(!substr.empty()) {
            switch(i % NoOfValues) {
                case 0: CarsList[j].ID = substr;
                        break;
                case 1: CarsList[j].X = std::stod(substr);
                        break;
                case 2: CarsList[j].Y = std::stod(substr);
                        break;
                case 3: CarsList[j].Speed = std::stod(substr);
                        break;
                case 4: CarsList[j].Dir = std::stod(substr);
                        break;
                case 5: CarsList[j].State = substr;
                        CarsList[j].Manager = SrcID;
                        j++;
            }
        }
        i++;
    }
    NumOfCarsExisting[CarsInfo->src_id()] = j;
    EnableRefresh = true;
}

/*
void VISUALS::DisplayCarsInfo(void *MainWindowObj) {
    //Ui::MainWindow *ui = (Ui::MainWindow*) MainWindowObj;
}

void VISUALS::on_pushButton_clicked()
{
    //
}*/
