//cpp dependencies
#include <iostream>
#include <string>
#include <map>

//ROOT dependencies
#include "TFile.h"
#include "TTree.h"
#include "TDatime.h"
#include "TDirectoryFile.h"

//local dependencies
#include "include/checkMakeDir.h"
#include "include/mntToXRootdFileString.h"
#include "include/returnRootFileContentsList.h"

int duplicateRemoval(const std::string inFileName, std::vector<std::string> ttreeSelection, const Int_t nEvtToKeep = -1)
{
  if(inFileName.size() == 0){
    std::cout << "Given inFileName \'" << inFileName << "\' is invalid. return 1." << std::endl;
    return 1;
  }
  if(!checkFile(inFileName) || inFileName.find(".root") == std::string::npos){
    std::cout << "Given inFileName \'" << inFileName << "\' is invalid. return 1." << std::endl;
    return 1;
  }

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);

  TFile* inFile_p = TFile::Open(mntToXRootdFileString(inFileName).c_str(), "READ");
  std::vector<std::string> listOfTrees = returnRootFileContentsList(inFile_p, "TTree");
  inFile_p->Close();
  delete inFile_p;

  unsigned int pos = 0;
  while(ttreeSelection.size() > pos){
    bool isGoodTTreeSelect = false;
    for(unsigned int tI = 0; tI < listOfTrees.size(); ++tI){
      if(isStrSame(ttreeSelection.at(pos), listOfTrees.at(tI))){
	isGoodTTreeSelect = true;
	break;
      }
    }

    if(isGoodTTreeSelect) ++pos;
    else{
      std::cout << "WARNING: Given ttree to select, \'" << ttreeSelection.at(pos) << "\', is not found in file \'" << inFileName << "\'. Removing from selection list" << std::endl;
      ttreeSelection.erase(ttreeSelection.begin()+pos);
    }
  }

  if(ttreeSelection.size() == 0 && nEvtToKeep < 1){
    std::cout << "WARNING: No valid ttree selected and also no events selected for skimming. Would just duplicate file. terminate job, return 1" << std::endl;
    return 1;
  }

  if(ttreeSelection.size() != 0) listOfTrees = ttreeSelection;

  std::string nEvtKeepStr = "nEvtAll";
  if(nEvtToKeep >= 0) nEvtKeepStr = "nEvt" + std::to_string(nEvtToKeep);

  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  if(outFileName.find(".") != std::string::npos) outFileName = outFileName.substr(0, outFileName.find("."));
  outFileName = "output/" + dateStr + "/" + outFileName + "_TTreeSkim_" + nEvtKeepStr + "_" + dateStr + ".root";

  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  const Int_t nTrees = listOfTrees.size();
  TDirectoryFile* outDirs_p[nTrees];
  TTree* outTrees_p[nTrees];

  std::cout << "IGNORE FOLLOWING CD ERRORS - HANDLED CORRECTLY" << std::endl;
  for(unsigned int tI = 0; tI < listOfTrees.size(); ++tI){
    std::string dirName = listOfTrees.at(tI).substr(0, listOfTrees.at(tI).find("/"));
    if(outFile_p->cd(dirName.c_str())){
      for(unsigned int tI2 = 0; tI2  < tI; ++tI2){
	if(isStrSame(dirName, outDirs_p[tI2]->GetName())) outDirs_p[tI] = outDirs_p[tI2];
      }
    }
    else outDirs_p[tI] = (TDirectoryFile*)outFile_p->mkdir(dirName.c_str());
  }


  inFile_p = TFile::Open(mntToXRootdFileString(inFileName).c_str(), "READ");
  
  TTree* trees_p[nTrees];
  Int_t treeNEntries[nTrees];
  Int_t maxNEntries = -1;

  for(Int_t tI = 0; tI < nTrees; ++tI){
    trees_p[tI] = (TTree*)inFile_p->Get(listOfTrees.at(tI).c_str());
    treeNEntries[tI] = trees_p[tI]->GetEntries();
    maxNEntries = TMath::Max(maxNEntries, treeNEntries[tI]);
    std::string dirName = listOfTrees.at(tI).substr(0, listOfTrees.at(tI).find("/"));
    outFile_p->cd(dirName.c_str());
    outTrees_p[tI] = trees_p[tI]->CloneTree(0);
  }

  Int_t nEvtToKeepTemp = nEvtToKeep;
  if(nEvtToKeep < 1)  nEvtToKeepTemp = maxNEntries;
  const Int_t nEntries = TMath::Min(maxNEntries, nEvtToKeepTemp);
  const Int_t nDiv = TMath::Max(1, nEntries/20);

  std::cout << "Processing " << nEntries << " events..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << "..." << std::endl;

    for(Int_t tI = 0; tI < nTrees; ++tI){
      if(entry < treeNEntries[tI]) trees_p[tI]->GetEntry(entry);
    }
    
    for(Int_t tI = 0; tI < nTrees; ++tI){
      if(entry < treeNEntries[tI]) outTrees_p[tI]->Fill();
    }    
  }

  inFile_p->Close();
  delete inFile_p;

  outFile_p->cd();
  
  for(Int_t tI = 0; tI < nTrees; ++tI){
    std::string dirName = listOfTrees.at(tI).substr(0, listOfTrees.at(tI).find("/"));
    outFile_p->cd(dirName.c_str());
    outTrees_p[tI]->Write("", TObject::kOverwrite);
    delete outTrees_p[tI];
  }

  outFile_p->Close();
  delete outFile_p;


  return 0;
}


int main(int argc, char* argv[])
{
  if(argc != 3 && argc != 4){
    std::cout << "Usage: ./bin/duplicateRemoval.exe <inFileName> <commaSeparatedTreeList> <opt-nEvtToKeep>. return 1" << std::endl;
    return 1;
  }

  std::string argv2Str = std::string(argv[2]) + ",";
  while(argv2Str.find(",,") != std::string::npos){argv2Str.replace(argv2Str.find(",,"), 2, ",");}
  std::vector<std::string> argv2Vect;

  if(argv2Str.size() > 1){
    while(argv2Str.find(",") != std::string::npos){
      argv2Vect.push_back(argv2Str.substr(0, argv2Str.find(",")));
      argv2Str.replace(0, argv2Str.find(",")+1, "");
    }
  }

  int retVal = 0;
  if(argc == 3) retVal += duplicateRemoval(argv[1], argv2Vect);
  else retVal += duplicateRemoval(argv[1], argv2Vect, std::stoi(argv[3]));
  return retVal;
}
