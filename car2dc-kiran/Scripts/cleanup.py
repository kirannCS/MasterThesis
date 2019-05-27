#!/usr/bin/env python

########################################################################################################
##### After each experiment it cleansup previous unnecessary data and if any unkilled processes ########
##### Takes 2 optional arguments "logs experiment num" "<0|1> 1-clear screen 0-donot clear screen" #####
########################################################################################################

from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import zmq

ArgsCount = len(sys.argv)

def KillProcesses():
	os.system("killall BS")
	os.system("killall CarCluster")
	os.system("killall CarSpeed")
	os.system("killall CarGPS")
	os.system("killall RSU")
	os.system("killall DCA")
	os.system("killall APDC")
	os.system("killall TaskD")
	

if ArgsCount == 1:
	KillProcesses()
	print("Deleted background processes")
	
elif ArgsCount == 2:
	KillProcesses()
	KillString = "rm ../results/logs/\["+sys.argv[1]+"\]*"
	os.system(KillString)
	os.system("rm ../results/data_files/*")
	os.system("rm ../results/ch_received_files/*")
	os.system("rm ../results/ch_generated_files/*")
	os.system("rm ../results/ap_received_files/*")
	print("Deleted background processes and logs with experiment number " + sys.argv[1])

elif ArgsCount == 3:
	KillProcesses()
	os.system("rm ../results/logs/\["+sys.argv[1]+"\]*")
	os.system("rm ../results/data_files/*")
	os.system("rm ../results/ch_received_files/*")
	os.system("rm ../results/ch_generated_files/*")
	os.system("rm ../results/ap_received_files/*")
	os.system("clear")
	print("Deleted background processes and logs with experiment number " + sys.argv[1])

