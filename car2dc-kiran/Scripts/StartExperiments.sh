#!/bin/bash




#Directory where all the stats (results) of the experiments are stored
StatsScratchDir="/scratch-shared/kiran.narayanaswamy/ScriptsAndStats/Stats/"

#Path where scripts required to run experiments are stored
ScriptsScratchDir="/scratch-shared/kiran.narayanaswamy/ScriptsAndStats/Script/"

#Path from where Mobility Trace files are stored under runtime tags
TraceDir="/scratch-shared/kiran.narayanaswamy/Traces/"

#Path where required Mobility Trace file is copied during the experiment
SumoFCD="../sumo_data/manhattan/mini.rou.xml"

#Path from where required config files are stored
ConfigDir="/scratch-shared/kiran.narayanaswamy/Config/"
ulimit -n 40960


#########################################################
####### Average Cluster Forrmation Time #################
#########################################################
#########################################################

######## Variables ####################################################################################
TrafficDensity=( 50 100 150 200)
ResultsDir=$StatsScratchDir"ClusterFormationTime/"
InitialResultsDir="../results/logs/"
Runtime="2000"
ReqConfigFile=$ConfigDir"5_0_0/config"
NoOfTrials=1

########## Script #####################################################################################
mkdir -p $StatsScratchDir"ClusterFormationTime"
#### copy required config file ###
cp -r $ReqConfigFile "../"
rm $InitialResultsDir"AvgClusterFormationTime.csv"
for i in "${TrafficDensity[@]}"
do
        Count=1
        while [ $Count -le $NoOfTrials ]; do
                #### copy Required Trace file ####
                TraceFile=$TraceDir"runtime_"$Runtime"/"$i"/"$Count".xml"
                cp $TraceFile $SumoFCD
                python StartTraffic.py
                sleep 60
                python $ScriptsScratchDir"RemoveLastLines.py" $InitialResultsDir"AvgClusterFormationTime.csv"
                python cleanup.py 24 1
                Count=$((Count+1))
        done
        mv $InitialResultsDir"AvgClusterFormationTime.csv" $ResultsDir"AvgClusterFormationTime_"$i".csv"
done
#######################################################################################################


#########################################################
####### Mobility Trace vs Experiments ###################
#########################################################
#########################################################


########Variables######################################################################################
TrafficDensity=( 200)
ResultsDir=$StatsScratchDir"NumberOfCarsInCluster/results/"
InitialResultsDir="../results/logs/"
NumberOfTrials=1
ReqConfigFile=$ConfigDir"5_0_0/config"
Runtime="2000"

########## Script #####################################################################################
mkdir -p $StatsScratchDir"NumberOfCarsInCluster"
mkdir -p $StatsScratchDir"NumberOfCarsInCluster/results"
for i in "${TrafficDensity[@]}"
do
	#### copy Required Trace file ####
	TraceFile=$TraceDir"runtime_"$Runtime"/"$i"/1.xml"
	cp $TraceFile $SumoFCD
	#### copy required config file ###
	cp -r $ReqConfigFile "../"
	#### Start Traffic ###############
	python StartTraffic.py
	sleep 60
	mv $InitialResultsDir"NoOfVehiclesInCluster.csv" $ResultsDir"Experiment_"$i"_"$Runtime".csv"
	python ClusterCalculation.py
	mv $InitialResultsDir"ClusterCalculation1.csv" $ResultsDir"FCD_"$i"_"$Runtime".csv"
	python cleanup.py 24 1
done
#######################################################################################################



#########################################################
####### Consecutive CH ##################################
#########################################################
#########################################################

########Variables######################################################################################
TrafficDensity=( 50 100 200)
ResultsDir=$StatsScratchDir"ConsecutiveCH/results/"
InitialResultsDir="../results/logs/"
Runtime="2000"
ReqConfigFile=$ConfigDir"5_0_0/config"

