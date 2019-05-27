 

#############################################################################################
#### Generates a bar graph showing Number of tasks completed at different traffic densities #
#############################################################################################

import numpy as np
import matplotlib.pyplot as plt
import matplotlib
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 13})

# Graph input data
Dir = "../Stats/TaskResults/"			# Recent results
#Dir = "../Stats_ResultsInThesis/TaskResults/"	# Thesis results
Filenames = [Dir+"50_5_0_0_1.txt",Dir+"100_5_0_0_1.txt",Dir+"150_5_0_0_1.txt",Dir+"200_5_0_0_1.txt"]

j = 0
from collections import defaultdict
Y = defaultdict(list)

# Prepare data
for each_file in Filenames:
        Lines = []
        with open(each_file) as f:
                for line in f:
                        Y[j].append(float(line))
        j = j + 1

Std = []
Mean = []

# Calculate standard deviation and mean
for key,value in Y.iteritems():
        Std.append(np.std(Y[key]))
for key,value in Y.iteritems():
        Mean.append(np.mean(Y[key]))

bars = (50, 100, 150, 200)
 
y_pos = np.arange(len(bars))
color = ['cyan']

# Create bars
plt.bar(y_pos, Mean, yerr=Std,ecolor='black', align='center', width=0.3, color=color[0], alpha=0.5)
plt.xticks(y_pos, bars, fontweight='heavy')

matplotlib.rcParams ['ps.useafm'] = True
matplotlib.rcParams ['pdf.use14corefonts'] = True
matplotlib.rcParams ['text.usetex'] = True

 
# Create names on the x-axis and y-axis
plt.xlabel('Number of cars')
plt.ylabel('Total tasks completed')
plt.title('Total tasks completed by CHs at microcloud with 50, 100, 150 and 200 cars\nin the duration of 2000s', fontsize=12)
 
# Show and save graph
plt.savefig("../plots/TaskD.pdf")
plt.savefig("../plots/TaskD.svg")
#plt.show()
