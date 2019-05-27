#!/bin/bash

##########################################################################################
##### Does SSH and execute command on remote machine to log Memory Utilization ###########
##########################################################################################

if [ "$#" -ne 2 ]; then
    echo "Illegal number of parameters"
    echo "USAGE: Script.sh <sim machine number> <number of seconds>"
    exit
fi
 
ssh "ccs-lab"$1 "bash /scratch-shared/kiran.narayanaswamy/ScriptsAndStats/Script/LogMem.sh "$2" "$1

