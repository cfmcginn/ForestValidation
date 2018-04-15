#!/bin/bash

for i in *.log
do
    cat $i
    echo "Log was '$i'"
    sleep 3
done