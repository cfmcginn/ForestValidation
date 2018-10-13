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
#include "include/returnRootFileContentsList.h"
#include "include/runLumiEventKey.h"

int duplicateRemoval(const std::string inFileName)
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

  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  std::vector<std::string> listOfTrees = returnRootFileContentsList(inFile_p, "TTree");
  const Int_t nTrees = listOfTrees.size();
  inFile_p->Close();
  delete inFile_p;

  Int_t hiEvtPos = -1;
  for(Int_t tI = 0; tI < nTrees; ++tI){
    if(listOfTrees.at(tI).find("hiEvtAnalyzer") != std::string::npos){
      hiEvtPos = tI;
      break;
    }
  }

  if(hiEvtPos < 0){
    std::cout << "No tree for run lumi evt... return 1" << std::endl;
    return 1;
  }

  UInt_t run_, lumi_;
  ULong64_t evt_;
  


  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  outFileName = "output/" + dateStr + "/" + outFileName + "_RemoveDuplicates_" + dateStr + ".root";

  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TDirectoryFile* outDirs_p[nTrees];
  TTree* outTrees_p[nTrees];
  for(unsigned int tI = 0; tI < listOfTrees.size(); ++tI){
    std::string dirName = listOfTrees.at(tI).substr(0, listOfTrees.at(tI).find("/"));
    if(outFile_p->cd(dirName.c_str())){
      for(unsigned int tI2 = 0; tI2  < tI; ++tI2){
	if(isStrSame(dirName, outDirs_p[tI2]->GetName())) outDirs_p[tI] = outDirs_p[tI2];
      }
    }
    else outDirs_p[tI] = (TDirectoryFile*)outFile_p->mkdir(dirName.c_str());
  }


  inFile_p = new TFile(inFileName.c_str(), "READ");
  
  TTree* trees_p[nTrees];

  for(Int_t tI = 0; tI < nTrees; ++tI){
    trees_p[tI] = (TTree*)inFile_p->Get(listOfTrees.at(tI).c_str());
    std::string dirName = listOfTrees.at(tI).substr(0, listOfTrees.at(tI).find("/"));
    outFile_p->cd(dirName.c_str());
    outTrees_p[tI] = trees_p[tI]->CloneTree(0);
  }
  
  trees_p[hiEvtPos]->SetBranchAddress("run", &run_);
  trees_p[hiEvtPos]->SetBranchAddress("lumi", &lumi_);
  trees_p[hiEvtPos]->SetBranchAddress("evt", &evt_);
  
  std::map<ULong64_t, Int_t> runLumiEvtToEntry;

  const Int_t nEntries = trees_p[0]->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/20);

  Int_t removed = 0;

  std::cout << "Processing " << nEntries << " events..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << "..." << std::endl;

    trees_p[hiEvtPos]->GetEntry(entry);

    ULong64_t key = keyFromRunLumiEvent(run_, lumi_, evt_);
    if(runLumiEvtToEntry.count(key) != 0){
      removed++;
      continue;
    }

    for(Int_t tI = 0; tI < nTrees; ++tI){
      if(tI == hiEvtPos) continue;
      trees_p[tI]->GetEntry(entry);
    }

    
    runLumiEvtToEntry[key] = entry;

    for(Int_t tI = 0; tI < nTrees; ++tI){
      outTrees_p[tI]->Fill();
    }    
  }

  std::cout << "Removed " << removed << "/" << nEntries << " duplicates" << std::endl;

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
  if(argc != 2){
    std::cout << "Usage: ./bin/duplicateRemoval.exe <inFileName>. return 1" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += duplicateRemoval(argv[1]);
  return retVal;
}
