//
// Created by kiran.narayanaswamy on 10/24/18.
//

#ifndef VMCPI_RSUBEACON_H
#define VMCPI_RSUBEACON_H

struct RSUBEACON {
    std::string Dataline1;
    std::string Dataline2;
    std::string SrcID;
    double X;
    double Y;
    std::string Datatype;
};

#endif //VMCPI_RSUBEACON_H
