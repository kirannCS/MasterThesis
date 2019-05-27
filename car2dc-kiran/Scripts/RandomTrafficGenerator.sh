#!/bin/sh
# This is a comment!

#######################################################################################################
######## Used to generate traces for manhattan scenario (Old version). ################################
######## For new version check GenerateTraces.sh ######################################################
#######################################################################################################

if [ "$#" -ne 5 ]; then
    echo "Illegal number of parameters"
    echo "USAGE: Script.sh <GenerateSumoCfg - 0|1> <Number of cars in the scenario> <seed> <scenario_name> <runtime>"
    exit
fi

GenSumoCfg=$1
NumCars=$2
Seed=$3
CfgName=$4
Runtime=$5


cd ../sumo_data/GenerateRandTraffic/

if [ $GenSumoCfg = "0" ]; then
	sumo -c "../manhattan/manhattan_"$CfgName"/manhattan_"$CfgName".sumocfg" --seed $Seed --fcd-output ../manhattan/mini.rou.xml
else
        ./generateScenario.sh $CfgName $NumCars $Runtime
	mv "manhattan_"$4 ../manhattan/
	sumo -c "../manhattan/manhattan_"$CfgName"/manhattan_"$CfgName".sumocfg" --seed $Seed --fcd-output ../manhattan/mini.rou.xml
fi
