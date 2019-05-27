

#############################################################################################
##### Generates a line graph showing CPU utilization ########################################
#############################################################################################


# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Graph input data
Dir = "../Stats/CPUUsageResults/"			# Recent results
Dir = "../Stats_ResultsInThesis/CPUUsageResults/"	# Thesis results
Filename = [Dir+"10_Nill_CPU.txt",Dir+"10_100_5_1_2001_CPU.txt", Dir+"10_200_5_1_2001_CPU.txt", Dir+"10_500_5_1_2001_CPU.txt"]
Data_Size = "5KB"

# Prepare data
j = 0
lines = {}
for each_file in Filename:
	with open(each_file) as f:
	    lines[j] = f.readlines()
	    j = j + 1

from collections import defaultdict
float_lines_y = defaultdict(list)

def is_number_tryexcept(s):
    # Returns True is string is a number
    try:
        float(s)
        return True
    except ValueError:
        return False

j = 0

for each_file in Filename:
	for each in lines[j]:
		if is_number_tryexcept(each):
			float_lines_y[j].append(float(each))
		else:
			float_lines_y[j].append(100.0)
	j = j + 1

# Values in X-axis
Time_X = []
count = 1800
i = 1
while i <= count:
	Time_X.append(i)
	i = i + 1

axes = plt.gca()
axes.set_ylim([0,15])
 
# Make a data frame
df=pd.DataFrame({'x': Time_X, 'System idle': float_lines_y[0], '100 cars': float_lines_y[1],'200 cars': float_lines_y[2], '500 cars':float_lines_y[3]})
 
# style
plt.style.use('seaborn-darkgrid')
 
colors = ['rosybrown', 'red', 'darkgreen', 'lime', 'red', 'olive', 'dodgerblue', 'yellow','cyan', 'teal']
alpha_color=[0.8,0.9,0.7,0.5]

# Create multiple lines plot
num=0
for column in df.drop('x', axis=1):
	plt.plot(df['x'], df[column], marker='', linewidth=1, label=column, color=colors[num], alpha=0.8) 
	num = num + 1
 
# Add legend
handles, labels = plt.gca().get_legend_handles_labels()
order = [3,0,1,2]
plt.legend([handles[idx] for idx in order],[labels[idx] for idx in order],loc='best', prop={'size': 11}, ncol=1,handleheight=2.4, labelspacing=0.05)
 
# Add titles
Title = "CPU Utilization for 0, 100, 200 and 500 number of cars in one VM with \nData Sizes 5kB"
plt.title(Title, fontsize=12, fontweight=0, color='black')
plt.xlabel("Time (s)")
plt.ylabel("CPU Utilization (%)")

# Save and show graph
plt.savefig("../plots/CPULines.svg")
plt.savefig("../plots/CPULines.pdf")
plt.show()

