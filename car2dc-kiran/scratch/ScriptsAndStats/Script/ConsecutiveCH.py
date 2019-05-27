
##########################################################################################
##### Generates a bar graph to describe Consecutive times car elected as CH distribution #
##########################################################################################


import numpy as np
import matplotlib.pyplot as plt
import csv
import matplotlib
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 13})


# Graph input data
Dir = "../Stats/ConsecutiveCH/results/"                                # Recent run results
#Dir = "../Stats_ResultsInThesis/ConsecutiveCH/results/"		       # Thesis results
Filename = [Dir+"ConsecutiveCH_50.csv", Dir+"ConsecutiveCH_100.csv", Dir+"ConsecutiveCH_200.csv"]

from collections import defaultdict
Y = defaultdict(list)

# Prepare data
j = 0
for each_file in Filename:
	my_dict_keys = []
	with open(each_file) as csv_file:
		csv_data=csv.reader(csv_file)
		my_dict = {rows[0]:rows[1] for rows in csv_data}
		print(my_dict)
		del my_dict["Interval"]
		for key,value in my_dict.iteritems():
			my_dict_keys.append(int(key))
		my_dict_keys.sort()
		for each in my_dict_keys:
			Y[j].append(my_dict[str(each)])
	j = j + 1
	del my_dict_keys
	del my_dict
		


X = ['0','1','2', '3', '4', '5', '6', '7', '8']
bar_color = ['cyan', 'blue','green','red']
bar_label = ['50 cars', '100 cars', '200 cars']

# Create bars
def subcategorybar(X, vals, width=0.8):
    num = 0
    n = len(vals)
    _X = np.arange(len(X))
    for i in range(n):
	plt.style.use('grayscale')
        plt.bar(_X - width/2. + i/float(n)*width, vals[i], 
                width=width/float(n), color = bar_color[num], alpha=0.8, label = bar_label[num], align="edge") 
	num = num + 1
    plt.xticks(_X, X)

subcategorybar(X, [Y[0][:9],Y[1][:9],Y[2][:9]])

# Font settings
matplotlib.rcParams ['ps.useafm'] = True
matplotlib.rcParams ['pdf.use14corefonts'] = True
matplotlib.rcParams ['text.usetex'] = True

# Name x-axis and y-axis labels
plt.xlabel("Consecutive count")
plt.ylabel("Number of times")
plt.title("Consecutive times cars elected as Cluster Head in different traffic densities", fontsize=12)

# Create legend
plt.legend(loc='best',prop={'size': 12})
		
# Save and show graph
plt.savefig("../plots/ConsecutiveCount.pdf")
plt.savefig("../plots/ConsecutiveCount.svg")
#plt.show()
