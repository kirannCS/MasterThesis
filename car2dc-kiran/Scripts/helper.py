#!/usr/bin/env python

#######################################################################################
########### Used to spawn different car modules #######################################
#######################################################################################


from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import subprocess

os.system("../bin/CarCluster "+ sys.argv[1] + " " + sys.argv[2] + " " + sys.argv[3] + " &")
os.system("../bin/CarSpeed "+ sys.argv[1] + " " + sys.argv[2] + " " + sys.argv[3] + " &")
os.system("../bin/CarGPS "+ sys.argv[1] + " " + sys.argv[2] + " " + sys.argv[3] + " &")
os.system("../bin/DCA "+ sys.argv[1] + " " + sys.argv[2] + " " + sys.argv[3] + " &")
os.system("../bin/TaskD "+ sys.argv[1] + " " + sys.argv[2] + " " + sys.argv[3] + " &")

#subprocess.call("./TaskD "+ sys.argv[1] + " " + sys.argv[2] + " " + sys.argv[3] + " &", shell=True)
