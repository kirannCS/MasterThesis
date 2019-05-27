//
// Created by kiran.narayanaswamy on 10/24/18.
//

#ifndef VMCPI_CLUSTERINFO_H
#define VMCPI_CLUSTERINFO_H

struct CLUSTER_INFO {
    std::string Dataline1;
    std::string Dataline2;
    std::string CH;
    std::string ListOfCM;
    std::string MC_ID;
    std::string Datatype;
    std::string Timestamp;
    int ClusterSeqNum;
};

#endif //VMCPI_CLUSTERINFO_H
