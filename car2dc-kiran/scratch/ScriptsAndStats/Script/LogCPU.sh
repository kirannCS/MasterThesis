#!/bin/bash

##########################################################################################
##### Logs CPU utilization using top command ########################################### #
##########################################################################################

if [ "$#" -ne 2 ]; then
    echo "Illegal number of parameters"
    echo "USAGE: Script.sh <Number of seconds to capture CPU utilization> <machine number>"
    exit
fi


top -d 1 -b -n $1  | awk '/%Cpu/ { printf "%s\n", $2}' > $2"_CPU.txt"
