# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Graph input data
Dir = "../Stats/CPUUsageResults/"                       # Recent results
Dir = "../Stats_ResultsInThesis/CPUUsageResults/"	# Thesis results
Filename = [Dir+"10_Nill_Mem.txt",Dir+"10_100_5_1_2001_Mem.txt", Dir+"10_200_5_1_2001_Mem.txt", Dir+"10_500_5_1_2001_Mem.txt"]
Data_Size = "5KB"

j = 0
lines = {}

# Prepare data
for each_file in Filename:
	with open(each_file) as f:
	    lines[j] = f.readlines()
	    j = j + 1

j = 0
from collections import defaultdict
Y = defaultdict(list)

for each_file in Filename:
	for each in lines[j]:
	    Y[j].append(float(each)/1024.0)	# Display memory in Y axis in MB
	j = j + 1


# Values in X-axis
Time_X = []
count = 1800
i = 1
while i <= count:
	Time_X.append(i+120)
	i = i + 1


 
# Make a data frame
df=pd.DataFrame({'x': Time_X, 'System idle':Y[0],'100 Cars':Y[1], '200 Cars': Y[2], '500 Cars': Y[3]})
 
# style
plt.style.use('seaborn-darkgrid') 
colors = ['rosybrown', 'red', 'darkgreen', 'lime', 'darkorange', 'olive', 'dodgerblue', 'green','cyan', 'teal']

# Create multiple lines plot
num=0
for column in df.drop('x', axis=1):
	plt.plot(df['x'], df[column], marker='', color=colors[num], linewidth=1, alpha=0.8, label=column)
	num = num + 1
 
# Add legend
handles, labels = plt.gca().get_legend_handles_labels()
order = [3,0,1,2]
plt.legend([handles[idx] for idx in order],[labels[idx] for idx in order],loc='best', prop={'size': 11}, ncol=1,handleheight=2.4, labelspacing=0.05)
 
# Add titles
Title = "Memory usage for System idle, 100, 200 and 500 number of cars with datasize = " + Data_Size  
plt.title(Title,fontsize=12, fontweight=0, color='black')
plt.xlabel("Time (s)")
plt.ylabel("Memory Usage (MB)")

# Save and show graph
plt.savefig("../plots/MemLines.svg")
plt.savefig("../plots/MemLines.pdf")
plt.show()

