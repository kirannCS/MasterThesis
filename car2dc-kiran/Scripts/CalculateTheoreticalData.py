

#################################################################################
################# Calculates expected amount of data collected at AP ############
#################################################################################

import csv
import sys

if len(sys.argv) != 4:
	print("Usage: CalculateTheoreticalData.py 'Output file' 'DataSize(kB)' 'Cluster Interval'")
	exit()
 

# Input file containing cluster size at each interval
Filename = "../results/logs/NoOfVehiclesInCluster.csv"
ClusterInterval = int(sys.argv[3])
with open(Filename) as Datafile:
	csv_data=csv.reader(Datafile)
    	my_dict = {rows[0]:rows[1] for rows in csv_data}
	
del my_dict["StepCount"]
 
CountValues = []
IterValues = []
NewIterValues = []

for key,value in my_dict.iteritems():
	IterValues.append(int(key))

keys = sorted(IterValues)
for each in keys:
	NewIterValues.append(each * ClusterInterval)
	CountValues.append(int(my_dict[str(each)]))

TotalData = 0
# Data size used
DataSize = int(sys.argv[2]) * 1024
# CM to CH data transfer interval
DataInterval = 2
for each in CountValues:
	TotalData += each * DataSize * (ClusterInterval/DataInterval)

# Append data to output file
f=open(sys.argv[1], "a+")
f.write(str(TotalData) + "\n")
f.close()



