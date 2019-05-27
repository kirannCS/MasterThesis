//
// Created by kiran.narayanaswamy on 1/3/19.
//

#include "../../header/apps/TaskD.h"

extern zmq::socket_t* DistributeTasksToCMSocket;
extern zmq::socket_t* SendResultsToCHSocket;


/* Creates a socket that is used to assign subtasks to Cluster Members */
void CreateDistributeTasksToCMSocket(std::string IP, std::string Port) {
    zmq::context_t* Context = new zmq::context_t(1);
    DistributeTasksToCMSocket = new zmq::socket_t(*Context, ZMQ_PUSH);
    DistributeTasksToCMSocket->connect("tcp://" + IP + ":" + Port);
}

/* Creates a socket that is used to send subtasks results to Cluster Head */
void CreateSendResultsToCHSocket(std::string IP, std::string Port) {
    zmq::context_t* Context = new zmq::context_t(1);
    SendResultsToCHSocket = new zmq::socket_t(*Context, ZMQ_PUSH);
    SendResultsToCHSocket->connect("tcp://" + IP + ":" + Port);
}

int main(int argc, char *argv[]) {

    auto LogsObj = new LOGGING_UTILITY(std::string(argv[1]), "CAR", "   TASK");
    auto LocalInfoObj = new LOCAL_INFO();
    LocalInfoObj->SetCarServerIP(std::string(argv[3]));
    LocalInfoObj->AssignPorts(argv[2], LogsObj);
    auto ClusterObj = new TASK_CLUSTERINFO();
    auto CommObj = new COMMUNICATION(LogsObj);
    auto StatsObj = new STATISTICS;
    TASK_DISTRIBUTION *TaskDObj;

	/* Creates a Task Distribution Base Class object */
    if(true) {
        TaskDObj = new TASK_DISTRIBUTION(std::string(argv[1]), LogsObj, LocalInfoObj, ClusterObj, CommObj, StatsObj);
    }
    CreateDistributeTasksToCMSocket(CommObj->GetParserIP(), CommObj->GetUDMPort());
    CreateSendResultsToCHSocket(CommObj->GetParserIP(), CommObj->GetUDMPort());

    TaskDObj->TaskDistributionService();
    DistributeTasksToCMSocket->close();
    SendResultsToCHSocket->close();
    StatsObj->PrintVehStats(LogsObj);

    delete LogsObj;
    delete LocalInfoObj;
    delete CommObj;
    delete ClusterObj;
    delete TaskDObj;
    delete StatsObj;
    return 0;
}

