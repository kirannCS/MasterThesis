#include <string>
#include <stdlib.h>
#include <fstream>
#include <dirent.h>
#include <chrono>
#include <iostream>
#include <zmq.hpp>
#include <thread>
#include <unistd.h>


#include "../../../lib/chilkat/include/CkFileAccess.h"
#include "../../packets/header/SampleData.pb.h"

void StoreData(TEST_DATA *DataString) {
    unsigned long long Start;
    std::string Path = "../results/sample_received_files/";
    std::string OutputFilename = Path + DataString->filename() + std::to_string(DataString->chunknum())+".part";
    std::ofstream Out(OutputFilename);
    std::string Data = DataString->data();
    Out << Data;
    Out.close();
    if(DataString->lastpkt()) {
	 CkFileAccess fac;
        std::string reassembledFilename = Path + DataString->filename();
        const char *partPrefix = DataString->filename().c_str();
        const char *partExtension = "part";

        int Success = fac.ReassembleFile(Path.c_str(),partPrefix,partExtension,reassembledFilename.c_str());
        if (Success) {
            std::cout << "Success.................................................... AP" << DataString->filename() << "\r\n";
        }
        else {
            std::cout << fac.lastErrorText() << "\r\n";
        }
        int Count = 1;
        while(Count <= DataString->chunknum()) {
            std::string RemoveFile = Path + DataString->filename() + std::to_string(Count) + ".part";
            remove(RemoveFile.c_str());
            Count++;
        }
    }
}




int main(int argc, char* argv[]) {
	zmq::context_t context(1);
    	zmq::socket_t Subscriber (context, ZMQ_SUB);
	std::string IP = "131.234.28.26";
	std::string Port = "8501";
	
	Subscriber.connect("tcp://" + IP + ":" + Port);
    	Subscriber.setsockopt(ZMQ_SUBSCRIBE, "AP1ID", std::strlen("AP1ID"));
    	zmq::message_t MessageReceived;
    	zmq::message_t Address;
	int count = 0;

	while (true) {
		Subscriber.recv(&Address);
            	Subscriber.recv(&MessageReceived);
//		std::cout << "Message Received\n";
		
		std::string NewMessageBuffer(static_cast<char *>(MessageReceived.data()), MessageReceived.size());
		TEST_DATA Message;
		Message.ParseFromString(NewMessageBuffer);
		if(Message.datatype() == "Data")
			StoreData(&Message);
		else if(Message.datatype() == "Beacon") {
			if(count % 250 == 0)
				std::cout << "Beacon " << Message.chunknum() << std::endl;
			count++;
			usleep(0000050);
		}
			
		usleep(0010000);
	
	}
	return 0;
}



