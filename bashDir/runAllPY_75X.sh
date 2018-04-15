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

filesToRun=(runForestAOD_PbPb_DATA_75X.py runForestAOD_PbPb_MIX_75X.py runForestAOD_PbPb_MB_75X.py runForestAOD_pp_MC_75X.py runForestAOD_pp_DATA_75X.py)
inputs=("root://cms-xrd-global.cern.ch//store/hidata/HIRun2015/HIHardProbes/AOD/PromptReco-v1/000/263/234/00000/84F9A542-51A7-E511-A31A-02163E0133D2.root" "root://cms-xrd-global.cern.ch//store/himc/HINPbPbWinter16DR/Pythia6_Dijet80_pp502_Hydjet_Cymbal_MB/AODSIM/75X_mcRun2_HeavyIon_v14-v1/100000/0C0AA366-F6EC-E611-A99F-008CFAFBEFE8.root" "root://cms-xrd-global.cern.ch//store/himc/HINPbPbWinter16DR/Hydjet_Quenched_Cymbal5Ev8_PbPbMinBias_5020GeV/AODSIM/NoPU_75X_mcRun2_HeavyIon_v14_75X_mcRun2_HeavyIon_v14-v1/80000/02D120BC-FADF-E611-9E3E-141877344134.root" "root://cms-xrd-global.cern.ch//store/himc/HINppWinter16DR/Pythia6_Dijet80_pp502/AODSIM/75X_mcRun2_asymptotic_ppAt5TeV_v3-v1/50000/0EB77382-040D-E611-9A34-0CC47A1DF5FA.root" "root://cms-xrd-global.cern.ch//store/data/Run2015E/HighPtJet80/AOD/PromptReco-v1/000/262/327/00000/02549305-CEA6-E511-8B93-02163E0136CE.root")


mkdir -p outDir$outputStr

pos=0
for i in "${filesToRun[@]}"
do
    j=${i%.py}
    j=$j\_$outputStr.py
    cp $i $j

    typeStr=${i%_75X.py}
    typeStr=${typeStr#runForestAOD_}
    echo $typeStr

    sed -i -e "s:HiForestAOD.root:HiForestAOD_$typeStr_$outputStr.root:g" $j

    echo "" >> $j
    echo "#BASH SCRIPT OVERRIDES" >> $j
    echo "process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32($numberEvt))" >> $j
    echo "process.source.fileNames = cms.untracked.vstring('${inputs[$pos]}')" >> $j

    cmsRun $j >& run_$typeStr\_$outputStr.log &
    pos=$((pos + 1))
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