########## Script #####################################################################################
mkdir -p $StatsScratchDir"ConsecutiveCH"
mkdir -p $StatsScratchDir"ConsecutiveCH/results/"
#### copy required config file ###
cp -r $ReqConfigFile "../"
for i in "${TrafficDensity[@]}"
do
	#### copy Required Trace file ####
        TraceFile=$TraceDir"runtime_"$Runtime"/"$i"/1.xml"
        cp $TraceFile $SumoFCD
	python StartTraffic.py
	mv $InitialResultsDir"ConsecutiveCH.csv" $ResultsDir"ConsecutiveCH_"$i".csv"
	python cleanup.py 24 1	
done
#######################################################################################################



#########################################################
####### DCA expected vs experimental results ############
#########################################################
#########################################################


######## DCA Comparison ###############################################################################
TrafficDensity=( 50 100 150 200)
ResultsDir=$StatsScratchDir"DCAComparisonResults/"
InitialResultsDir="../results/logs/"
Runtime="2000"
ReqConfigFile=$ConfigDir"5_0_0_1/config"
NoOfTrials=3
DataSize=5
ClusterInterval=6

########## Script #####################################################################################
mkdir -p $StatsScratchDir"DCAComparisonResults"
CurrentDir=`pwd`
cd $StatsScratchDir"DCAComparisonResults"
rm *
cd $CurrentDir
#exit

#mkdir -p $StatsScratchDir"DCAComparisonResults"
#### copy required config file ###
cp -r $ReqConfigFile "../"

for i in "${TrafficDensity[@]}"
do
	Count=1
	TResultsFile=$ResultsDir""$i"_T.txt"
	EResultsFile=$ResultsDir""$i"_E.txt"
	while [ $Count -le $NoOfTrials ]; do
		#### copy Required Trace file ####
       		TraceFile=$TraceDir"runtime_"$Runtime"/"$i"/"$Count".xml"
		cp $TraceFile $SumoFCD
		python StartTraffic.py
		sleep 60
		python CalculateTheoreticalData.py $TResultsFile $DataSize $ClusterInterval
		python CalDataTransmission.py $EResultsFile "DCA"
		python cleanup.py 24 1
		Count=$((Count+1))
	done
done
#######################################################################################################



#########################################################
####### System Resources usage ##########################
#########################################################


######## Variables ####################################################################################
TrafficDensity=( 100 200 500)
DataSize=( 5)
ResultsDir=$StatsScratchDir"CPUUsageResults/"
InitialResultsDir="../results/logs/"
Runtime="2001"
NoOfTrials=1
SimMachines=( 10)
Time=1800

########## Script #####################################################################################
mkdir -p $StatsScratchDir"CPUUsageResults"
for l in "${SimMachines[@]}"
do
	echo $l
	bash $ScriptsScratchDir"SSHCPUusage.sh" $l $Time &
	bash $ScriptsScratchDir"SSHMemusage.sh" $l $Time &
				
done
sleep 1860
CurrentDir=`pwd`
cd
for l in "${SimMachines[@]}"
do
	mv $l"_CPU.txt" $ResultsDir""$l"_Nill_CPU.txt"
	mv $l"_Mem.txt" $ResultsDir""$l"_Nill_Mem.txt"
done
cd $CurrentDir

for i in "${TrafficDensity[@]}"
do
	for j in "${DataSize[@]}"
	do
		ReqConfigFile=$ConfigDir"OneVM_"$j"/config"
		cp -r $ReqConfigFile "../"
		Count=1
		while [ $Count -le $NoOfTrials ]; do
			#### copy Required Trace file ####
	       		TraceFile=$TraceDir"runtime_"$Runtime"/"$i"/"$Count".xml"
			cp $TraceFile $SumoFCD
			for l in "${SimMachines[@]}"
			do
				echo $l
				bash $ScriptsScratchDir"SSHCPUusage.sh" $l $Time &
				bash $ScriptsScratchDir"SSHMemusage.sh" $l $Time &
				
			done
			python StartTraffic.py
			CurrentDir=`pwd`
			cd
			for l in "${SimMachines[@]}"
			do
				mv $l"_CPU.txt" $ResultsDir""$l"_"$i"_"$j"_"$Count"_"$Runtime"_CPU.txt"
				mv $l"_Mem.txt" $ResultsDir""$l"_"$i"_"$j"_"$Count"_"$Runtime"_Mem.txt"
			done
			cd $CurrentDir
			python cleanup.py 24 1
			Count=$((Count+1))
		done
	done
