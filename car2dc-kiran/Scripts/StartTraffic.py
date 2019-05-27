#!/usr/bin/env python3

#################################################################################
################# Helper Module #################################################
################# Provides abstraction to car sensors and PHY layer #############
#################################################################################


from __future__ import absolute_import
from __future__ import print_function

import os
import sys
sys.path.append('../src/packets/header/')

# Import proto modules
import CARRequestToMT_pb2
import MTGPSResponse_pb2
import MTSpeedResponse_pb2
import MsgFromNodeToUDM_pb2
import MessageForwardFromUDM_pb2
import BigData_pb2
import DistributeProcesses_pb2

# Import other libraries
import optparse
import subprocess
import random
import time
import zmq
import thread
import json
import xml.etree.ElementTree as ET
import netifaces as ni
import math 
import sys
import linecache
import datetime
import threading
import base64
from threading import Lock, Thread
import time
from xmlr import xmliter


# Uncomment when required debugging
# debugger proc 
"""def traceit(frame, event, arg):
    if event == "line":
        lineno = frame.f_lineno
        filename = frame.f_globals["__file__"]
        if (filename.endswith(".pyc") or
            filename.endswith(".pyo")):
            filename = filename[:-1]
        name = frame.f_globals["__name__"]
        line = linecache.getline(filename, lineno)
        print(name, lineno, line.rstrip())
    return traceit"""

# Global Vaiables
ParserIP = ""
ParserPort = ""
UDMPort = ""
Keys = []
SumoFloatingDataPath = ""
VehInfoHashMap = {}
APInfoHashMap = {}
StartPort = 12000
Incrementor = 0
AllAddrTable = {}
CommRange = 0.0

LogInfoEnable = ""
LogInfoFile = ""
LogInfoStdOutput = ""
LogDebugEnable = ""
LogDebugFile = ""
LogDebugStdOutput = ""
LogStatsEnable = ""
LogStatsFILE = ""
LogStatsStdOutput = ""
LogErrorEnable = ""
LogErrorFILE = ""
LogErrorStdOutput = ""
LogFilePath = ""
ExperimentNumber = ""
RunInForeGround = "" 
RunInForeGroundList = ""
LogFile = ""
UDMPublisher = "NULL"
UDMExitPublisher = "NULL"

SystemsIP = []
SystemsIPSubscript = 0
DistributedSystemsPublisher = ""
NOTFinished = True
lock = Lock()


# Converts string into list which has literals separated by commas and returns it
def ConvertStringToList(string):
    li = list(string.split(" "))
    return li


# Starts a car process in different terminal, takes vid(Vehicle ID) and starting port number helps car to spawn in-car processes
# incrementor determines the number of ports could be reserved for each car 
def start_carProc(vid):
	global StartPort, Incrementor, RunInForeGround, RunInForeGroundList, SystemsIPSubscript, SystemsIP, DistributedSystemsPublisher

	Message = DistributeProcesses_pb2.PROCESS_DISTRIBUTION()

	# Create message
	if RunInForeGround == "TRUE" or vid in RunInForeGroundList:
		Message.RUN_IN_FOREGROUND = True
	else: 
		Message.RUN_IN_FOREGROUND = False

	Message.ID = vid
	Message.START_PORT_NO = str(StartPort + Incrementor)
	Message.MODULE_TYPE = "CAR"
	
	if SystemsIPSubscript % len(SystemsIP) == 0:
		SystemsIPSubscript += 1

	# Send message 
	DistributedSystemsPublisher.send_multipart([SystemsIP[SystemsIPSubscript % len(SystemsIP)], Message.SerializeToString()])
		
	SystemsIPSubscript = SystemsIPSubscript % len(SystemsIP) + 1
	Incrementor += 5
	
	LogsDebug("A Car process " + vid + " is started")
	LogsInfo("A Car process " + vid + " is started")


# Sends message to Command Receiver running on remote machine with ID=-1 indicating one experiment run is completed 
def SendKillSigToTerminals():
	Message = DistributeProcesses_pb2.PROCESS_DISTRIBUTION()
	Message.ID = "-1"

	for each in SystemsIP:
		DistributedSystemsPublisher.send_multipart([each, Message.SerializeToString()])


