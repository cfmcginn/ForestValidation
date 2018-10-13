//cpp dependencies
#include <iostream>
#include <string>

//ROOT depdencies
#include "TFile.h"
#include "TTree.h"

//cpp dependencies
#include "include/checkMakeDir.h"
#include "include/runLumiEventKey.h"

int checkDuplicateEvents(const std::string inFileName)
{
  if(inFileName.size() == 0){
    std::cout << "Given inFileName \'" << inFileName << "\' is invalid. return 1." << std::endl;
    return 1;
  }
  if(!checkFile(inFileName) || inFileName.find(".root") == std::string::npos){
    std::cout << "Given inFileName \'" << inFileName << "\' is invalid. return 1." << std::endl;
    return 1;
  }

  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  TTree* hiTree_p = (TTree*)inFile_p->Get("hiEvtAnalyzer/HiTree");

  UInt_t run_, lumi_;
  ULong64_t evt_;

  hiTree_p->SetBranchStatus("*", 0);
  hiTree_p->SetBranchStatus("run", 1);
  hiTree_p->SetBranchStatus("lumi", 1);
  hiTree_p->SetBranchStatus("evt", 1);

  hiTree_p->SetBranchAddress("run", &run_);
  hiTree_p->SetBranchAddress("lumi", &lumi_);
  hiTree_p->SetBranchAddress("evt", &evt_);

  const Int_t nEntries = hiTree_p->GetEntries();

  std::map<ULong64_t, Int_t> runLumiEventToEntry;


  Int_t keyCount = 0;
  bool noRepeat = true;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    hiTree_p->GetEntry(entry);

    ULong64_t key = keyFromRunLumiEvent(run_, lumi_, evt_);

    if(runLumiEventToEntry.count(key) != 0){
      if(keyCount < 100) std::cout << "WARNING REPEAT KEY!!!!!" << std::endl;
      ++keyCount;
      noRepeat = false;
    }
    else runLumiEventToEntry[key] = entry;
  }
  if(!noRepeat){
    std::cout << "inFileName \'" << inFileName << "\' contains duplicate run,lumi,event. Consider invalid" << std::endl;
  }

  inFile_p->Close();
  delete inFile_p;

  return 0;
}


int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/checkDuplicateEvents.exe <inFileName>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += checkDuplicateEvents(argv[1]);
  return retVal;
}
