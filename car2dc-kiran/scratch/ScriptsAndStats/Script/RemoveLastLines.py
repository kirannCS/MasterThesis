import csv
import sys

####### Takes one argument filename #################################################################### 
##### This file removes last five lines of cluster formation csv file ##################################
##### This is because after all time steps AP gets its termination message after 20 to 25 seconds ######
##### That accounts to 4 to 5 cluster intervals which contains either 0 or false values ###############

my_dict = {}
Count = 0
i = 0
DataValues = []
with open(sys.argv[1], mode='r+w') as csv_file:
	csv_data=csv.reader(csv_file)
	for row in csv_data:
		Count += 1
		#print(row)
with open(sys.argv[1], mode='r+w') as csv_file:
	csv_data=csv.reader(csv_file)
	for row in csv_data:
		print(row)
		if i < Count - 5:
			DataValues.append(row)
		i = i + 1


with open(sys.argv[1], 'w') as csvFile:
    writer = csv.writer(csvFile)
    for row in DataValues:
    	writer.writerow(row)

csvFile.close()