# Server waits for the client(car process) requests - for position and speed
def MobilityServer():
	
    	global ParserIP, ParserPort, VehInfoHashMap, NOTFinished

    	context = zmq.Context()
    	socket = context.socket(zmq.REP)
    	LogsDebug("Sumo Data Parser Server binds at the address "+ParserIP+":" + ParserPort)
    	socket.bind("tcp://"+ParserIP+":"+ParserPort)
	Request = CARRequestToMT_pb2.CARREQUESTTOMT()
	GPSResponse = MTGPSResponse_pb2.MTGPSRESPONSE()
	SpeedResponse = MTSpeedResponse_pb2.MTSPEEDRESPONSE()

	    	
    	while NOTFinished:
           	#  Wait for next request from client
           	vehInfoReq = socket.recv()
		Request.ParseFromString(vehInfoReq)
	   
	   	# parse the request
	   	VehID = Request.VID
	   	ReqType = Request.REQ

		# Check what is the request type (for position or speed) accordingly build the json reply
	   	if ReqType == "POS":
			LogsDebug("A request for Position is arrived for Vehicle ID " + VehID)
			if VehID in VehInfoHashMap:
				LogsDebug("Vehicle ID " + VehID + " exists and a response with updated Position data will be sent")

				GPSResponse.INFO_EXISTS = 1
				GPSResponse.X = float(json.loads(VehInfoHashMap[VehID])["X"])
				GPSResponse.Y = float(json.loads(VehInfoHashMap[VehID])["Y"])
				GPSResponse.DIR = float(json.loads(VehInfoHashMap[VehID])["DIR"])
				GPSResponse.LANE = json.loads(VehInfoHashMap[VehID])["LANE"]
			else:
				LogsDebug("Vehicle ID " + VehID + " do not exist in the network and response is sent")
				GPSResponse.INFO_EXISTS = 0
			DataToSend = GPSResponse.SerializeToString()

		elif ReqType == "SPE":
			LogsDebug("A request for Speed is arrived for Vehicle ID " + VehID)
			if VehID in VehInfoHashMap:
				LogsDebug("Vehicle ID " + VehID + " exists and a response with updated Speed data will be sent")
				SpeedResponse.INFO_EXISTS = 1
				SpeedResponse.SPEED = float(json.loads(VehInfoHashMap[VehID])["SPE"])
			else:
				LogsDebug("Vehicle ID " + VehID + " do not exist in the network")
				SpeedResponse.INFO_EXISTS = 0
			DataToSend = SpeedResponse.SerializeToString()

		socket.send(DataToSend)
		
		



# Updates the speed and position information in the hashmap
def update_hashmap(vid, x, y, speed, angle, lane):
	global VehInfoHashMap
	jsonVehData = {
		"X" : x,
                "Y" : y,
		"SPE" : speed,
		"DIR" : angle,
		"LANE" : lane
	}
	VehInfoHashMap[vid] = json.dumps(jsonVehData)



Message = MsgFromNodeToUDM_pb2.MSGFROMNODETOUDM()

# udm module: server awaits the message forward requests from its clients (car or ap)
def udm_server():
	global ParserIP, UDMPort, Message
	context = zmq.Context()
    	socket = context.socket(zmq.PULL)
    	LogsDebug("UDM Server binds at the address "+ParserIP+":" + UDMPort)
	LogsInfo("UDM Server binds at the address "+ParserIP+":" + UDMPort)
    	socket.bind("tcp://"+ParserIP+":"+UDMPort)

	while True:
           	#  Wait for next request from client
           	msgForwardReq = socket.recv()
		Message.ParseFromString(msgForwardReq)

		# mtype - message type (INIT or UNICAST or BROADCAST)
		# Only cars send INIT message - once, when the process get started
		try:
			mtype = Message.MTYPE
		except ValueError:
			mtype = ""
        		LogsError("This occur very very rarely and yet to investigate on why this error!! ...")

		if mtype == "INIT":
			LogsInfo("INIT message recieved from " + Message.SRC_ID)
			LogsDebug("INIT message recieved from " + Message.SRC_ID)
			store_ip_port(msgForwardReq)
		elif mtype == "UNI" or mtype == "BROAD" or mtype == "MUL":
			forward_msg(msgForwardReq)
	

# Called when udm recieves INIT messages
# Stores IP and Port details of the Cars
def store_ip_port(msgForwardReq):
	global AllAddrTable, Message
	Message.ParseFromString(msgForwardReq)
	_id = Message.SRC_ID
	newip = Message.IP
	newport = Message.PORT
	_type = Message.SRC_TYPE
	LogsDebug("From the INIT Message Vehicle " + _id + " IP PORT and TYPE are extracted " + newip + " " + newport + " " + _type
		+ " and are stored") 
	new_data = {
		'IP' : newip,
		'PORT' : newport,
		'TYPE' : _type
	}
	AllAddrTable[_id] = json.dumps(new_data)


