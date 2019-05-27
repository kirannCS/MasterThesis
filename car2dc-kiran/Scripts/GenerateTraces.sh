#!/bin/bash

#######################################################################################################
######## Used to generate traces for manhattan scenario ###############################################
#######################################################################################################

if [[ $# -lt 1 ]] ; then
    echo 'usage: ./GenerateTraces.sh <sumoconfigname>'
    exit 0
fi

# Traces are saved in this directory
SumoCfgScratchDir="/scratch-shared/kiran.narayanaswamy/Traces/"

# Traffic density in the scenario
TrafficDensity=( 600 700 800)

# Runtime in seconds
RunTime=( 2001)

# Total number of traces required
NumberOfTraces=1

CfgName=$1


GenerateScenario() 
{
        cd ../sumo_data/GenerateRandTraffic/
        ./generateScenario.sh $1 $2 $3
}

for i in "${RunTime=[@]}"
do
	SaveDir=$SumoCfgScratchDir"runtime_"$i
	mkdir -p $SaveDir
	for j in "${TrafficDensity[@]}"
	do
		# For one traffic density only once .sumocfg is generated
		GenerateScenario $CfgName $j $i 
		cp -r "manhattan_"$CfgName ../manhattan/
		TrafficDensityDir=$SaveDir"/"$j
		mkdir -p $TrafficDensityDir
		Count=1
		# For number of different traces random seed is used (same .sumocfg is used)
		while [ $Count -le $NumberOfTraces ]; do
			SaveFileAs=$TrafficDensityDir"/"$Count".xml"
			
			sumo -c ../manhattan/"manhattan_"$CfgName"/manhattan_"$CfgName.sumocfg --seed $RANDOM --fcd-output ../manhattan/mini.rou.xml
			cp ../manhattan/mini.rou.xml $SaveFileAs
			Count=$((Count+1))
		done
   	done
done

