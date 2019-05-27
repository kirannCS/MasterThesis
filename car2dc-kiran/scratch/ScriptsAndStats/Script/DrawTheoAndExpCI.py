
#############################################################################################
##### Generates a bar graph showing expected vs experimental data collection results ########
#############################################################################################

import numpy as np
import matplotlib.pyplot as plt
import matplotlib
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 13})

# Graph inpt data
Dir = "../Stats/DCAComparisonResults/"				# Recent results
#Dir = "../Stats_ResultsInThesis/DCAComparisonResults/"       	# Thesis results
Filename = {0:[Dir +"50_T.txt", Dir +"100_T.txt",Dir +"150_T.txt",Dir +"200_T.txt"], 1: [Dir +"50_E.txt", Dir +"100_E.txt", Dir +"150_E.txt",Dir +"200_E.txt"]}
j = 0

from collections import defaultdict
Y = defaultdict(list)

# Prepare data
Count_files = 0
while Count_files <= 1:
	each = Filename[Count_files]
	for each_file in each:
		Lines = []
		with open(each_file) as f:
		    for line in f:
		    	Lines.append(float(line) /1024 / 1024)
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


		
X = ['50','100', '150', '200']
bar_color = ['cyan','blue']
bar_label = ['Theoretical','Experiments']


# Create bars
def subcategorybar(X, vals, Std, Mean,width=0.6):
    n = len(vals)
    _X = np.arange(len(X))
    for i in range(n):
        plt.bar(_X - width/2. + i/float(n)*width, Mean[i], yerr=Std[i],ecolor='black', 
                width=width/float(n), color = bar_color[i], alpha = 0.5,label = bar_label[i], align="edge")   
    plt.xticks(_X, X, weight='normal')

subcategorybar(X, [Y[0],Y[1]],Std,Mean)

matplotlib.rcParams ['ps.useafm'] = True
matplotlib.rcParams ['pdf.use14corefonts'] = True
matplotlib.rcParams ['text.usetex'] = True

# Draw labels
plt.xlabel("Number of cars")
plt.ylabel("Data collected (MB)")
plt.title("Total data collected at micro cloud - expected \nversus experimental results", fontsize=12)

#Draw legend
plt.legend(loc='best',prop={'size': 12})

# Save and show graph
plt.savefig("../plots/DCATheoVsExperiment.pdf")
plt.savefig("../plots/DCATheoVsExperiment.svg")
#plt.show()