# Based on the MTYPE(message type) calls different message forwarding modules
def forward_msg(msgForwardReq):
	global VehInfoHashMap, APInfoHashMap, Message
	Message.ParseFromString(msgForwardReq)
	mtype = Message.MTYPE
	src_id = Message.SRC_ID

	# Checks if the source id is either present in updated vehicle hash map or AP hash map
	# If it exists only then forward the messages (else the vehicles not in the network anymore)
	if VehInfoHashMap and APInfoHashMap:
		if src_id in VehInfoHashMap or src_id in APInfoHashMap:
			if mtype == "UNI":
				send_unicast_msg(msgForwardReq)
			elif mtype == "BROAD":
				send_broadcast_msg(msgForwardReq)
			elif mtype == "MUL":
				send_multicast_msg(msgForwardReq)

# Forwards unicast messages 
def send_unicast_msg(msgForwardReq):
	global APInfoHashMap, VehInfoHashMap, AllAddrTable, Message
	Message.ParseFromString(msgForwardReq)
	src_id = Message.SRC_ID
	dest_id = Message.DEST_ID

	LogsDebug("UNICAST message is recieved from " + src_id + " to forawrd it to " + dest_id)
	LogsInfo("UNICAST message is recieved from " + src_id + " to forawrd it to " + dest_id)

	if src_id in VehInfoHashMap:	
		src_details = json.loads(VehInfoHashMap[src_id])
	elif src_id in APInfoHashMap:
		src_details = json.loads(APInfoHashMap[src_id])

	src_X = src_details['X']
	src_Y = src_details['Y']
	
	SrcExistsInAddrTable = True
	try:
		srcType_json = json.loads(AllAddrTable[src_id])
	except KeyError:
		SrcExistsInAddrTable = False
		
	
	# Checks if destination node is still in the network
	if (dest_id in VehInfoHashMap or dest_id in APInfoHashMap) and SrcExistsInAddrTable:
		# Checks if destination address is known to the UDM
		if dest_id in AllAddrTable:
			# If the source type is BS donot check for communication range
			if srcType_json['TYPE'] == "BS":
				PublishMsg(msgForwardReq, dest_id)
				LogsDebug("Received UNICAST message is forwarded to " + dest_id)
			else:
				Dest_json = json.loads(AllAddrTable[dest_id])
				# If dest type is car or RSU check if source and destination are in the communication range
				# else for BS skips the check and forward the message
				if Dest_json['TYPE'] == "CAR":
					car_json = json.loads(VehInfoHashMap[dest_id])
					LogsDebug("range check between " + src_id + " -----------> " + dest_id)
					LogsDebug(src_id+"("+src_X+","+src_Y+")  "+dest_id+"("+ car_json["X"]+ ","+ car_json["Y"]+ ")") 
					if within_range(src_X, src_Y, car_json["X"], car_json["Y"]):
						PublishMsg(msgForwardReq, dest_id)
						LogsDebug("Received UNICAST message is forwarded to " + dest_id)
				elif Dest_json['TYPE'] == "RSU":
					RSU_json = json.loads(APInfoHashMap[dest_id])
					LogsDebug("range check between " + src_id + " -----------> " + dest_id)
					LogsDebug(src_id+"("+src_X+","+src_Y+")  "+dest_id+"("+ RSU_json["X"]+ ","+ RSU_json["Y"]+ ")") 
					if within_range(src_X, src_Y, RSU_json["X"], RSU_json["Y"]):
						PublishMsg(msgForwardReq, dest_id)
						LogsDebug("Received UNICAST message is forwarded to " + dest_id)
				elif Dest_json['TYPE'] == "BS":
					PublishMsg(msgForwardReq, dest_id)
					LogsDebug("Received UNICAST message is forwarded to " + dest_id)
	else:
		LogsError("Unicast message to VID " + dest_id + "failed, since vehicle doesn't exist or Address is still Unknown")

