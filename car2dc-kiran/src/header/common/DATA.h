//
// Created by kiran on 10/28/18.
//


#ifndef VMCPI_DATA_H
#define VMCPI_DATA_H

#include <string>

struct DATA {
    std::string Dataline1;
    std::string Dataline2;
    std::string SrcID;
    std::string Timestamp;
    std::string Datatype;
    std::string Data;
    std::string ClusterID;
    std::string Filename;
    int ChunkNum;
    bool LastPkt;
    unsigned long long StartTime;
    unsigned long long ClusterSeqNum;
};

#endif //VMCPI_DATA_H