done
####################################################################################################### 		


#########################################################
####### DCA and Task Results ############################
#########################################################
#########################################################


######## TASK and DCA #################################################################################
TrafficDensity=( 50 100 200)
DataSize=( 5 10)
AggregationSize=( 0)
ResultsDir1=$StatsScratchDir"TaskResults/"
ResultsDir2=$StatsScratchDir"DCAResults/"
InitialResultsDir="../results/logs/"
Runtime="2000"
NoOfTrials=3

########## Script #####################################################################################
mkdir -p $StatsScratchDir"TaskResults"
mkdir -p $StatsScratchDir"DCAResults"

#CurrentDir=`pwd`
#cd $StatsScratchDir"TaskResults"
#rm *
#cd $CurrentDir
#cd $StatsScratchDir"DCAResults"
#rm *
#cd $CurrentDir


for i in "${TrafficDensity[@]}"
do
        for j in "${DataSize[@]}"
        do
                ReqDataAggregation="0"
                for k in "${AggregationSize[@]}"
                do
                        ReqConfigFile=$ConfigDir""$j"_"$ReqDataAggregation"_"$k"/config"
                        cp -r $ReqConfigFile "../"
                        Count=1
                        ResultsFile1=$ResultsDir1"/"$i"_"$j"_"$ReqDataAggregation"_"$k"_"$Count".txt"
                        ResultsFile2=$ResultsDir2"/"$i"_"$j"_"$ReqDataAggregation"_"$k"_"$Count".txt"
                        while [ $Count -le $NoOfTrials ]; do
                                #### copy Required Trace file ####
                                TraceFile=$TraceDir"runtime_"$Runtime"/"$i"/"$Count".xml"
				cp $TraceFile $SumoFCD
                                python StartTraffic.py
                                sleep 60                
                                python CalDataTransmission.py $ResultsFile1 "Task"
                                python CalDataTransmission.py $ResultsFile2 "DCA"
                                python cleanup.py 24 1
                                Count=$((Count+1))
                        done
                        ReqDataAggregation="1"
                done
        done
done
####################################################################################################### 


#########################################################
####### DCA aggregation results #########################
#########################################################
#########################################################


######## Variables  ###################################################################################
TrafficDensity=( 150)
DataSize=( 5 10)
AggregationSize=( 0 25 50 75)
ResultsDir1=$StatsScratchDir"TaskResults/"
ResultsDir2=$StatsScratchDir"DCAResults/"
InitialResultsDir="../results/logs/"
Runtime="2000"
NoOfTrials=3

########## Script #####################################################################################
mkdir -p $StatsScratchDir"TaskResults"
mkdir -p $StatsScratchDir"DCAResults"

#CurrentDir=`pwd`
#cd $StatsScratchDir"TaskResults"
#rm *
#cd $CurrentDir
#cd $StatsScratchDir"DCAResults"
#rm *
#cd $CurrentDir