def PublishMsg(msgForwardReq, dest_id):
	global UDMPublisher, Message
	Message.ParseFromString(msgForwardReq)
	src_id = Message.SRC_ID
	dest_json = json.loads(AllAddrTable[dest_id])
	src_json = json.loads(AllAddrTable[src_id])

	DataToForward = MessageForwardFromUDM_pb2.MESSAGEFROMUDM()
	DataToForward.SRC_ID = src_id

	# Copy contents of the incoming message to new message that is forwarded to destination node	
	DataToForward.DATA.DATALINE1 = Message.DATA.DATALINE1
	DataToForward.DATA.DATALINE2 = Message.DATA.DATALINE2
	DataToForward.DATA.ID = Message.DATA.ID
	DataToForward.DATA.X = Message.DATA.X
	DataToForward.DATA.DIR = Message.DATA.DIR
	DataToForward.DATA.SPEED = Message.DATA.SPEED
	DataToForward.DATA.DATATYPE = Message.DATA.DATATYPE 
	DataToForward.DATA.TIMESTAMP = Message.DATA.TIMESTAMP
	DataToForward.DATA.Y = Message.DATA.Y
	DataToForward.DATA.CH = Message.DATA.CH
	DataToForward.DATA.CMLIST = Message.DATA.CMLIST
	DataToForward.DATA.CLUSTERID = Message.DATA.CLUSTERID
	DataToForward.DATA.CLUSTER_SEQ_NUM = Message.DATA.CLUSTER_SEQ_NUM;
	DataToForward.DATA.FILENAME = Message.DATA.FILENAME
	DataToForward.DATA.DATA = Message.DATA.DATA
	DataToForward.DATA.CHUNKNUM = Message.DATA.CHUNKNUM
	DataToForward.DATA.LASTPKT = Message.DATA.LASTPKT
	DataToForward.DATA.START_TIME = Message.DATA.START_TIME
	DataToForward.DATA.TASKSEQNUM = Message.DATA.TASKSEQNUM
	DataToForward.DATA.TASKMAXTIME = Message.DATA.TASKMAXTIME
	DataToForward.DATA.FINISHTIME = Message.DATA.FINISHTIME
	DataToForward.DATA.RESULT = Message.DATA.RESULT


	# Adding additional fields 
	DataToForward.SRC_TYPE = src_json['TYPE']
	DataToForward.DEST_TYPE = dest_json['TYPE']
	DataToForward.EXIT = False
	DataToSend = DataToForward.SerializeToString()

	if "DCA" in Message.SUB_DEST_ID:
		DestID = dest_id + "DCAID"
	elif "TASK" in Message.SUB_DEST_ID:
		DestID = dest_id + "TASKDID"
	elif "DC" in Message.SUB_DEST_ID:
		DestID = dest_id + "DCID"
	else: 
		DestID = dest_id + "ID"

	UDMPublisher.send_multipart([str(DestID), DataToSend])



# Forwards broadcast messages 
def send_broadcast_msg(msgForwardReq):
	global APInfoHashMap, VehInfoHashMap, AllAddrTable, Message
	Message.ParseFromString(msgForwardReq)
	src_id = Message.SRC_ID

	LogsDebug("Broadcast message is recieved from " + src_id)
	LogsInfo("Broadcast message is recieved from "+ src_id)

	if src_id in VehInfoHashMap:	
		src_details = json.loads(VehInfoHashMap[src_id])
	elif src_id in APInfoHashMap:
		src_details = json.loads(APInfoHashMap[src_id])
	src_X = src_details['X']
	src_Y = src_details['Y']
	
	SrcExistsInAddrTable = True
	try:
		srcType_json = json.loads(AllAddrTable[src_id])
	except KeyError:
		SrcExistsInAddrTable = False

	# Check all the cars in the address table if they are within the communication range
	CopyOfAllAddrTable = dict(AllAddrTable)
	for _id in CopyOfAllAddrTable:
		# Check if _id is exists in the network and
		# and the _id should not be the source id (to avoid broadcast back to the source)
		if (_id in VehInfoHashMap or _id in APInfoHashMap) and _id != src_id and SrcExistsInAddrTable:
			# If the source type is BS donot check for communication range
			if srcType_json['TYPE'] == "BS":
				PublishMsg(msgForwardReq, _id)
				LogsDebug("Received BROADCAST message is forwarded to" + _id)
			# If dest type is car or RSU check if source and destination are in the communication range
			# else if it is a BS skips the check and forward the message
			else:
				broad_json = json.loads(CopyOfAllAddrTable[_id])
				if broad_json['TYPE'] == 'CAR':
					car_json = json.loads(VehInfoHashMap[_id])
					LogsDebug("range check between "+src_id+" -----------> "+_id) 
					if within_range(src_X, src_Y, car_json["X"], car_json["Y"]):
						PublishMsg(msgForwardReq, _id)
						LogsDebug("Received BROADCAST message is forwarded to" + _id)
				elif broad_json['TYPE'] == "RSU":
					RSU_json = json.loads(APInfoHashMap[_id])
					LogsDebug("range check between "+src_id+" -----------> "+_id) 
					if within_range(src_X, src_Y, RSU_json["X"], RSU_json["Y"]):
						PublishMsg(msgForwardReq, _id)
						LogsDebug("Received BROADCAST message is forwarded to" + _id)
				elif broad_json['TYPE'] == "BS":
					PublishMsg(msgForwardReq, _id)
					LogsDebug("Received BROADCAST message is forwarded to" + _id)

