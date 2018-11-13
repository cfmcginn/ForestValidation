//cpp dependencies
#include <iostream>
#include <string>

//ROOT depdencies
#include "TFile.h"
#include "TTree.h"

//cpp dependencies
#include "include/checkMakeDir.h"
#include "include/L1AnalysisEventDataFormat.h"
#include "include/mntToXRootdFileString.h"
#include "include/returnRootFileContentsList.h"
#include "include/runLumiEventKey.h"
#include "include/stringUtil.h"

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

  TFile* inFile_p = TFile::Open(mntToXRootdFileString(inFileName).c_str(), "READ");
  std::vector<std::string> listOfTrees = returnRootFileContentsList(inFile_p, "TTree");

  bool containsHITree = false;
  bool containsHLTTree = false;
  bool containsL1Tree = false;
  std::string hltTreeStr = "";

  std::cout << "Checking TTrees..." << std::endl;
  for(unsigned int tI = 0; tI < listOfTrees.size(); ++tI){
    std::cout << " " << tI << "/" << listOfTrees.size() << ": " << listOfTrees.at(tI) << std::endl;
    if(isStrSame(listOfTrees.at(tI), "hiEvtAnalyzer/HiTree")) containsHITree = true;
    else if(isStrSame(listOfTrees.at(tI), "hltanalysis/HltTre") || isStrSame(listOfTrees.at(tI), "hltbitanalysis/HltTree")){
      containsHLTTree = true;
      hltTreeStr = listOfTrees.at(tI);
    }
    else if(isStrSame(listOfTrees.at(tI), "l1EventTree/L1EventTree")){
      containsL1Tree = true;
      hltTreeStr = listOfTrees.at(tI);
    }
  }

  if(!containsHITree && !containsHLTTree && !containsL1Tree){
    std::cout << "No trees for event matching return 1" << std::endl;
    return 1;
  }

  TTree* hiTree_p = NULL;
  if(containsHITree) hiTree_p = (TTree*)inFile_p->Get("hiEvtAnalyzer/HiTree");
  else if(containsHLTTree || containsL1Tree) hiTree_p = (TTree*)inFile_p->Get(hltTreeStr.c_str());

  UInt_t runHI_, lumiHI_;
  ULong64_t evtHI_;

  Int_t runHLT_, lumiHLT_;
  ULong64_t evtHLT_;


  L1Analysis::L1AnalysisEventDataFormat* Event = new L1Analysis::L1AnalysisEventDataFormat();

  hiTree_p->SetBranchStatus("*", 0);
  if(containsHITree){
    hiTree_p->SetBranchStatus("run", 1);
    hiTree_p->SetBranchStatus("lumi", 1);
    hiTree_p->SetBranchStatus("evt", 1);
    
    hiTree_p->SetBranchAddress("run", &runHI_);
    hiTree_p->SetBranchAddress("lumi", &lumiHI_);
    hiTree_p->SetBranchAddress("evt", &evtHI_);
  }
  else if(containsHLTTree){
    hiTree_p->SetBranchStatus("Run", 1);
    hiTree_p->SetBranchStatus("LumiBlock", 1);
    hiTree_p->SetBranchStatus("Event", 1);
    
    hiTree_p->SetBranchAddress("Run", &runHLT_);
    hiTree_p->SetBranchAddress("LumiBlock", &lumiHLT_);
    hiTree_p->SetBranchAddress("Event", &evtHLT_);
  }
  else if(containsL1Tree){
    hiTree_p->SetBranchStatus("Event", 1);
    hiTree_p->SetBranchStatus("run", 1);
    hiTree_p->SetBranchStatus("lumi", 1);
    hiTree_p->SetBranchStatus("event", 1);

    hiTree_p->SetBranchAddress("Event", &Event);
  }

  UInt_t run_, lumi_;
  ULong64_t evt_;

  const Int_t nEntries = hiTree_p->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/20);

  std::map<ULong64_t, Int_t> runLumiEventToEntry;

  Int_t keyCount = 0;
  bool noRepeat = true;
  std::cout << "Processing " << nEntries << " entries..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << "..." << std::endl;
    hiTree_p->GetEntry(entry);

    if(containsHITree){
      run_ = runHI_;
      lumi_ = lumiHI_;
      evt_ = evtHI_;
    }
    else if(containsHLTTree){
      run_ = (UInt_t)runHLT_;
      lumi_ = (UInt_t)lumiHLT_;
      evt_ = evtHLT_;
    }
    else{
      run_ = (UInt_t)Event->run;
      lumi_ = (UInt_t)Event->lumi;
      evt_ = (ULong64_t)Event->event;
    }

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
  else std::cout << "NO DUPLICATES" << std::endl;

  if(containsL1Tree) delete Event;

  inFile_p->Close();
  delete inFile_p;

  std::cout << "Total duplicated events: " << keyCount << std::endl;
  std::cout << "Job complete!" << std::endl;

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
