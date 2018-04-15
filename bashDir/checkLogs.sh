#!/bin/bash

for i in *.log
do
    cat $i
    echo "Log was '$i'"
    sleep 3
done

for j in outDir*
do
    for i in $j/*.log
    do
	cat $i
	echo "Log was '$i'"
	sleep 3
    done
done