# Detects if both are in the communication range(Do not consider obstacles)
def within_range(x1, y1, x2, y2):
	global CommRange
	distance = math.sqrt( (float(x1) - float(x2)) * (float(x1) - float(x2)) + (float(y1) - float(y2)) * (float(y1) - float(y2)) )
	LogsDebug("Comparison " + str(distance) +" < " + CommRange + " ??")
	if float(distance) < float(CommRange):
		return True
	else:
		return False

VehiclesExitList = []
import random

# When vehicle exists network this method sends exit message to the vehicle
def DeleteFromNetwork(each):
	# Sleeps for random time (because at last timestep all cars exits at once) to avoid race to acquire lock
	time.sleep(float(random.randint(1,100))/100.0)
	DataToForward = MessageForwardFromUDM_pb2.MESSAGEFROMUDM()
	# Indicates vehicles to exit
        DataToForward.EXIT = True
        DataToSend = DataToForward.SerializeToString()
	print(each)
	lock.acquire()
	# Send all modules of cars an exit message
	UDMExitPublisher.send_multipart([str(each) + "DCAEXITID", DataToSend])
	UDMExitPublisher.send_multipart([str(each) + "TASKDEXITID", DataToSend])
	UDMExitPublisher.send_multipart([str(each) + "EXITID", DataToSend])
	UDMExitPublisher.send_multipart([str(each) + "EXITGPSID", DataToSend])
	UDMExitPublisher.send_multipart([str(each) + "EXITSPEEDID", DataToSend])
	lock.release()
	# Allows cars to terminate before Helper removes car from the existing list
	time.sleep(12.0)
	del VehInfoHashMap[each]

# Deletes the car data when the car no longer exists in the network
def cleanup_VehInfoHashMap(VIDList):
	global VehInfoHashMap, Keys, VehiclesExitList, UDMExitPublisher
	Keys = VehInfoHashMap.keys()
	for each in Keys:
		if each not in VIDList and each not in VehiclesExitList:
			VehiclesExitList.append(each)
			# Creates a thread for each exited car to send a exit message to vehicle
			thread.start_new_thread(DeleteFromNetwork,(each,))


# Subscriber always misses the first message. FirstMsg is used to send first message twice
FirstMsg = True
i = 0
_list = []
_dict = {}

