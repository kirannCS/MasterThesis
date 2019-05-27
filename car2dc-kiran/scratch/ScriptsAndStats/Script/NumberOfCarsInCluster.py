
##########################################################################################
##### Generates a bar graph showing Cluster size in Mobility trace and Experiment ########
##########################################################################################

# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import csv
import matplotlib
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 18})
plt.rcParams.update({'font.size': 13})
from collections import defaultdict
float_lines_y = defaultdict(list)

# Graph input data
Dir = "../Stats/NumberOfCarsInCluster/results/"                                # Recent run results
#Dir = "../Stats_ResultsInThesis/NumberOfCarsInCluster/results/"                # Thesis results
FilenameList = [Dir+"FCD_200_2000.csv", Dir+"Experiment_200_2000.csv"]

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
df=pd.DataFrame({'x': Time_X, 'Mobility trace': float_lines_y[0], 'Implementation': New_List})
 
# create a color palette
colors = ['red', 'blue']

# multiple line plot
num=0
linestyle = ['-','--']
for column in df.drop('x', axis=1):
	#plt.plot(df['x'], df[column], marker='', color='black', linewidth=1, alpha=alpha_color, label=column)
	plt.plot(df['x'], df[column], label=column, color=colors[num],alpha=0.8, linestyle=linestyle[num])
	num = num + 1

# Font settings 
matplotlib.rcParams ['ps.useafm'] = True
matplotlib.rcParams ['pdf.use14corefonts'] = True
matplotlib.rcParams ['text.usetex'] = True

# Add legend
plt.legend(loc='best',prop={'size': 12})
 
# Add titles
Title = "Cluster size calculated from Mobility Trace and \nExperiment results vs time (Traffic Density = 200 Cars)"
plt.title(Title, loc='center', fontsize=12, fontweight=0, color='black')
plt.xlabel("Time (s)")
plt.ylabel("Cluster size")

# Save and show graph
plt.savefig("../plots/NumberOfCarsInCluster_200.pdf")
plt.savefig("../plots/NumberOfCarsInCluster_200.svg")
#plt.show()
