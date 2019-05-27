
#######################################################################################
########### Used to receive commands from Helper module ###############################
########### Used to spawn vehicle and AP processes ####################################
########### After one experiment run on receiving message it cleans up previous data ##
#######################################################################################

import os
import sys
sys.path.append('../src/packets/header/')

import time
import zmq
import DistributeProcesses_pb2

import netifaces as ni
ni.ifaddresses('eth0')
ip = ni.ifaddresses('eth0')[ni.AF_INET][0]['addr']
print ip

if len(sys.argv) != 2:
	print("Usage: CommandReceiver.py 'Trace_IP_ADDRESS'")
	exit()

context = zmq.Context()
subscriber = context.socket(zmq.SUB)
subscriber.connect("tcp://"+sys.argv[1]+":6668")

try:
    subscriber.setsockopt(zmq.SUBSCRIBE, ip)
except TypeError:
    subscriber.setsockopt_string(zmq.SUBSCRIBE, ip)

Message = DistributeProcesses_pb2.PROCESS_DISTRIBUTION()
os.system("ulimit -n 40960")


while True:
    	#  Wait for next request from client
    	[address, contents] = subscriber.recv_multipart()
	Message.ParseFromString(contents)

	if Message.ID == "-1":
		os.system("python cleanup.py 2 1")

	else :
		if Message.MODULE_TYPE == "CAR":
			if Message.RUN_IN_FOREGROUND:
				os.system("gnome-terminal -x sh -c \"python helper.py "+ Message.ID + " " + Message.START_PORT_NO + " " + ip + "\""+"\"; bash\"")
			else:
				os.system("python helper.py "+ Message.ID + " " + Message.START_PORT_NO + " " + ip + " &")

		elif Message.MODULE_TYPE == "BS":
			if Message.RUN_IN_FOREGROUND:
				os.system("gnome-terminal -x sh -c \"valgrind --tool=memcheck --leak-check=full ./BS "+ Message.ID + "\""+"\"; bash\"")
			else:
				#os.system("valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./BS "+ Message.ID + " &")
				os.system("../bin/BS "+ Message.ID + " &")

		elif Message.MODULE_TYPE == "RSU":
			if Message.RUN_IN_FOREGROUND:
				os.system("gnome-terminal -x sh -c \"./RSU "+ Message.ID + "\""+"\"; bash\"")
			else:
				os.system("../bin/RSU "+ Message.ID + " &")
		
		if Message.MODULE_TYPE == "CAR":
			print("New Process started ", Message.RUN_IN_FOREGROUND, Message.ID, Message.START_PORT_NO, ip)
		elif Message.MODULE_TYPE == "BS" or Message.MODULE_TYPE == "RSU":
			print("New Process started ", Message.RUN_IN_FOREGROUND, Message.ID, ip)
