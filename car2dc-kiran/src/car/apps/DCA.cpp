//
// Created by kiran on 10/27/18.
//

#include "../../header/apps/DCA.h"


extern zmq::socket_t* TransferDataToCHSocket;
extern zmq::socket_t* TransferDataToAPSocket;


/* Creates a socket that is used to transfer data from Cluster Member to Cluster Head */
void CreateTransferDataToCHSocket(std::string IP, std::string Port) {
    zmq::context_t* Context = new zmq::context_t(1);
    TransferDataToCHSocket = new zmq::socket_t(*Context, ZMQ_PUSH);
    TransferDataToCHSocket->connect("tcp://" + IP + ":" + Port);
}


/* Creates a socket that is used to transfer data to Cluster Head to Access Point */
void CreateTransferDataToAPSocket(std::string IP, std::string Port) {
    zmq::context_t* Context = new zmq::context_t(1);
    TransferDataToAPSocket = new zmq::socket_t(*Context, ZMQ_PUSH);
    TransferDataToAPSocket->connect("tcp://" + IP + ":" + Port);
}



int main(int argc, char *argv[]) {
    LOGGING_UTILITY LogsObj(std::string(argv[1]), "CAR", "    DCA");
    LOCAL_INFO LocalInfoObj;
    LocalInfoObj.SetCarServerIP(std::string(argv[3]));
    DCA_CLUSTERINFO DCAClusterObj;
    COMMUNICATION CommObj(&LogsObj); 
    STATISTICS StatsObj;
    LocalInfoObj.AssignPorts(argv[2], &LogsObj);

    /* Creates a Data collection Base Class object */
    DCA_BASE_DATACOLLECTION *DCAObj;
    if(true) {
        DCAObj = new DCA_DATACOLLECTION(std::string(argv[1]), &LogsObj, &LocalInfoObj, &DCAClusterObj, &CommObj, &StatsObj);
    }

    CreateTransferDataToCHSocket(CommObj.GetParserIP(), CommObj.GetUDMPort());
    CreateTransferDataToAPSocket(CommObj.GetParserIP(), CommObj.GetUDMPort());

    DCAObj->DataCollectionService();
    TransferDataToAPSocket->close();
    TransferDataToCHSocket->close();
    StatsObj.PrintVehStats(&LogsObj);

    delete DCAObj;
    return 0;
}
