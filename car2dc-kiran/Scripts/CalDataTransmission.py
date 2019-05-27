

#######################################################################################
########### Used to calculate data collected at AP in an experiment  ##################
########### Takes 2 arguments "Output_flename" "<'DCA'|'TTask' DCA-stats data collected Task-stats tasks completed
#######################################################################################


import os
import re
import sys

TotalDataSentByCH = 0
TotalDataReceivedByAP = 0
TotalDataIGenerated = 0
TotalDataReceived = 0
TotalDataSentToCH = 0
ReassembledAtAP = 0
TotalTasksPerformed = 0

def add(numbers, addTo):
	for each in numbers:
		addTo += int(each)
	return addTo

LogsDir = "../results/logs/"
for root, dirs, files in os.walk(LogsDir):
    for filename in files:
	if "[CAR]" in filename:
		#print(filename)
		# Open each log fle created by different car nodes and search for specific lines that stats data count
		with open(LogsDir + filename) as f:
    			lines = f.readlines()
			# Counts total data sent to AP
			for each in reversed(lines):
				if "Total Data Sent to AP" in each:
					numbers = re.findall(' \d+',each)
					TotalDataSentByCH = add(numbers, TotalDataSentByCH)
					break;
			# Counts total data CH has generated
			for each in reversed(lines):
				if "Total Data I generated =" in each:
					numbers = re.findall(' \d+',each)
					TotalDataIGenerated = add(numbers, TotalDataIGenerated)
					break;
			# Counts total data CH has received from CM
			for each in reversed(lines):
				if "Total Data Received from CM =" in each:
					numbers = re.findall(' \d+',each)
					TotalDataReceived = add(numbers, TotalDataReceived)
					break;
			# Counts total all CMs has sent to CH
			for each in reversed(lines):
				if "Total Data Sent to CH =" in each:
					numbers = re.findall(' \d+',each)
					TotalDataSentToCH = add(numbers, TotalDataSentToCH)
					break;
			# Counts total tasks assigned to CMs
			for each in reversed(lines):
                                if "Total Tasks Assigned to CM " in each:
					print(" I am here")
                                        numbers = re.findall(' \d+',each)
                                        TotalTasksPerformed = add(numbers, TotalTasksPerformed)
                                        break;
								

	# This counts data received by AP
	elif "[BASE" in filename or "[ROAD" in filename:
		with open(LogsDir + filename) as f:
    			lines = f.readlines()
			for each in reversed(lines):
				#print(each)
				if "Total Data Received" in each:
					numbers = re.findall(' \d+',each)
					TotalDataReceivedByAP = add(numbers, TotalDataReceivedByAP)
					#print(filename, "AP Received",int(numbers[0]))
					break;
			for each in reversed(lines):
				#print(each)
				if "re assembled file size =" in each:
					numbers = re.findall(' \d+',each)
					ReassembledAtAP = add(numbers, ReassembledAtAP)
					#print(filename, "Reassembled At AP",int(numbers[0]))
					break;

#print("Total Data Sent by all CH = ", TotalDataSentByCH)
print("Total Data Received by all AP = ", TotalDataReceivedByAP)
print("Total tasks performed by all cluster heads = ", TotalTasksPerformed)
#print("Total Data all CH Generated = ", TotalDataIGenerated)
#print("Total Data Received from all CM = ", TotalDataReceived)
#print("Total Data sent to CH = ", TotalDataSentToCH)
#print("Total Data Received from all CM + Generated = ", TotalDataIGenerated + TotalDataReceived)
#print("Re assembled at AP = ", ReassembledAtAP)


f=open(sys.argv[1], "a+")
if sys.argv[2] == "DCA":
	f.write(str(TotalDataReceivedByAP) + "\n")
elif sys.argv[2] == "Task":
	f.write(str(TotalTasksPerformed) + "\n")
f.close()