# First proc called in the main()
def run():
	start = 0.0
	end = 0.0
	FirstTime = 1
	WaitPeriod = 0.0
	WaitDue = 0.0
	
	# Creates a UDM server thread
	thread.start_new_thread(udm_server, ())
	# Creates server that responds to speed and position requests from the vehicles
    	thread.start_new_thread(MobilityServer, ())

    	global SumoFloatingDataPath, VehInfoHashMap, APInfoHashMap, NOTFinished
	car_proc_list = []

	# Read AP info from XML file and spawns AP process in appropriate machine
	update_APInfoHashMap()

	# iterates through the floating car data timestamps(interval of 0.1 seconds)
	#for iteration in root.iter('timestep'):
	for d in xmliter(SumoFloatingDataPath, 'timestep'):
                if float(d['@time']) % 20.0 == 0:
                        print("Current timestep = " , d['@time'])
                # VIDList at every timestamp iteration is set to empty. Global parameter stores existing vehicles details
                VIDList = []
		
		# If at certain timestep there is no vehicle exist, it raises exception
		try:
			type(d['vehicle'])
		except:
			time.sleep(0.1)
			continue
			
		
                if type(d['vehicle']) == type(_dict):
                        vid = d['vehicle']['@id']
                        if vid not in VIDList:
                                # If its a new vehicle add it to the list
                                VIDList.append(vid)
                        if vid not in car_proc_list:
				# If its a new vehicle spwan car processes
                                car_proc_list.append(vid)
                                start_carProc(vid)
                        update_hashmap(vid, d['vehicle']['@x'], d['vehicle']['@y'], d['vehicle']['@speed'], d['vehicle']['@angle'],d['vehicle']['@lane'])

                elif type(d['vehicle']) == type(_list):
                        for each in d['vehicle']:
                                vid = each['@id']
                                if vid not in VIDList:
                                # If its a new vehicle add it to the list
                                        VIDList.append(vid)
                                if vid not in car_proc_list:
					# If its a new vehicle spwan car processes
                                        car_proc_list.append(vid)
                                        start_carProc(vid)
                                update_hashmap(vid, each['@x'], each['@y'], each['@speed'], each['@angle'],each['@lane'])

                # After each time stamp remove vehicles from the hashmap which doesnt exist anymore in the network
                cleanup_VehInfoHashMap(VIDList)

		#LogsDebug("Currently the following vehicles exist in the network")
                #for key, value in VehInfoHashMap.iteritems():
                        #LogsDebug("Vehicle ID " + key)

		# Print timestep once in 20 seconds
                if float(d['@time']) % 20.0 == 0:
                        print(0.1 - ((end - WaitPeriod) - start))
                end = time.time() 						# Record End time
                if FirstTime == 1:
                        end = 0.0
                        FirstTime = 0
                WaitPeriod = 0.1 - ((end - WaitPeriod) - start) - WaitDue	# Calculate waitperiod
                start = time.time()						# Calculate Start time
		try:
			# If time.sleep is passed a negative argument it raises exception
                	time.sleep(WaitPeriod + 0.03)
			WaitDue = 0.0
                except:
			print("exception raised",WaitPeriod)
			# If at certail step Waitperiod becomes negative - it takes the negative time due to next step
			# If Waitperiod becomes negative at every timestep that starting inducing delay endless
			WaitDue = -1 * WaitPeriod
			WaitPeriod = 0.0
			time.sleep(WaitPeriod)
		

	print("Finished timesteps")	
	VIDList = []
	# Send termination message to all existing vehicles
	cleanup_VehInfoHashMap(VIDList)
        print("Sent Termination message to all active cars")
	time.sleep(18.0)
	Keys = APInfoHashMap.keys()
	DataToSend = ""
	# Send termination message to all existin APs
	for each in Keys:
		print(each)
		DataToForward = MessageForwardFromUDM_pb2.MESSAGEFROMUDM()
                DataToForward.EXIT = True
                DataToSend = DataToForward.SerializeToString()
		lock.acquire()
		UDMPublisher.send_multipart([str(each) + "ID", DataToSend])
		UDMPublisher.send_multipart([str(each) + "DCID", DataToSend])
		lock.release()

	# Indicating in-car sensors that timesteps finished
	NOTFinished = False
	time.sleep(5.0)
	print("Sent Termination message to all active APs")
	LogsInfo("FINISHED")
	LogsDebug("FINISHED")
	# sleep allows all the vehicles and APs to write thier stats
	time.sleep(60)
	# Send termination messages to remote machine terminal indicating completed one experiment run
	SendKillSigToTerminals()
    	sys.stdout.flush()

# Parse config file to extract details of base stations and rsu's and store them in 'APInfoHashMap'
# 'APInfoHashMap' stores id versus (ip, port and type(rsu or bs)) 