for i in "${TrafficDensity[@]}"
do
        for j in "${DataSize[@]}"
        do
                ReqDataAggregation="0"
                for k in "${AggregationSize[@]}"
                do
                        ReqConfigFile=$ConfigDir""$j"_"$ReqDataAggregation"_"$k"/config"
                        cp -r $ReqConfigFile "../"
                        Count=1
                        ResultsFile1=$ResultsDir1"/"$i"_"$j"_"$ReqDataAggregation"_"$k"_"$Count".txt"
                        ResultsFile2=$ResultsDir2"/"$i"_"$j"_"$ReqDataAggregation"_"$k"_"$Count".txt"
                        while [ $Count -le $NoOfTrials ]; do
                                #### copy Required Trace file ####
                                TraceFile=$TraceDir"runtime_"$Runtime"/"$i"/"$Count".xml"
				cp $TraceFile $SumoFCD
                                python StartTraffic.py
                                sleep 60
				if [[ $i -eq 150 && $j -eq 5 && $ReqDataAggregation -eq "0" ]]
				then            
                                	python CalDataTransmission.py $ResultsFile1 "Task"
				fi
                                python CalDataTransmission.py $ResultsFile2 "DCA"
                                python cleanup.py 24 1
                                Count=$((Count+1))
                        done
                        ReqDataAggregation="1"
                done
        done
done
#######################################################################################################



#########################################################
####### 3 Micro clouds managed by 3 RSU #################
####### Cluster size validation #########################
#########################################################

######## Variables ####################################################################################
TrafficDensity=( 500)
DataSize=( 5)
AggregationSize=( 0)
ResultsDir=$StatsScratchDir"500_Vehicles/"
InitialResultsDir="../results/logs/"
Runtime="2000"
NoOfTrials=1
ConfigDir="/scratch-shared/kiran.narayanaswamy/500_Config/"
SimMachines=( )
Time=1800

########## 3 micro clouds #############################################################################
mkdir -p $StatsScratchDir"500_Vehicles"
CurrentDir=`pwd`
cd $StatsScratchDir"500_Vehicles"
rm *
cd $CurrentDir

for i in "${TrafficDensity[@]}"
do
	for j in "${DataSize[@]}"
	do
		ReqDataAggregation="0"
		for k in "${AggregationSize[@]}"
		do
			ReqConfigFile=$ConfigDir""$j"_"$ReqDataAggregation"_"$k"/config"
			cp -r $ReqConfigFile "../"
			Count=1
			while [ $Count -le $NoOfTrials ]; do
				#### copy Required Trace file ####
		       		TraceFile=$TraceDir"runtime_"$Runtime"/"$i"/"$Count".xml"
				cp $TraceFile $SumoFCD
				for l in "${SimMachines[@]}"
	                        do
        	                        bash $ScriptsScratchDir"SSHCPUusage.sh" $l $Time &
                	                bash $ScriptsScratchDir"SSHMemusage.sh" $l $Time &
                                
                        	done

				python StartTraffic.py
				sleep 60		
				CurrentDir=`pwd`
	                        cd
        	                for l in "${SimMachines[@]}"
                	        do
                        	        mv $l"_CPU.txt" $ResultsDir""$l"_"$i"_"$j"_"$Count"_"$Runtime"_CPU.txt"
                                	mv $l"_Mem.txt" $ResultsDir""$l"_"$i"_"$j"_"$Count"_"$Runtime"_Mem.txt"
	                        done
        	                cd $CurrentDir

				mv $InitialResultsDir"NoOfVehiclesInCluster_MC1.csv" $ResultsDir
				mv $InitialResultsDir"NoOfVehiclesInCluster_MC2.csv" $ResultsDir
				mv $InitialResultsDir"NoOfVehiclesInCluster_MC3.csv" $ResultsDir
				mv $InitialResultsDir"DataInMultipleCluster_RSU1.csv" $ResultsDir
				mv $InitialResultsDir"DataInMultipleCluster_RSU2.csv" $ResultsDir
				mv $InitialResultsDir"DataInMultipleCluster_RSU3.csv" $ResultsDir
				python ClusterCalculation_500.py
				mv $InitialResultsDir"ClusterCalculation1.csv" $ResultsDir
				mv $InitialResultsDir"ClusterCalculation2.csv" $ResultsDir
				mv $InitialResultsDir"ClusterCalculation3.csv" $ResultsDir
				python cleanup.py 24 1
				Count=$((Count+1))
			done
			ReqDataAggregation="1"
		done
	done
done
####################################################################################################### 

