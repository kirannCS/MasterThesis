#!/usr/bin/env python3

#######################################################################################
########### Generates cpp and python proto modules ####################################
#######################################################################################

from __future__ import absolute_import
from __future__ import print_function

import os
import sys

SourceDirectory = "./packets"
DestinationDircetory = "./packets/header/"

os.system("protoc -I=" + SourceDirectory + " --cpp_out=" + DestinationDircetory + " " + SourceDirectory + "/*.proto")
os.system("protoc -I=" + SourceDirectory + " --python_out=" + DestinationDircetory + " " + SourceDirectory + "/BigData.proto")
os.system("protoc -I=" + SourceDirectory + " --python_out=" + DestinationDircetory + " " + SourceDirectory + "/MTGPSResponse.proto")
os.system("protoc -I=" + SourceDirectory + " --python_out=" + DestinationDircetory + " " + SourceDirectory + "/CARRequestToMT.proto")
os.system("protoc -I=" + SourceDirectory + " --python_out=" + DestinationDircetory + " " + SourceDirectory + "/MTSpeedResponse.proto")
os.system("protoc -I=" + SourceDirectory + " --python_out=" + DestinationDircetory + " " + SourceDirectory + "/MsgFromNodeToUDM.proto")
os.system("protoc -I=" + SourceDirectory + " --python_out=" + DestinationDircetory + " " + SourceDirectory + "/MessageForwardFromUDM.proto")
os.system("protoc -I=" + SourceDirectory + " --python_out=" + DestinationDircetory + " " + SourceDirectory + "/DistributeProcesses.proto")
os.system("protoc -I=" + SourceDirectory + " --python_out=" + DestinationDircetory + " " + SourceDirectory + "/SampleData.proto")