def update_APInfoHashMap():
	global APInfoHashMap, RunInForeGround, RunInForeGroundList, DistributedSystemsPublisher, FirstMsg
	LogsDebug("Parsing config file ../config/ap/config.xml")
	tree = ET.parse('../config/ap/config.xml')
	root = tree.getroot()
	Message = DistributeProcesses_pb2.PROCESS_DISTRIBUTION()
	for iteration in root.iter('config'):
		for APDetails in iteration.iter('BS'):
			BS_data = {
				'X' : APDetails.get('x'),
				'Y' : APDetails.get('y')
			}
			APInfoHashMap[APDetails.get('id')] = json.dumps(BS_data)
			BS_IP_data = {
				'IP' : APDetails.get('ip'),
				'PORT' : APDetails.get('port'),
				'TYPE' : "BS"
			}
			AllAddrTable[APDetails.get('id')] = json.dumps(BS_IP_data)
			LogsDebug("BASESTATION module " + APDetails.get('id') + " is started in a separate terminal")
			LogsInfo("BASESTATION module " + APDetails.get('id') + " is started in a separate terminal")
			if RunInForeGround == "TRUE" or APDetails.get('id') in RunInForeGroundList:
				Message.RUN_IN_FOREGROUND = True
			else:
				Message.RUN_IN_FOREGROUND = False
			Message.ID = APDetails.get('id')
			Message.MODULE_TYPE = "BS"
			if FirstMsg:
				DistributedSystemsPublisher.send_multipart([SystemsIP[0], Message.SerializeToString()])
				FirstMsg = False
			time.sleep(2)
			DistributedSystemsPublisher.send_multipart([SystemsIP[0], Message.SerializeToString()])
		
			
		
		for APDetails in iteration.iter('RSU'):
			RSU_data = {
				'X' : APDetails.get('x'),
				'Y' : APDetails.get('y')
			}
			APInfoHashMap[APDetails.get('id')] = json.dumps(RSU_data)
			RSU_IP_data = {
				'IP' : APDetails.get('ip'),
				'PORT' : APDetails.get('port'),
				'TYPE' : "RSU"
			}
			AllAddrTable[APDetails.get('id')] = json.dumps(RSU_IP_data)
			LogsDebug("ROADSIDE module " + APDetails.get('id') + " is started")
			LogsInfo("ROADSIDE module " + APDetails.get('id') + " is started")

			if RunInForeGround == "TRUE" or APDetails.get('id') in RunInForeGroundList:
				Message.RUN_IN_FOREGROUND = True
			else:
				Message.RUN_IN_FOREGROUND = False
			Message.ID = APDetails.get('id')
			Message.MODULE_TYPE = "RSU"
			if FirstMsg:
				DistributedSystemsPublisher.send_multipart([SystemsIP[0], Message.SerializeToString()])
				FirstMsg = False
			time.sleep(2)
			DistributedSystemsPublisher.send_multipart([SystemsIP[0], Message.SerializeToString()])
	
	LogsDebug("Parsed details from ../config/ap/config.xml and stored. Details::")
	for key, value in AllAddrTable.iteritems():
		LogsDebug(str(key) + " " + str(json.loads(value)))

def updateLoggingUtilConfigParams():
	global LogInfoEnable, LogInfoFile, LogInfoStdOutput, LogDebugEnable, LogDebugFile, LogDebugStdOutput, LogStatsEnable, LogStatsFILE, LogStatsStdOutput, LogErrorEnable, LogErrorFILE, LogErrorStdOutput, LogFilePath, ExperimentNumber, LogFile
	tree = ET.parse('../config/logging_utility/config.xml')
	root = tree.getroot()
	for iteration in root.iter('config'):
		for LogDetails in iteration.iter('Tool'):
			name = LogDetails.get("name")
			if name == "LogInfo":
				LogInfoEnable = LogDetails.get('LogInfoEnable')
				LogInfoFile = LogDetails.get('LogInfoFile')
				LogInfoStdOutput = LogDetails.get('LogInfoStdOutput')
			elif name == "LogDebug":
				LogDebugEnable = LogDetails.get('LogDebugEnable')
				LogDebugFile = LogDetails.get('LogDebugFile')
				LogDebugStdOutput = LogDetails.get('LogDebugStdOutput')
			elif name == "LogStats":
				LogStatsEnable = LogDetails.get('LogStatsEnable')
				LogStatsFILE = LogDetails.get('LogStatsFILE')
				LogStatsStdOutput = LogDetails.get('LogStatsStdOutput')
			elif name == "LogError":
				LogErrorEnable = LogDetails.get('LogErrorEnable')
				LogErrorFILE = LogDetails.get('LogErrorFILE')
				LogErrorStdOutput = LogDetails.get('LogErrorStdOutput')
			elif name == "path":
				LogFilePath = LogDetails.get('LogFilePath')
			elif name == "ExperimentNum":
				ExperimentNumber = LogDetails.get('ExperimentNumber')
	print(LogInfoEnable, LogInfoFile, LogInfoStdOutput, LogDebugEnable, LogDebugFile, LogDebugStdOutput, LogStatsEnable, LogStatsFILE, LogStatsStdOutput, LogErrorEnable, LogErrorFILE, LogErrorStdOutput, LogFilePath, ExperimentNumber)
	filename = "../results/logs/" + "["+ExperimentNumber+"][MOBILITYANDUDM].txt"	
	LogFile = open(filename, "a")	

# Creates a socket used by UDM to publish messages
def InitiatePubSocket():
	global UDMPublisher
	context = zmq.Context()
    	UDMPublisher = context.socket(zmq.PUB)
    	UDMPublisher.bind("tcp://"+str(ParserIP)+":"+str(int(UDMPort) + 1))	

