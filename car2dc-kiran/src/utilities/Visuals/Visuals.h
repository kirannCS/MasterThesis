//
// Created by kiran.narayanaswamy on 12/10/18.
//

#ifndef VMCPI_VISUALS_H
#define VMCPI_VISUALS_H

#include <GL/glut.h>  // GLUT, include glu.h and gl.h
#include <GL/glu.h>
#include <iostream>
#include <string>
#include <thread>
#include <zmq.hpp>
#include "../../packets/header/ClusterInfo.pb.h"
#include <sstream>

/* Holds details of the car */
struct MyCar {
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


/* Class for graphical visualization of cars */
class TRAFFIC_VISUALS {
private:
    long No_Of_Lanes;
    long TotalCars;
	/* Number of cars from each AP */
    std::map<std::string,int>NumOfCarsExisting;
	/* Details of all the cars */
    std::map<std::string, struct MyCar*> ListOfCars;
	/* Details of all APs */
    std::map<std::string, struct AP_INFO*> AP_Details;
	/* Details of all micro clouds */
    std::map<std::string, struct MC_INFO*> MC_Details;
public:
    long Road_Length;
    long Road_Width;
    long No_Of_X_Intersec;
    long No_Of_Y_Intersec;
    bool RedisplayEnable;
    std::map<std::string, std::string> Car_vs_MC;

    TRAFFIC_VISUALS();
    void DrawRoads();
    void DrawCars();
    void DrawMC();
    void StoreCarsInfo(CLUSTERINFO *);
    void SpawnAPReceivers();
    void CarVsMCDetails(CLUSTERINFO *);
    void DrawScenario();
};

#endif //VMCPI_VISUALS_H
