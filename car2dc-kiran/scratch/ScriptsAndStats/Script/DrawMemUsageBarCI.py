

########################################################################################
##### Generates a bar graph showing Memory utilization #################################
########################################################################################


import numpy as np
import matplotlib.pyplot as plt
import matplotlib
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 13})

# Graph input data
Dir = "../Stats/CPUUsageResults/"			# Recent results
#Dir = "../Stats_ResultsInThesis/CPUUsageResults/"       # Thesis results
Filename = {0:[Dir+"10_Nill_Mem.txt", Dir+"10_100_5_1_2001_Mem.txt", Dir+"10_200_5_1_2001_Mem.txt", Dir+"10_500_5_1_2001_Mem.txt"]}
j = 0

from collections import defaultdict
Y = defaultdict(list)

# Prepare data
Count_files = 0
while Count_files <= 0:
	each = Filename[Count_files]
	for each_file in each:
		Lines = []
		IgnoreLinesCount = 0
		with open(each_file) as f:
		    	for line in f:
		    		if IgnoreLinesCount >= 500:
		    			Lines.append(float(line)/1024.0)
				IgnoreLinesCount += 1
		Y[j].append(list(Lines))
        j = j + 1
	Count_files += 1


Std = defaultdict(list)
Mean = defaultdict(list)

# Calculate standard deviation and mean
for key,value in Y.iteritems():
	j = 0
	for each in value:
		Std[key].append(np.std(Y[key][j]))
		j = j + 1
for key,value in Y.iteritems():
	j = 0
        for each in value:
                Mean[key].append(np.mean(Y[key][j]))
		j = j + 1


		

X = ['0','100','200','500']
bar_color = ['cyan','blue','green','red']
bar_label = ['5kB','2kB','10kB','25kB']

# Create bars
def subcategorybar(X, vals, Std, Mean, Width=0.3):
    n = len(vals)
    _X = np.arange(len(X))
    for i in range(n):
        plt.bar(_X - Width/2. + i/float(n)*Width, Mean[i], yerr=Std[i],ecolor='black', 
                width=Width/float(n), color = bar_color[i], alpha=0.5,label = bar_label[i], align="edge")   
    plt.xticks(_X, X)

subcategorybar(X, [Y[0]],Std,Mean)

# Font settings
matplotlib.rcParams ['ps.useafm'] = True
matplotlib.rcParams ['pdf.use14corefonts'] = True
matplotlib.rcParams ['text.usetex'] = True

# Name x-axis and y-axis labels
plt.xlabel("Number of cars")
plt.ylabel("Memory usage (MB)")
plt.title("Average memory usage of 0, 100, 200 and 500 cars with \ndata size transfer 5kB running on one machine", fontsize=12)

# Save and show graph
plt.savefig("../plots/MemoryUsage.pdf")
plt.savefig("../plots/MemoryUsage.svg")
#plt.show()
