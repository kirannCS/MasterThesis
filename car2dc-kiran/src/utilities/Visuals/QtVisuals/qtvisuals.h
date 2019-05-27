#ifndef QTVISUALS_H
#define QTVISUALS_H

#include <iostream>
#include <string>
#include <thread>
#include <zmq.hpp>
#include "../../../packets/header/ClusterInfo.pb.h"
#include "../../../packets/header/CountData.pb.h"
#include <sstream>
#include <QDebug>
#include <QtGui>
#include <qobject.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <unistd.h>

/* Holds details of the car */
struct CAR_INFO {
    std::string ID;
    double X;
    double Y;
    std::string State;
    double Dir;
    double Speed;
    std::string Manager;
};

/* Holds details of AP */
struct AP_INFO {
    std::string ID;
    std::string IP;
    std::string Port;
    double PosX;
    double PosY;
};

/* Holds details of micro cloud */
struct MC_INFO {
    std::string ID;
    double PosX;
    double PosY;
    std::string Manager;
    double Radius;

};

/* Holds details of Data collection and aggregation application data */
struct DATA_COUNT {
    long DataSent;
    long DataCountPreviousInterval;
};


/* Holds details of Task distribution application data */
struct TASK_INFO {
	/* Total subtasks assigned to CMs */
    long TotalSubTasks;
	/* Total subtasks results it received from CMs */
    long TotalSubTasksResults;
	/* Total subtasks results it has computed */
    long TotalSubTasksComputed;
};

class VISUALS {
private:
    long TotalCars;
    bool RedisplayEnable = false;
public:
    VISUALS();
    std::string ReceiverIP; 
    std::string ReceiverPort; 
	/* stores details of all APs */
    std::map<std::string, struct AP_INFO*> AP_Details;
	/* stores details of all the cars */
    std::map<std::string, struct CAR_INFO*> ListOfCars;
	/* stores details of all the micro clouds */
    std::map<std::string, struct MC_INFO*> MC_Details;
	/* Number of cars from each AP */
    std::map<std::string, int> NumOfCarsExisting;
	/* Details of cars in cluster <Car ID, Cluster ID> */
    std::map<std::string, std::string> Car_vs_MC;
	/* Cars and APs application details */
    std::map<std::string, struct DATA_COUNT*> DataCount;
    std::map<std::string, struct TASK_INFO*> TaskInfo;
    void StoreCarsInfo(CLUSTERINFO *);
    std::map<std::string, struct AP_INFO*> GetAPDetails() {
        return AP_Details;
    }
    void DisplayCarsInfo(void *);
    void CarVsMCDetails(CLUSTERINFO *);
    void ProcessReceivedMsg(std::string);
private slots:
    void on_pushButton_clicked();
};


#endif // QTVISUALS_H
