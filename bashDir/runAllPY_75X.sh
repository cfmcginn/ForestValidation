#!/bin/bash

outputStr="DEFAULT"
numberEvt=100


if [ $# -ne 1 ]; 
then
    echo "Usage: sh runAllPY_75X.sh <isDefault>"
    echo "Exit"
    exit 1
elif [ $1 -eq 0  ] || [ $1 -eq 1  ];
then
    if [ $1 -eq 1 ];
    then
	echo "Running default forest"
    else
	echo "Running modded forest"
	outputStr="MODDED"
    fi
else
    echo "Argument <isDefault> given, '$1', is invalid. Must be '0' or '1' (Boolean). Exit"
    exit 1
fi

filesToRun=(runForestAOD_PbPb_DATA_75X.py runForestAOD_PbPb_MIX_75X.py runForestAOD_pp_MC_75X.py runForestAOD_PbPb_MB_75X.py runForestAOD_pp_DATA_75X.py)

mkdir -p outDir$outputStr

for i in "${filesToRun[@]}"
do
    j=${i%.py}
    j=$j\_$outputStr.py
    cp $i $j

    typeStr=${i%_75X.py}
    typeStr=${typeStr#runForestAOD_}
    echo $typeStr

    sed -i -e "s:HiForestAOD.root:HiForestAOD_$typeStr_$outputStr.root:g" $j

    echo "process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32($numberEvt))" >> $j

    cmsRun $j >& run_$typeStr\_$outputStr.log &
done

wait

for i in "${filesToRun[@]}"
do
    j=${i%.py}
    j=$j\_$outputStr.py
    
    typeStr=${i%_75X.py}
    typeStr=${typeStr#runForestAOD_}

    mv run_$typeStr\_$outputStr.log outDir$outputStr
    mv $j outDir$outputStr
    mv HiForestAOD*.root outDir$outputStr
done