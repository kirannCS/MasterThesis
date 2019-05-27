//
// Created by kiran.narayanaswamy on 10/24/18.
//

#ifndef VMCPI_LOCALINFO_H
#define VMCPI_LOCALINFO_H

struct UPDATE_LOCALINFO {
    std::string Dataline1;
    std::string Dataline2;
    std::string SrcID;
    double X;
    double Y;
    double Speed;
    double Direction;
    std::string Timestamp;
    std::string Datatype;
};

#endif //VMCPI_LOCALINFO_H
