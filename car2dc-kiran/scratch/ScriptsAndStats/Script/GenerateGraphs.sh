#!/bin/bash

if [ "$#" -ne 0 ]; then
    echo "Illegal number of parameters"
    echo "USAGE: Script.sh <Folder to save plots>"
    exit
fi

python NumberOfCarsInCluster.py
echo '----------------------------> Cluster size plot generated'
python NumberOfCarsInCluster_500.py ClusterCalculation1.csv NoOfVehiclesInCluster_MC1.csv 1
python NumberOfCarsInCluster_500.py ClusterCalculation2.csv NoOfVehiclesInCluster_MC2.csv 2
python NumberOfCarsInCluster_500.py ClusterCalculation3.csv NoOfVehiclesInCluster_MC3.csv 3
echo '----------------------------> cluster size 3 micro clouds generated'
python DrawTheoAndExpCI.py
echo '----------------------------> Theoretical and experimental based data collection results generated'
python ConsecutiveCH.py
echo '----------------------------> Consecutive cluster head count plot generated'
python AvgClusterTimeBar.py
echo '----------------------------> Average cluster formation time plot generated'
python DrawDCA1Bar.py
echo '----------------------------> Data collection plot generated'
python DrawDCA2Bar.py
echo '----------------------------> Data aggregation plot generated'
python DrawTaskD.py
echo '----------------------------> Task distribution plot generated'
python DrawCPUUsageBarCI.py 
echo '----------------------------> CPU usage plot generated'
python DrawMemUsageBarCI.py
echo '----------------------------> Memory usage plot generated'


