
##################################################################################################################################
# Readme contents
    1. 	Required installations and instructions to run experiments on a new machine
    2. 	Compilation steps
    3. 	Steps to run experiments on thesis setup
    4. 	Steps to run experiments on a new setup
	5. 	Steps to generate plots
	6.	Ports information
	7.	IP configuration
    8. 	Other details
	

##################################################################################################################################
1. 	Instructions to install software, libraries and required packages to run on fresh Virtual Machine (VM)

	a.  Install basics 
		+ sudo apt-get install open-vm-tools open-vm-tools-desktop vim git cmake 
		
	b.  SSH configuration: copy .ssh folder from the machine where already SSH to experimental setup machines was working before
		+ chmod 600 ~/.ssh/config # After copying requires change of permissions
		
	c.  Install "Protoc"
		Download -----> https://github.com/google/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz
		+ cd Downloads
		+ tar -zxvf protobuf-2.6.1.tar.gz
		+ sudo apt-get update
		+ sudo apt-get install build-essential 
		+ cd protobuf-2.6.1/ 
		+ ./configure
		+ make -j4
		+ make check -j4
		+ sudo make install -j4
		+ vim ~/.bashrc
		add export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
		+ source ~/.bashrc
		+ protoc --version     #To check if installed
		
	d.  Install "ZeroMQ", "CPPZMQ"
		+ sudo apt-get install libzmq-dev
		+ sudo apt-get install libzmq3-dev
		
	e.  Install "glut" required for visualization
		+ sudo apt-get install freeglut3-dev
		+ sudo apt-get install qt4-qmake
		+ sudo apt-get install libqt4-dev
		
	f.  Python modules required to run the "Helper" module
		+ sudo apt-get install python-pip  #pip install
		+ sudo pip install google
		+ sudo pip install protobuf #protobuf install  
		+ sudo apt-get install python-zmq #zmq install
		+ pip install netifaces #netifaces install
		+ pip install xmlr #xmlr iterative parser
		+ sudo apt-get update
		+ sudo apt-get -y upgrade
		

##################################################################################################################################    
2.  Compilation
	a.  Make a git clone
		+ git clone git@git.ccs-labs.org:car2dc-kiran
		
	b.  Steps to compile
		+ cd car2dc-kiran/src
		+ cmake .
		+ make -j8
	
	c. 	Steps to compile Qt-based visualization window
		+ cd car2dc-kiran/src/utilities/Visuals/build-QtVisuals-Desktop-Debug
		+ qmake ../QtVisuals/QtVisuals.pro
		+ make
		 

##################################################################################################################################
3.  Run experiments in the thesis setup
	+ rm -r /scratch-shared/kiran.narayanaswamy/ScriptsAndStats/Stats/
    cssh (cluster ssh) on ccs-sim02 to ccs-sim10 and ccs-lab9 to ccs-lab10 (Call it cluster "A")
    On cluster "A" execute the following steps
    + cd car2dc-kiran/Scripts
    + python cleanup.py 2 1                    #Clears previous logs and unkilled processes if any
    + python CommandReceiver.py 131.234.28.14        #131.234.28.14 is the IP address of ccs-sim01 (Helper module)    
    ssh on ccs-sim01 and execute
    + cd car2dc-kiran/Scripts
    + bash StartExperiments.sh
	All the results are saved in "/scratch-shared/kiran.narayanaswamy/ScriptsAndStats/Stats"

	
##################################################################################################################################
4.  Run experiments on any other setup	
    ssh on Machine "B" (where helper module is intended to run) and follow the steps given below
    + cd car2dc-kiran/
    + cp -r scratch/Config/ /scratch-shared/<Folder1>/            
    + cp -r scratch/ScriptsAndStats/  /scratch-shared/<Folder1>/
    + mkdir -p /scratch-shared/<Folder1>/ScriptsAndStats/Stats
	
    Open car2dc-kiran/config/common/config.xml and under tag "ParserIP" change the IP address to the IP address of the machine 
		on which Helper module is intended to run.
	In the same file, under tag "IPList" mention all the IPs on which AP processes and Car processes are intended to run. 
		In the list, the first IP address should be where the AP process is intended to run. Other IPs are corresponding to 
        machines where car processes are intended to run.
    Open car2dc-kiran/config/ap/config.xml and under tag "BS" change the IP address to new IP address where the AP process should run.
    Open car2dc-kiran/Scripts/StartExperiments.sh and change value of "StatsScratchDir" to "/scratch-shared/<Folder1>/ScriptsAndStats/Stats/" 
		where stats(results) of the experiments will be saved. "ScriptsScratchDir" to "/scratch-shared/<Folder1>/ScriptsAndStats/Scripts/",
        "TraceDir" to "/scratch-shared/<Folder1>/Traces" and "/scratch-shared/<Folder1>/Config" to "/scratch-shared/<Folder1>/Config/"
		
	cssh (cluster ssh) on all IP addresses in the "IPList" (Call it cluster "A")
    On cluster "A" execute the following steps
    + cd car2dc-kiran/Scripts
    + python cleanup.py 2 1                    #Clears previous logs and unkilled processes if any
    + python CommandReceiver.py <IP>        #IP is the IP address of M/C where Helper module should run
	
    ssh on Machine "B" in a new tab and execute
    + cd car2dc-kiran/Scripts
    + bash StartExperiments.sh


