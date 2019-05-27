//
// Created by kiran.narayanaswamy on 1/3/19.
//

#ifndef VMCPI_TASK_H
#define VMCPI_TASK_H

struct TASK {
    unsigned int TaskSeqNum;
    long MaxTime;
    std::string SrcID;
    std::string DatatType;
    std::string Timestamp;
};

struct TASK_RESULTS {
    unsigned int TaskSeqNum;
    unsigned int FinishTime;
    std::string SrcID;
    std::string DatatType;
    std::string Timestamp;
    std::string Result;
};
#endif //VMCPI_TASK_H
