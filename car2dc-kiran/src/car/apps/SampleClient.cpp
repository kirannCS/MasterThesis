#include <string>
#include <stdlib.h>
#include <fstream>
#include <dirent.h>
#include <chrono>
#include <iostream>
#include <zmq.hpp>
#include <thread>
#include <unistd.h>
#include <list>


#include "../../../lib/chilkat/include/CkFileAccess.h"
#include "../../packets/header/SampleData.pb.h"

zmq::socket_t* TransferDataToAPSocket;
zmq::socket_t* TransferBeaconToAPSocket[50];
void T_SendDataToAP();
int DataSize = 400;
int ChunkSize = 1;

void CreateTransferDataToAPSocket(std::string IP, std::string Port) {
    zmq::context_t* Context = new zmq::context_t(1);
    TransferDataToAPSocket = new zmq::socket_t(*Context, ZMQ_PUSH);
    TransferDataToAPSocket->connect("tcp://" + IP + ":" + Port);
}

void CreateTransferBeaconSocket(std::string IP, std::string Port) {
	for(int i = 0; i < 50; i++) {
		zmq::context_t* Context = new zmq::context_t(1);
    		TransferBeaconToAPSocket[i] = new zmq::socket_t(*Context, ZMQ_PUSH);
    		(TransferBeaconToAPSocket[i])->connect("tcp://" + IP + ":" + Port);
	}
}
bool SplitFile(std::string Path, std::string Filename) {
    Filename = Path + Filename;
    int TempChunkSize = ChunkSize * 1024;
    CkFileAccess fac;
    const char *partPrefix = Filename.c_str();
    const char *partExtension = "part";
    bool success = fac.SplitFile(Filename.c_str(),partPrefix,partExtension,TempChunkSize,Path.c_str());

    if (!success) {
        std::cout << fac.lastErrorText() << " " << Path.c_str() <<  "\r\n";
	return false;
    }
	return true;
}

void T_SendBeacons(int Bytes, int ThreadNum) {
	//std::cout << "Bytes = " << Bytes << std::endl;
	for(int count = 1; count <= 600; count++) {
		std::string Data(Bytes, 'H');
		std::string DataToSend;
		TEST_DATA DataPkt;
		DataPkt.set_data(Data);
		DataPkt.set_chunknum(count);
		DataPkt.set_datatype("Beacon");
		DataPkt.SerializeToString(&DataToSend);
        	zmq::message_t Packet(DataToSend.size());
        	memcpy ((void*)Packet.data(), (void*)DataToSend.data(), DataToSend.length());
        	(TransferBeaconToAPSocket[ThreadNum])->send(Packet);
		//usleep(1000000);
		sleep(1);
	}
	
}


int main(int argc, char*argv[]) {
	std::string UDMIP = "131.234.28.26";
	std::string UDMPort = "8500";
	CreateTransferDataToAPSocket(UDMIP, UDMPort);
	CreateTransferBeaconSocket(UDMIP, UDMPort);
	std::vector<std::thread> threads;
	std::thread SendDataToAP(T_SendDataToAP);
	for (int i=0; i<50; ++i) {
    		threads.push_back(std::thread(T_SendBeacons,180, i));
	}
//wait for them to complete
	for (auto& th : threads) 
    	th.join();
	SendDataToAP.join();
	return 0;
}
void T_SendDataToAP() {	
	std::string Path = "../results/sample_generated_files/";
	std::string Filename = "Data";

	std::string Command = "truncate -s " + std::to_string(DataSize)+"K " + Path + Filename;
        system(Command.c_str());
	if(SplitFile(Path, Filename)) {
		for (int Count = 1; Count <= 150; Count++) {
			std::cout << ".............................................. " << Count << std::endl;
			//SplitFile(Path, Filename+"["+std::string(Count)+"]");
			int TotalTransfers = (DataSize) / ChunkSize;
			
			for (int i = 1; i <= TotalTransfers; i++) {
				TEST_DATA DataPkt;
			        if (i == TotalTransfers)
					DataPkt.set_lastpkt(true);
				else 
					DataPkt.set_lastpkt(false);
				
				DataPkt.set_chunknum(i);
				DataPkt.set_datatype("Data");
				DataPkt.set_filename(Filename+"["+std::to_string(Count)+"]");
				std::string SendFileName = Path+Filename+std::to_string(i)+".part";
        			std::ifstream ifs(SendFileName);
			        std::string Content( (std::istreambuf_iterator<char>(ifs) ),(std::istreambuf_iterator<char>()) );
        			DataPkt.set_data(Content);
				DataPkt.set_dest_id("AP1ID");
				std::string DataToSend;
				DataPkt.SerializeToString(&DataToSend);
				zmq::message_t Packet(DataToSend.size());
				memcpy ((void*)Packet.data(), (void*)DataToSend.data(), DataToSend.length());
				TransferDataToAPSocket->send(Packet);
				usleep(5000);
			}
			usleep(4000000);
		}
	}
}		