##################################################################################################################################
5.	Steps to generate plots
	On thesis setup
	+ mkdir -p /scratch-shared/kiran.narayanaswamy/ScriptsAndStats/plots
	+ cd /scratch-shared/kiran.narayanaswamy/ScriptsAndStats/Scripts
	+ bash GenerateGraphs.sh
	All plots generated are saved in "/scratch-shared/kiran.narayanaswamy/ScriptsAndStats/plots"
	
	On new setup
	+ mkdir -p /scratch-shared/<Folder1>/ScriptsAndStats/plots
	+ cd /scratch-shared/<Folder1>/ScriptsAndStats/Scripts
	+ bash GenerateGraphs.sh
	All plots generated are saved in "/scratch-shared/<Folder1>/ScriptsAndStats/plots"
	

##################################################################################################################################
6. 	Ports information
	-> Each car entity is assigned 5 ports. If Starting Port Number assigned to a particular vehcile is X then
	Clustering service - Port X, GPS - Port X + 1, Speed - Port X + 2, Data Collecion and Aggregation app - Port X + 3 (unused/reserved)
	Task Distribution app - Port X + 4 (Unused/Reserved)
	--> Ports usage
	* Clustering service in car (Pub) 	  ----------   X   ---------->> Apps (Data collection and Task Dist) # Publishes cluster information
	* GPS (Req) 						  <<--------  5555 ---------->> GPS Sensor (Rep)
	* GPS (Rep)							  <<--------  X+1  ---------->> Clustering service (Req)
	* GPS (Req) 						  <<--------  5555 ---------->> GPS Sensor (Rep)
	* GPS (Rep) 						  <<--------  X+2  ---------->> Clustering service (Req)
	* Car/AP (Push) 					  ----------  6666 ---------->> UDM/Decider (Pull) # Unicast/Broadcast messages
	* UDM/Decider (Pub) 				  ----------  6667 ---------->> Car/AP (Sub) # Unicast/Broadcast messages
	* Parser (Pub) 						  ----------  6668 ---------->> Command Receiver (Sub) # Used to distribute Cars and APs in different machines
	* Parser (Pub) 						  ----------  6669 ---------->> Car (Sub) # Publishes exit status to cars
	* DCA app (Push) 					  ----------  4000 ---------->> Visual Component (Pull)
	* Task app (Push)                     ----------  4000 ---------->> Visual Component (Pull)
	
	If Y is the port assigned in xml config file for an AP (BS/RSU)
	# AP on receving messages push them to worker threads which processes the new messages
	* AP (Push) 						  ----------   Y   ---------->> AP Worker Threads (Pull) 
	* AP (Pub) 							  ----------  Y+1  ---------->> Visual Component (Qt and OpenGL) (Sub) # Visual info
	# AP Data Collection process on receving messages push them to worker threads which processes the new messages
	* AP Data Collection (Push)           ----------  Y+2  ---------->> AP Worker Threads (Pull) 
	* AP (Pub) 							  ----------  Y+3  ---------->> Apps (Sub) # Cluster change information
	
	
##################################################################################################################################
7.	IP configuration (Not for running thesis experiments. This configuration is to run experiments individually only)
	+ vim car2dc-kiran/config/common/config.xml
	Change the "ParserIP" to the IP address of the M/C where Helper module should run
	Change the "IPLIst" to the list of IP's where in the 1st IP is corresponding to M/C where all AP's should run 
		And next, mention IPs of all the M/Cs where car processes should run
	+ vim car2dc-kiran/config/ap/config.xml
	Under the tag BS/RSU change value of "ip" to 1st IP in the IPList (as specified in the previous step)
	
	In order to run "python CommandReceiver.py <IP>" (On machines where cars and AP's processes should run) to receive commands 
		from helper module (StartTraffic.py)
	+ vim car2dc-kiran/Scripts/CommandReceiver.py
	Change the interface name (say eth0 or lo) in the line "ip = ni.ifaddresses('eth0')[ni.AF_INET][0]['addr']" to interface name
		on which IP address mentioned in "IPLIst" is present.
	<IP> should be the IP of the M/C where Helper module should run (This IP is required to connect with the Helper module so that
		it sends commands to run car and AP processes)
	Open car2dc-kiran/Scripts/CommandReceiver.py and change the IP address to new IP address to whatever IP address that was stored in 
	

##################################################################################################################################
8.  Other details
    a. 	Traces used in the Thesis experiments are in "/scratch-shared/kiran.narayanaswamy/Traces"
    b. 	In the experiments (bash StartExperiments.sh), according to the requirement, the 
		specific trace is by itself copied to "car2dc-kiran/sumo_data/manhattan/mini.rou.xml" 
    c. 	To run a single experiment
        1 Make the required changes in the config files
		2 Copy required trace file to "car2dc-kiran/sumo_data/manhattan/mini.rou.xml"
        3 In folder "car2dc-kiran/Scripts/" execute "python StartTraffic.py"
	d. 	Results produced in Thesis are found in "/scratch-shared/kiran.narayanaswamy/ScriptsAndStats/Stats_ResultsInThesis"
	f. 	Plots produced in Thesis are found in "/scratch-shared/kiran.narayanaswamy/ScriptsAndStats/plots_ResultsInThesis"
