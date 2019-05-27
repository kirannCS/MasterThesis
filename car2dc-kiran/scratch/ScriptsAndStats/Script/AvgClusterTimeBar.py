
########################################################################################
##### Generates a bar graph for average cluster formation time #########################
########################################################################################



import numpy as np
import csv
import matplotlib.pyplot as plt
from collections import defaultdict
import matplotlib


plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 13})

# Input graph data
Dir = "../Stats/ClusterFormationTime/"                                    ## Recenet run results
#Dir = "../Stats_ResultsInThesis/ClusterFormationTime/"                    ## Results used for thesis 
Filename = [Dir+"AvgClusterFormationTime_50.csv", Dir+"AvgClusterFormationTime_100.csv",Dir+"AvgClusterFormationTime_150.csv",Dir+"AvgClusterFormationTime_200.csv"]
j = 0


Map = defaultdict(list)
Std = []
Mean = []

# Prepare data
for each_file in Filename:
        with open(each_file) as csv_file:
                csv_data=csv.reader(csv_file)
		for row in csv_data:
			Map[j].append(float(row[1]))
		j = j + 1
			
			
# Calculate standard deviation and mean	
for each_list,value in Map.iteritems():
	Std.append(np.std(Map[each_list]))

for each_list,value in Map.iteritems():
	Mean.append(np.mean(Map[each_list]))


# Create bars
X = [50,100,150, 200]
fig, ax = plt.subplots()
ax.bar(X, Mean, xerr=0,yerr=Std, align='center', width = 14.0,ecolor='black', alpha=0.5,capsize=10, color='cyan')
ax.set_xticks(X)
ax.margins(0.1,0,tight=False)

# Font settings
matplotlib.rcParams ['ps.useafm'] = True
matplotlib.rcParams ['pdf.use14corefonts'] = True
matplotlib.rcParams ['text.usetex'] = True

# Name x-axis and y-axis labels
ax.set_ylabel('Cluster formation time (microseconds)')
plt.xlabel('Number of Cars')
ax.set_title('Average cluster formation time increases linearly with linear \nincrease in traffic density', fontsize=12)

# Save the figure and show
plt.tight_layout()
plt.savefig("../plots/ClusterFormationTime.pdf")
plt.savefig("../plots/ClusterFormationTime.svg")
#plt.show()
