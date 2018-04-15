#!/bin/bash

echo $#

if [ $# -ne 1 ];
then
    echo "Usage: sh testForestValidation.sh <inRootPath>"
    echo "Exit"
    exit 1
fi

for i in $1
do
    j=${i%.root}
    j=$j\_COPY.root
    cp $i $j
    ./bin/runForestDQM.exe $i $j
    exit 1
done