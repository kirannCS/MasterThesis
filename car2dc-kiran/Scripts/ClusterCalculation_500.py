 #!/usr/bin/env python3
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import csv
import time
import xml.etree.ElementTree as ET
import math 


# Global Vaiables
ParserIP = ""
ParserPort = ""
UDMPort = ""
SumoFloatingDataPath = ""
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

from xmlr import xmliter

i = 0
_list = []
_dict = {}


# Returns euclid distance between two coordinates
def DistanceCalc(x1, y1, x2, y2):
	return math.sqrt((float(x1) - float(x2)) * (float(x1) - float(x2)) + (float(y1) - float(y2)) * (float(y1) - float(y2)))
				

# First proc called in the main()
def run():

    	global SumoFloatingDataPath, VehInfoHashMap, CommRange
	car_proc_list = []

	PrintTime = 0.0
	DetectCarInMoreThanOneCluster = 0
	FirstTimePrintMC1 = False
	FirstTimePrintMC2 = False
	FirstTimePrintMC3 = False

	### Output files
	CSVFile1 = "../results/logs/ClusterCalculation1.csv";
	CSVFile2 = "../results/logs/ClusterCalculation2.csv";
	CSVFile3 = "../results/logs/ClusterCalculation3.csv";
	with open(CSVFile1, mode='w') as data_file:
		data_writer = csv.writer(data_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
		data_writer.writerow(["StepCount", "Number Of Cars"])
	with open(CSVFile2, mode='w') as data_file:
		data_writer = csv.writer(data_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
		data_writer.writerow(["StepCount", "Number Of Cars"])
	with open(CSVFile3, mode='w') as data_file:
		data_writer = csv.writer(data_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
		data_writer.writerow(["StepCount", "Number Of Cars"])
	iterationCount = 1;
	
	

	# iterates through the floating car data timestamps(interval of 0.1 seconds)
	for d in xmliter(SumoFloatingDataPath, 'timestep'):
		TotaLCarsInCluster1 = 0;
		TotaLCarsInCluster2 = 0;
		TotaLCarsInCluster3 = 0;

		if float(d['@time']) == PrintTime:
			PrintTime = PrintTime + 1.0
			DetectCarInMoreThanOneCluster = 0
			CHPrevDist1 = 99999999.0
			CHPrevDist2 = 99999999.0
			CHPrevDist3 = 99999999.0
			CH1 = ""
			CH2 = ""
			CH3 = ""
			CMList1 = ""
			CMList2 = ""
			CMList3 = ""
			Distance1 = 99999999.0
			Distance2 = 99999999.0
			Distance3 = 99999999.0

			if type(d['vehicle']) == type(_dict):
				vid = d['vehicle']['@id']				
			
				# Provide micro cloud(s) position coordinates here
				Distance1 = DistanceCalc(float(d['vehicle']['@x']), float(d['vehicle']['@y']), 768.0, 1069.0)
				Distance2 = DistanceCalc(float(d['vehicle']['@x']), float(d['vehicle']['@y']), 321.0, 468.0)
				Distance3 = DistanceCalc(float(d['vehicle']['@x']), float(d['vehicle']['@y']), 918.0, 468.0)

				if Distance1 <= float(CommRange)/2.0:
					TotaLCarsInCluster1 += 1			# Cluster members count
					if Distance1 < CHPrevDist1:
						CH1 = vid				# Identifying CH
						CHPrevDist1 = Distance1
					else:
						CMList1 = CMList1 + " " + vid + " "
					DetectCarInMoreThanOneCluster += 1
				

				
				if Distance2 <= float(CommRange)/2.0:
					TotaLCarsInCluster2 += 1
					if Distance2 < CHPrevDist2:
						CH2 = vid
						CHPrevDist2 = Distance2
					else:
						CMList2 = CMList2 + " " + vid + " "
					DetectCarInMoreThanOneCluster += 1

				if Distance3 <= float(CommRange)/2.0:
                                        TotaLCarsInCluster3 += 1
                                        if Distance3 < CHPrevDist3:
                                                CH3 = vid
                                                CHPrevDist3 = Distance3
                                        else:
                                                CMList3 = CMList3 + " " + vid + " "
                                        DetectCarInMoreThanOneCluster += 1
			
			elif type(d['vehicle']) == type(_list):
                        	for each in d['vehicle']:
                                	vid = each['@id']
					Distance1 = DistanceCalc(float(each['@x']), float(each['@y']), 768.0, 1069.0)
	                                Distance2 = DistanceCalc(float(each['@x']), float(each['@y']), 321.0, 468.0)
	                                Distance3 = DistanceCalc(float(each['@x']), float(each['@y']), 918.0, 468.0)
                	                if Distance1 <= float(CommRange)/2.0:
                        	                print("Distance = ", Distance1)
                                	        TotaLCarsInCluster1 += 1
                                        	if Distance1 < CHPrevDist1:
                                                	CH1 = vid
         	                                       	CHPrevDist1 = Distance1
                	                        else:
                        	                        CMList1 = CMList1 + " " + vid + " "
                                	        DetectCarInMoreThanOneCluster += 1
                                

                                
	                                if Distance2 <= float(CommRange)/2.0:
        	                                TotaLCarsInCluster2 += 1
                	                        if Distance2 < CHPrevDist2:
                        	                        CH2 = vid
                                	                CHPrevDist2 = Distance2
                                        	else:
                                                	CMList2 = CMList2 + " " + vid + " "
	                                        DetectCarInMoreThanOneCluster += 1

					if Distance3 <= float(CommRange)/2.0:
                                        	TotaLCarsInCluster3 += 1
                                        	if Distance3 < CHPrevDist3:
                                                	CH3 = vid
                                                	CHPrevDist3 = Distance3
                                        	else:
                                                	CMList3 = CMList3 + " " + vid + " "
                                        	DetectCarInMoreThanOneCluster += 1
			
			if CH1 != "":
				LogsInfo("The number of cars in MC1 = " + str(TotaLCarsInCluster1))
				LogsInfo("MC1 with CH " + CH1 + " has following Cluster Members" + CMList1)
				print("MC1 with CH ", CH1, " has following Cluster Members", CMList1)
			if CH2 != "":
				LogsInfo("The number of cars in MC2 = " + str(TotaLCarsInCluster2))
				LogsInfo("MC2 with CH " + CH2 + " has following Cluster Members" + CMList2)
				print("MC2 with CH ", CH2, " has following Cluster Members", CMList2)
			if CH3 != "":
				LogsInfo("The number of cars in MC3 = " + str(TotaLCarsInCluster3))
				LogsInfo("MC3 with CH " + CH3 + " has following Cluster Members" + CMList3)
				print("MC3 with CH ", CH3, " has following Cluster Members", CMList3)

			#### Writes cluster sizes into output files			
			with open(CSVFile1, mode='a') as data_file:
				data_writer = csv.writer(data_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
				data_writer.writerow([str(iterationCount), str(TotaLCarsInCluster1)])
			with open(CSVFile2, mode='a') as data_file:
				data_writer = csv.writer(data_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
				data_writer.writerow([str(iterationCount), str(TotaLCarsInCluster2)])
			with open(CSVFile3, mode='a') as data_file:
				data_writer = csv.writer(data_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
				data_writer.writerow([str(iterationCount), str(TotaLCarsInCluster3)])
			iterationCount += 1
						
	LogsInfo("FINISHED")
	LogsDebug("FINISHED")
	time.sleep(1.0)
    	sys.stdout.flush()


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
	filename = "../results/logs/" + "["+ExperimentNumber+"][ClusterCalculation].txt"	
	LogFile = open(filename, "a")	


# Parse config file to read parser and udm related details
def ReadConfigfile():
	global ParserIP, ParserPort, UDMPort, SumoFloatingDataPath, CommRange, RunInForeGround, RunInForeGroundList
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
	print("Extracted values ParserIP = " + ParserIP + " ParserPort = " + ParserPort + " UDMPort = " + UDMPort 
			+ " SumoFloatingDataPath = " + SumoFloatingDataPath + " CommRange = " + CommRange + "RunInForeGround = " + 
			RunInForeGround + "RunInForeGroundList = " + RunInForeGroundList)
	#InitiatePubSocket()
	#update_APInfoHashMap()
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


# this is the main entry point of this script
if __name__ == "__main__":
	global LogString;
	#sys.settrace(traceit)
    	ReadConfigfile()
    	run()
