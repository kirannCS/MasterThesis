# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import csv
from collections import defaultdict


Dir = "../Stats/ClusterFormationTime/"
Filename = [Dir+"AvgClusterFormationTime_50.csv", Dir+"AvgClusterFormationTime_100.csv", Dir+"AvgClusterFormationTime_200.csv"]
Data_Size = "5KB"

j = 0
Time = defaultdict(list)
for each_file in Filename:
        with open(each_file) as csv_file:
                csv_data=csv.reader(csv_file)
		for row in csv_data:
			#if float(row[0]) <= 50 and float(row[0]) >= 10: 
			Time[j].append(float(row[1]))
		print(len(Time[j]))
		j = j + 1




count = len(Time[0])
Time[0] = Time[0][:321]
Time[1] = Time[1][:321]
Time[2] = Time[2][:321]


Time_X = []
i = 1
while i <= 321:
	Time_X.append(i)
	i = i + 1


 
# Make a data frame
df=pd.DataFrame({'x': Time_X, '50 cars': Time[0],'100 cars': Time[1], '200 cars':Time[2]})
#df=pd.DataFrame({'x': Time_X, '0 Cars': float_lines_y[0], '5 cars 2kB': float_lines_y[1],'10 cars 2kB': float_lines_y[2], '15 cars 2kB':float_lines_y[3], '5 cars 10kB': float_lines_y[4],'10 cars 10kB': float_lines_y[5], '15 cars 10kB':float_lines_y[6], '5 cars 25kB': float_lines_y[7],'10 cars 25kB': float_lines_y[8], '15 cars 25kB':float_lines_y[9]})
 
# style
plt.style.use('seaborn-darkgrid')
 
# create a color palette
palette = plt.get_cmap('Set2')
 

#colors = ['rosybrown', 'olivedrab', 'darkgreen', 'blue']
colors = ['red', 'olivedrab', 'darkgreen', 'lime', 'rosybrown', 'olive', 'dodgerblue', 'yellow','cyan', 'teal']
# multiple line plot
num=0
for column in df.drop('x', axis=1):
	plt.plot(df['x'], df[column], marker='', color=colors[num], linewidth=1, alpha=0.9, label=column) 
	num = num + 1
 
# Add legend
plt.legend(loc='lower right', prop={'size': 11}, ncol=2,handleheight=2.4, labelspacing=0.05)
 
# Add titles
#Title = "CPU Utilization for 5, 10 and 15 number of cars in one VM with " + Data_Size + " transfer"
Title = "Cluster formaton time for 50, 100 and 200 number of cars in one VM with \nData Sizes 5kB"
plt.title(Title, fontsize=12, fontweight=0, color='black')
plt.xlabel("Time (5s)")
plt.ylabel("Time taken(s)")

plt.show()