# Creates a socket used by UDM to send exit or termination messages
def InitiateExitPubSocket():
	global UDMExitPublisher
	context = zmq.Context()
    	UDMExitPublisher = context.socket(zmq.PUB)
    	UDMExitPublisher.bind("tcp://"+str(ParserIP)+":"+str(int(UDMPort) + 3))	

# Parse config file to read parser and udm related details
def ReadConfigfile():
	global ParserIP, ParserPort, UDMPort, SumoFloatingDataPath, CommRange, RunInForeGround, RunInForeGroundList, SystemsIP
	print("Parsing config file ../config/common/config.xml")
	tree = ET.parse('../config/common/config.xml')
	root = tree.getroot()
	for neighbor in root.iter('ParserIP'):
		ParserIP = neighbor.get('ParserIP')
	for neighbor in root.iter('ParserPort'):
		ParserPort = neighbor.get('ParserPort')
	for neighbor in root.iter('UDMPort'):
		UDMPort = neighbor.get('UDMPort')
	for neighbor in root.iter('SumoFloatingDataPath'):
		SumoFloatingDataPath = neighbor.get('SumoFloatingDataPath')
	for neighbor in root.iter('CommRange'):
		CommRange = neighbor.get('CommRange')
	for neighbor in root.iter('RunInForeGround'):
		RunInForeGround = neighbor.get('RunInForeGround')
	for neighbor in root.iter('RunInForeGroundList'):
		RunInForeGroundList = neighbor.get('RunInForeGroundList')
	for neighbor in root.iter('IPList'):
		SystemsIP = ConvertStringToList(neighbor.get('IPList'))
	print("Extracted values ParserIP = " + ParserIP + " ParserPort = " + ParserPort + " UDMPort = " + UDMPort 
			+ " SumoFloatingDataPath = " + SumoFloatingDataPath + " CommRange = " + CommRange + "RunInForeGround = " + 
			RunInForeGround + "RunInForeGroundList = " + RunInForeGroundList)
	InitiatePubSocket()
	InitiateExitPubSocket()
	PrepareSystemResources()
	updateLoggingUtilConfigParams()

def GetCurrentTime():
	currentDT = datetime.datetime.now()
	CurrentTime = str(currentDT.hour) + ":" + str(currentDT.minute) + ":" + str(currentDT.second) + "." + str(currentDT.microsecond / 1000) 
	return CurrentTime

def GetLogString(message):
	LogString = "[" + GetCurrentTime() + "]["+ExperimentNumber+"][MOBILITYANDUDM][" + message + "]\n"
	return LogString

def LogsFilewrite(message):
	global LogFile
	LogFile.write(GetLogString(message))

def LogsInfo(message):
	global LogInfoEnable, LogInfoFile, LogInfoStdOutput
	if LogInfoEnable == "TRUE":
		if LogInfoFile == "TRUE":
			LogsFilewrite(message)
		if LogInfoStdOutput == "TRUE":
			print(GetLogString(message))

def LogsDebug(message):
	global LogDebugEnable, LogDebugFile, LogDebugStdOutput
	if LogDebugEnable == "TRUE":
		if LogDebugFile == "TRUE":
			LogsFilewrite(message)
		if LogDebugStdOutput == "TRUE":
			print(GetLogString(message))

def LogsStats(message):
	global LogStatsEnable, LogStatsFILE, LogStatsStdOutput
	if LogStatsEnable == "TRUE":
		if LogStatsFILE == "TRUE":
			LogsFilewrite(message)
		if LogStatsStdOutput == "TRUE":
			print(GetLogString(message))

def LogsError(message):
	global LogErrorEnable, LogErrorFILE, LogErrorStdOutput
	if LogErrorEnable == "TRUE":
		if LogErrorFILE == "TRUE":
			LogsFilewrite(message)
		if LogErrorStdOutput == "TRUE":
			print(GetLogString(message))

def PrepareSystemResources():
	global DistributedSystemsPublisher
	context = zmq.Context()
    	DistributedSystemsPublisher = context.socket(zmq.PUB)
    	DistributedSystemsPublisher.bind("tcp://"+str(ParserIP)+":"+str(int(UDMPort) + 2))	


# this is the main entry point of this script
if __name__ == "__main__":
	print("here1")
	global LogString, SystemsIP

	#Uncomment when debugging is required
	#sys.settrace(traceit)
    	ReadConfigfile()
	print(SystemsIP)
    	run()
