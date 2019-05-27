

#############################################################################################
##### Generates a bar graph showing Data Aggregation results for different traffic densities #
#############################################################################################

import numpy as np
import matplotlib.pyplot as plt
import matplotlib
plt.rcParams["font.family"] = "Times New Roman"
plt.rcParams.update({'font.size': 13})

# Graph input data
Dir = "../Stats/DCAResults/"				# Recent results
#Dir = "../Stats_ResultsInThesis/DCAResults/"		# Thesis results
Filename = {0:[Dir +"150_5_1_25_1.txt", Dir +"150_5_1_50_1.txt",Dir +"150_5_1_75_1.txt",Dir +"150_5_0_0_1.txt"], 1: [Dir +"150_10_1_25_1.txt", Dir +"150_10_1_50_1.txt", Dir +"150_10_1_75_1.txt",Dir +"150_10_0_0_1.txt"]}

from collections import defaultdict
Y = defaultdict(list)

Count_files = 0
j = 0

# Prepare data
while Count_files <= 1:
	each = Filename[Count_files]
	for each_file in each:
		Lines = []
		with open(each_file) as f:
		    for line in f:
		    	Lines.append(float(line) /1024 / 1024)		# divide 2 times by 1024 to show results in MB
		Y[j].append(list(Lines))
        j = j + 1
	Count_files += 1

for key,value in Y.iteritems():
	print(len(Y[key]))

Std = defaultdict(list)
Mean = defaultdict(list)

# Calculates standard deviation and mean
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


		
X = ['25','50', '75', '100']
bar_color = ['cyan','blue']
bar_label = ['5kB','10kB']
rects = []

# Create bars
def subcategorybar(X, vals, Std, Mean,width=0.6):
    n = len(vals)
    _X = np.arange(len(X))
    for i in range(n):
        rects.append(plt.bar(_X - width/2. + i/float(n)*width, Mean[i], yerr=Std[i],ecolor='black', 
                width=width/float(n), color = bar_color[i], alpha=0.5,label = bar_label[i], align="edge"))
    plt.xticks(_X, X)

subcategorybar(X, [Y[0],Y[1]],Std,Mean)

# Font settings
matplotlib.rcParams ['ps.useafm'] = True
matplotlib.rcParams ['pdf.use14corefonts'] = True
matplotlib.rcParams ['text.usetex'] = True


# Name x-axis and y-axis labels
plt.xlabel("Percentage of data CH uploaded")
plt.ylabel("Data collected (MB)")
plt.title("Total data collected at microcloud when \ndifferent aggregation percentages are used", fontsize=12)

# Create a legend
plt.legend(loc='best',prop={'size': 12})

def autolabel(rects):
	#Attach a text label above each bar displaying its height
    for rect in rects:
        height = rect.get_height()
        plt.text(rect.get_x() + rect.get_width()/2., 1.0*height,
                '%d' % int(height),
                ha='center', va='bottom')

# Uncomment to print Y values on each bar
#autolabel(rects[0])
#autolabel(rects[1])

# Save and show graph
plt.savefig("../plots/DCAAggregationPercentage.pdf")
plt.savefig("../plots/DCAAggregationPercentage.svg")
#plt.show()
