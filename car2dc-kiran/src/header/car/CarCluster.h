//
// Created by kiran on 8/3/18.
//

#ifndef VMCPI_CARCLUSTERING_H
#define VMCPI_CARCLUSTERING_H

#include "../common/LocalInfo.h"
#include "../common/ClusterInfo.h"
#include "../common/Hello.h"
#include "../common/RSUBeacon.h"
#include <fstream>

#include "../../packets/header/CARClusterInfoReq.pb.h"
#include "../../packets/header/CARClusterInfoResponse.pb.h"

class VEHICLE;
class NEIGHBORTABLE;

/**
	Class related to clustering service module in Car
*/

class CAR_CLUSTER {
private:
    std::string ID;
    std::string ClusterHead;
    std::list<std::string> ClusterMembers;
    std::string MicrocloudID;
	/* Holds the state of the car 'CH' OR 'CM' OR 'INIT' */
    std::string ClusterState;
	/* Timestamp at which the state is changed */
    time_t ClusterStateInitialTimer;
	/* Time in seconds after which cluster state is invalid */
    double ClusterStateExpireTimer;
	/* Interval with which car's local informaation is uploaded to AP */
    double LocalInfoInterval;
	/* Is set to true when there is change in cluster state, so that change is notified to all applications */
    bool PublishClusterInfo;
	/* Total number of cluster computation intervals so far elapsed */
    int ClusterSeqNum;

public:
    void SetPublishClusterInfo(bool Val) {
        PublishClusterInfo = Val;
    }
    std::tuple<time_t, double, std::string> GetClusterStateAndTimers(){
        return std::make_tuple(ClusterStateInitialTimer, ClusterStateExpireTimer, ClusterState);
    }
    void SetClusterState(std::string CurrentState) {
        ClusterState = CurrentState;
    }
    void SetLocalInfoInterval(double interval) {
        LocalInfoInterval = interval;
    }
    double GetLocalInfoInterval() {
        return LocalInfoInterval;
    }
    std::string GetClusterState() {
        return ClusterState;
    }
	/* ID of the AP to which local info is currently being sent to (BS or RSU) */
    std::string CurrentClusterManager;
	/* ID of the AP by default to which car sends its local info */
    std::string DefaultClusterManager;
	/* Interval after which Cluster manager (if not default) is invalid */
    double ClusterManagerExpiryTimer;
	/* Timestamp at which cluster manager is changed to (RSU ID) from default cluster manager (generally BS) */
    time_t ClusterManagerInitTimer;
    CAR_CLUSTER(LOGGING_UTILITY *, std::string);
    void SendVehUpdatedInfo(VEHICLE *, BASE_COMMUNICATION *);
    void StoreClusterInfo(CLUSTER_INFO *, std::string);
    void ClusterServer(VEHICLE *);
    LOGGING_UTILITY *LogsObj;
};


/**
	Class related to Neighbor table in the car
	Holds information related to all its one hop neighbors 
	Not used in the thesis
*/

class NEIGHBORTABLE {
private:
    std::string VID;
    double X;
    double Y;
    double Speed;
    double Direction;
    time_t Timestamp;
    double NTExpiryTimer;
    double BeaconInterval;
    std::map<std::string, NEIGHBORTABLE*> NeighborTable;

public:
    void SendHelloPackets(VEHICLE *);
    void ProcessHelloPackets(HELLO_PACKET*);
    void AddToNeighborTable(std::string, NEIGHBORTABLE*);
    void ValidateNTEntries();
    void DelFromNeighborTable(std::string);
    LOGGING_UTILITY *LogsObj;
    std::map<std::string, NEIGHBORTABLE*> GetCopyNeighborTable() {
        return NeighborTable;
    };
    void SetNTExpiryTimer(double TVal) {
        NTExpiryTimer = TVal;
    }
    void SetBeaconInterval(double interval) {
        BeaconInterval = interval;
    }
    double GetBeaconInterval() {
        return BeaconInterval;
    }

    ~NEIGHBORTABLE() {
        for(auto it = NeighborTable.begin(); it != NeighborTable.end(); it++) {
            delete it->second;
            NeighborTable.erase(it->first);
        }
    }
};

#endif //VMCPI_CARCLUSTERING_H
