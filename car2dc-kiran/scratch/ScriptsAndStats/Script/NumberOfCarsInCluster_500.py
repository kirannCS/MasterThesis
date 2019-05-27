

#####################################################################################################################
##### Generates a bar graph showing Cluster size in Mobility trace and Experiment in 3 micro clouds scenario ########
##### Pass 3 arguments "ClusterCalculation<Num>.csv" "NoOfVehiclesInCluster_MC<Num>.csv" "<Num>" ####################
#####################################################################################################################


# libraries and data
import sys
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import csv
import matplotlib
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 18})

from collections import defaultdict
float_lines_y = defaultdict(list)

# Graph input data
Dir = "../Stats/500_Vehicles/" 
#Dir = "../Stats_ResultsInThesis/500_Vehicles/" 
FilenameList = [Dir+sys.argv[1], Dir+sys.argv[2]]

# Prepare data
j = 0
for each_file in FilenameList:
	NewIterValues = []
        IterValues = []
        CountValues = []
        print("filename", each_file)
	with open(each_file) as Datafile:
        	csv_data=csv.reader(Datafile)
        	my_dict = {rows[0]:rows[1] for rows in csv_data}

	del my_dict["StepCount"]


	for key,value in my_dict.iteritems():
                #print(key, value)
        	IterValues.append(int(key))
                if(j == 1):
        		IterValues.append(int(key))
        		IterValues.append(int(key))
        		IterValues.append(int(key))
        		IterValues.append(int(key))
	keys = sorted(IterValues)
	for each in keys:
        	NewIterValues.append(each)
        	CountValues.append(int(my_dict[str(each)]))

	float_lines_y[j] = list(CountValues)
	del NewIterValues[:]
        del IterValues[:]
        del CountValues[:]
	j = j+1


print("len", len(float_lines_y[0]))
print("len", len(float_lines_y[1]))

# Values in X-axis
Time_X = []
count = len(float_lines_y[0])
i = 1
while i <= count:
	Time_X.append(i)
	i = i + 1


New_List = list(float_lines_y[1][:len(float_lines_y[0])])
print("len", len(New_List))

# Make a data frame
df=pd.DataFrame({'x': Time_X, 'FCD file': float_lines_y[0], 'Implementation': New_List})
axes = plt.gca()
axes.set_ylim([0,35])
 
# create a color palette
colors = ['red', 'blue']

# multiple line plot
num=0
for column in df.drop('x', axis=1):
	#plt.plot(df['x'], df[column], marker='', color='black', linewidth=1, alpha=alpha_color, label=column)
	plt.plot(df['x'], df[column], marker='', linewidth=1, label=column,color=colors[num],alpha=0.8)
	num = num + 1
 
# Font settings
matplotlib.rcParams ['ps.useafm'] = True
matplotlib.rcParams ['pdf.use14corefonts'] = True
matplotlib.rcParams ['text.usetex'] = True

# Add legend
plt.legend(loc='best',prop={'size': 18})
 
# Add titles
Title = "Cluster size at micro cloud " + sys.argv[3]
plt.title(Title, loc='center', fontsize=18, fontweight=0, color='black')
plt.xlabel("Time (s)")
plt.ylabel("Cluster size")

# Save and show graph
plt.savefig("../plots/MC"+sys.argv[3]+".pdf")
plt.savefig("../plots/MC"+sys.argv[3]+".svg")
#plt.show()
