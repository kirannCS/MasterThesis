#!/bin/bash

##########################################################################################
##### Logs Memory usage using top command ################################################
##########################################################################################

if [ "$#" -ne 2 ]; then
    echo "Illegal number of parameters"
    echo "USAGE: Script.sh <Number of seconds to capture Memory usage> <machine number>"
    exit
fi


top -d 1 -b -n $1  | awk '/KiB Mem/ { printf "%s\n", $8}' > $2"_Mem.txt"
