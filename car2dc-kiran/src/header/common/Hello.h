//
// Created by kiran.narayanaswamy on 10/24/18.
//

#ifndef VMCPI_HELLO_H
#define VMCPI_HELLO_H

struct HELLO_PACKET {
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

#endif //VMCPI_HELLO_H
