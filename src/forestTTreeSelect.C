//cpp dependencies
#include <iostream>
#include <map>
#include <string>

//ROOT dependencies
#include "TDatime.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TTree.h"

//local dependencies
#include "include/checkMakeDir.h"
#include "include/mntToXRootdFileString.h"
#include "include/returnRootFileContentsList.h"
#include "include/runLumiEventKey.h"

int forestTTreeSelect(const std::string inFileName, std::vector<std::string> ttreeSelection, const Int_t nEvtToKeep = -1, const Int_t nEvtToStart = 0, const std::string commaSeparatedRLEList = "", std::string hltBranchSel = "", std::string commaSepCuts = "")
{
  if(inFileName.size() == 0){
    std::cout << "Given inFileName \'" << inFileName << "\' is invalid. return 1." << std::endl;
    return 1;
  }
    
  std::vector<std::string> cutVar;
  std::vector<std::string> cutSign;
  std::vector<std::string> cutVal;
  std::vector<int> cutVarTreePos;
  std::vector<std::string> cutVarStr;

  if(commaSepCuts.size() != 0){
    if(commaSepCuts.substr(commaSepCuts.size()-1, 1).find(",") == std::string::npos) commaSepCuts = commaSepCuts + ",";

    while(commaSepCuts.find(",") != std::string::npos){
      std::string tempStr = commaSepCuts.substr(0, commaSepCuts.find(","));
      while(tempStr.find(" ") != std::string::npos){
	tempStr.replace(tempStr.find(" "), 1, "");
      }
      
      std::string tempCutVar = "";
      std::string tempCutSign = "";
      std::string tempCutVal = "";
      std::string tempCutVarStr = "";

      if(tempStr.find("==") != std::string::npos){
	tempCutSign = "==";
	tempCutVarStr = "EQ";
      }
      else if(tempStr.find("!=") != std::string::npos){
	tempCutSign = "!=";
	tempCutVarStr = "NE";
      }
      else if(tempStr.find(">=") != std::string::npos){
	tempCutSign = ">=";
	tempCutVarStr = "GE";
      }
      else if(tempStr.find(">") != std::string::npos){
	tempCutSign = ">";
	tempCutVarStr = "GT";
      }
      else if(tempStr.find("<=") != std::string::npos){
	tempCutSign = "<=";
	tempCutVarStr = "LE";
      }
      else if(tempStr.find("<") != std::string::npos){
	tempCutSign = "<";
	tempCutVarStr = "LT";
      }
      
      if(tempCutSign.size() == 0){
	std::cout << "Cut \'" << tempStr << "\' contains no valid comparison. continue" << std::endl;
      }
      else{
	tempCutVar = tempStr.substr(0, tempStr.find(tempCutSign));
	tempStr.replace(0, tempStr.find(tempCutSign)+tempCutSign.size(), "");
	tempCutVal = tempStr;
	
	cutVar.push_back(tempCutVar);
	cutSign.push_back(tempCutSign);
	cutVal.push_back(tempCutVal);
	cutVarTreePos.push_back(-1);
	
	cutVarStr.push_back(tempCutVar + tempCutVarStr + tempCutVal);
      }
      
      commaSepCuts.replace(0, commaSepCuts.find(",")+1, "");
    }
  }

  std::vector<int> varTreePos;

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);

  bool doSpecialHLT = hltBranchSel.size() != 0;
  TFile* inFile_p = TFile::Open(mntToXRootdFileString(inFileName).c_str(), "READ");
  std::vector<std::string> listOfTrees = returnRootFileContentsList(inFile_p, "TTree");

  std::string hltTreeName = "";
  std::string hiTreeName = "";
  int hltTreePos = -1;
  int hiTreePos = -1;

  std::map<std::string, bool> hltBranchesMap;

  for(auto const& list : listOfTrees){
    if(list.find("hiEvtAna") != std::string::npos && list.find("HiTree") != std::string::npos) hiTreeName = list;
    if(list.find("hlt") != std::string::npos && list.find("HltTree") != std::string::npos){
      hltTreeName = list;

      if(hltBranchSel.size() != 0){
	TTree* hltTree_p = (TTree*)inFile_p->Get(list.c_str());
	TObjArray* arr = (TObjArray*)hltTree_p->GetListOfBranches();

	for(Int_t aI = 0; aI < arr->GetEntries(); ++aI){
	  std::string currStr = arr->At(aI)->GetName();
	  hltBranchesMap[currStr] = true;
	}
      }
    }
  }

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

  if(ttreeSelection.size() == 0 && nEvtToKeep < 1 && commaSeparatedRLEList.size() == 0){
    std::cout << "WARNING: No valid ttree selected and also no events selected for skimming. Would just duplicate file. terminate job, return 1" << std::endl;
    return 1;
  }

  std::map<unsigned long long, bool> rleMap;
  if(commaSeparatedRLEList.size() != 0){
    std::string commaSeparatedRLEListCopy = commaSeparatedRLEList;
    if(commaSeparatedRLEListCopy.substr(commaSeparatedRLEListCopy.size()-1,1).find(",") == std::string::npos) commaSeparatedRLEListCopy = commaSeparatedRLEListCopy + ",";

    while(commaSeparatedRLEListCopy.find(",") != std::string::npos){
      std::string rleStr = commaSeparatedRLEListCopy.substr(0,commaSeparatedRLEListCopy.find(","));
      UInt_t run = (UInt_t)(std::stoi(rleStr.substr(0, rleStr.find(":"))));
      rleStr.replace(0, rleStr.find(":")+1, "");
      UInt_t lumi = (UInt_t)(std::stoi(rleStr.substr(0, rleStr.find(":"))));
      rleStr.replace(0, rleStr.find(":")+1, "");
      ULong64_t evt = (ULong64_t)(std::stol(rleStr));

      ULong64_t key = keyFromRunLumiEvent(run,lumi,evt);
      rleMap[key] = true;

      commaSeparatedRLEListCopy.replace(0, commaSeparatedRLEListCopy.find(",")+1, "");
    }
  }

  if(ttreeSelection.size() != 0) listOfTrees = ttreeSelection;

  std::string nEvtKeepStr = "nEvtAll";
  if(nEvtToKeep >= 0) nEvtKeepStr = "nEvt" + std::to_string(nEvtToKeep);

  std::string nEvtStartStr = "nEvtStart0";
  if(nEvtToStart >= 0) nEvtStartStr = "nEvtStart" + std::to_string(nEvtToStart);

  std::string rleStr = "NoRLESkim";
  if(commaSeparatedRLEList.size() != 0) rleStr = "RLESkim";

  std::string cutStr = "NoCut";
  if(cutVar.size() != 0){
    cutStr = "Cut";
    for(int cI = 0; cI < TMath::Min(2, (int)cutVar.size()); ++cI){
      cutStr = cutStr + "_" + cutVarStr.at(cI);
    }
    while(cutStr.find(".") != std::string::npos){
      cutStr.replace(cutStr.find("."), 1, "p");
    }
    while(cutStr.find("-") != std::string::npos){
      cutStr.replace(cutStr.find("-"), 1, "Neg");
    }
  }

  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  if(outFileName.find(".") != std::string::npos) outFileName = outFileName.substr(0, outFileName.find("."));
  outFileName = "output/" + dateStr + "/" + outFileName + "_TTreeSkim_" + nEvtKeepStr + "_" + nEvtStartStr + "_" + rleStr  + "_" + cutStr + "_" + dateStr + ".root";

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

  Int_t runHLT_, lumiHLT_;
  ULong64_t eventHLT_;

  std::vector<std::string> hltBranchesToKeep;
  std::vector<Int_t> hltBranchVals;
  if(doSpecialHLT){
    if(hltBranchSel.substr(hltBranchSel.size()-1).find(",") == std::string::npos) hltBranchSel = hltBranchSel + ",";

    while(hltBranchSel.find(",") != std::string::npos){
      std::string branchStr = hltBranchSel.substr(0, hltBranchSel.find(","));
      if(hltBranchesMap.count(branchStr) != 0){
	hltBranchesToKeep.push_back(branchStr);
	hltBranchVals.push_back(0);
      }
      else std::cout << "WARNING: Requested branch \'" << branchStr << "\' is not found in TTree" << std::endl;

      hltBranchSel.replace(0, hltBranchSel.find(",")+1, "");
    }
  }

  UInt_t runHI_, lumiHI_;
  ULong64_t eventHI_;  

  TTree* trees_p[nTrees];
  TTree* hltTree_p = NULL;
  TTree* hiTree_p = NULL;
  Int_t treeNEntries[nTrees];
  Int_t maxNEntries = -1;

  for(Int_t tI = 0; tI < nTrees; ++tI){
    if(isStrSame(hltTreeName, listOfTrees.at(tI))) hltTreePos = tI;
    else if(isStrSame(hiTreeName, listOfTrees.at(tI))) hiTreePos = tI;

    trees_p[tI] = (TTree*)inFile_p->Get(listOfTrees.at(tI).c_str());
    treeNEntries[tI] = trees_p[tI]->GetEntries();
    maxNEntries = TMath::Max(maxNEntries, treeNEntries[tI]);
    std::string dirName = listOfTrees.at(tI).substr(0, listOfTrees.at(tI).find("/"));
    std::string treeName = listOfTrees.at(tI);
    treeName.replace(0, treeName.find("/")+1, "");
    outFile_p->cd(dirName.c_str());

    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
      if(cutVarTreePos.at(cI) >= 0) continue;

      TObjArray* branches = (TObjArray*)trees_p[tI]->GetListOfBranches();
      for(Int_t bI = 0; bI < branches->GetEntries(); ++bI){
	std::string branch = branches->At(bI)->GetName();
	if(isStrSame(branch, cutVar.at(cI))){
	  std::cout << "CUT VAR FOUND: " << cutVar.at(cI) << ", " << branch << ", " << listOfTrees.at(tI) << std::endl;
	  cutVarTreePos.at(cI) = tI;
	  break;
	}
      }
    }
    
    if(tI != hltTreePos || !doSpecialHLT) outTrees_p[tI] = trees_p[tI]->CloneTree(0);
    else{
      //      trees_p[tI]->SetBranchStatus("*", 0);
      trees_p[tI]->SetBranchStatus("Run", 1);
      trees_p[tI]->SetBranchStatus("LumiBlock", 1);
      trees_p[tI]->SetBranchStatus("Event", 1);

      trees_p[tI]->SetBranchAddress("Run", &runHLT_);
      trees_p[tI]->SetBranchAddress("LumiBlock", &lumiHLT_);
      trees_p[tI]->SetBranchAddress("Event", &eventHLT_);

      for(unsigned int bI = 0; bI < hltBranchesToKeep.size(); ++bI){
	trees_p[tI]->SetBranchStatus(hltBranchesToKeep.at(bI).c_str(), 1);

	trees_p[tI]->SetBranchAddress(hltBranchesToKeep.at(bI).c_str(), &(hltBranchVals.at(bI)));
      }

      outTrees_p[tI] = new TTree(treeName.c_str(), "");

      outTrees_p[tI]->Branch("Run", &runHLT_, "Run/I");
      outTrees_p[tI]->Branch("LumiBlock", &lumiHLT_, "LumiBlock/I");
      outTrees_p[tI]->Branch("Event", &eventHLT_, "Event/l");

      for(unsigned int bI = 0; bI < hltBranchesToKeep.size(); ++bI){
	outTrees_p[tI]->Branch(hltBranchesToKeep.at(bI).c_str(), &(hltBranchVals.at(bI)), (hltBranchesToKeep.at(bI) + "/I").c_str());
      }
    }
  }

  if(hiTreeName.size() != 0){
    if(hiTreePos < 0){
      hiTree_p = (TTree*)inFile_p->Get(hiTreeName.c_str());

      //      hiTree_p->SetBranchStatus("*", 0);
      hiTree_p->SetBranchStatus("run", 1);
      hiTree_p->SetBranchStatus("lumi", 1);
      hiTree_p->SetBranchStatus("evt", 1);

      hiTree_p->SetBranchAddress("run", &runHI_);
      hiTree_p->SetBranchAddress("lumi", &lumiHI_);
      hiTree_p->SetBranchAddress("evt", &eventHI_);
    }
    else{
      trees_p[hiTreePos]->SetBranchAddress("run", &runHI_);
      trees_p[hiTreePos]->SetBranchAddress("lumi", &lumiHI_);
      trees_p[hiTreePos]->SetBranchAddress("evt", &eventHI_);
    }
  }
  else if(hltTreeName.size() != 0){
    if(hltTreePos < 0){
      hltTree_p = (TTree*)inFile_p->Get(hltTreeName.c_str());

      //      hltTree_p->SetBranchStatus("*", 0);
      hltTree_p->SetBranchStatus("Run", 1);
      hltTree_p->SetBranchStatus("LumiBlock", 1);
      hltTree_p->SetBranchStatus("Event", 1);

      hltTree_p->SetBranchAddress("Run", &runHLT_);
      hltTree_p->SetBranchAddress("LumiBlock", &lumiHLT_);
      hltTree_p->SetBranchAddress("Event", &eventHLT_);
    }
    else{
      trees_p[hltTreePos]->SetBranchAddress("Run", &runHLT_);
      trees_p[hltTreePos]->SetBranchAddress("LumiBlock", &lumiHLT_);
      trees_p[hltTreePos]->SetBranchAddress("Event", &eventHLT_);
    }
  }
  
  Int_t nEvtToStartTemp = nEvtToStart;
  if(nEvtToStartTemp < 0) nEvtToStartTemp = 0;
  else if(nEvtToStartTemp >= maxNEntries) nEvtToStartTemp = maxNEntries-1;

  const Int_t startEntry = nEvtToStartTemp;

  Int_t nEvtToKeepTemp = nEvtToKeep;
  if(nEvtToKeep < 1)  nEvtToKeepTemp = maxNEntries;
  const Int_t nEntries = TMath::Min(maxNEntries, nEvtToKeepTemp + startEntry);
  const Int_t nDiv = TMath::Max(1, (nEntries - startEntry)/20);
  
  
  std::cout << "Processing " << nEntries << " events..." << std::endl;
  for(Int_t entry = startEntry; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << "..." << std::endl;

    if(commaSeparatedRLEList.size() != 0){
      if(hiTreeName.size() != 0){
	if(hiTreePos < 0) hiTree_p->GetEntry(entry);
	else trees_p[hiTreePos]->GetEntry(entry);
	
	ULong64_t key = keyFromRunLumiEvent((UInt_t)runHI_, (UInt_t)lumiHI_, (ULong64_t)eventHI_);	  
	if(rleMap.count(key) == 0) continue;
      }
      else if(hltTreeName.size() != 0){
	if(hltTreePos < 0) hltTree_p->GetEntry(entry);
	else trees_p[hltTreePos]->GetEntry(entry);
	
	ULong64_t key = keyFromRunLumiEvent((UInt_t)runHLT_, (UInt_t)lumiHLT_, (ULong64_t)eventHLT_);	  
	if(rleMap.count(key) == 0) continue;
      }
    }

    bool keepEvent = true;
    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
      if(cutVarTreePos.at(cI) < 0) continue;

      trees_p[cutVarTreePos.at(cI)]->GetEntry(entry);
      
      if(isStrSame(cutSign.at(cI), "==")){
	keepEvent = keepEvent && std::stoi(cutVal.at(cI)) == trees_p[cutVarTreePos.at(cI)]->GetLeaf(cutVar.at(cI).c_str())->GetValue();
      }
      else if(isStrSame(cutSign.at(cI), "!=")){
	keepEvent = keepEvent && std::stoi(cutVal.at(cI)) != trees_p[cutVarTreePos.at(cI)]->GetLeaf(cutVar.at(cI).c_str())->GetValue();
      }
      else if(isStrSame(cutSign.at(cI), ">=")){
	keepEvent = keepEvent && trees_p[cutVarTreePos.at(cI)]->GetLeaf(cutVar.at(cI).c_str())->GetValue() >= std::stod(cutVal.at(cI));
      }
      else if(isStrSame(cutSign.at(cI), ">")){
	keepEvent = keepEvent &&  trees_p[cutVarTreePos.at(cI)]->GetLeaf(cutVar.at(cI).c_str())->GetValue() > std::stod(cutVal.at(cI));
      }
      else if(isStrSame(cutSign.at(cI), "<=")){
	keepEvent = keepEvent &&  trees_p[cutVarTreePos.at(cI)]->GetLeaf(cutVar.at(cI).c_str())->GetValue() <= std::stod(cutVal.at(cI));
      }
      else if(isStrSame(cutSign.at(cI), "<")){
	keepEvent = keepEvent && trees_p[cutVarTreePos.at(cI)]->GetLeaf(cutVar.at(cI).c_str())->GetValue() < std::stod(cutVal.at(cI));
      }
    }

    if(!keepEvent) continue;

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
    outFile_p->cd();
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
  if(argc < 3 || argc > 8){
    std::cout << "Usage: ./bin/forestTTreeSelect.exe <inFileName> <commaSeparatedTreeList> <opt-nEvtToKeep> <opt-nEvtToStart> <commaSeparatedRLEList> <hltBranchSel> <opt-commaSepCuts>. return 1" << std::endl;
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

  std::cout << "Input args: " << std::endl;
  for(Int_t aI = 1; aI < argc; ++aI){
    std::cout << "argv[" << aI << "]: " << argv[aI] << std::endl;
  }

  int retVal = 0;
  if(argc == 3) retVal += forestTTreeSelect(argv[1], argv2Vect);
  else if(argc == 4) retVal += forestTTreeSelect(argv[1], argv2Vect, std::stoi(argv[3]));
  else if(argc == 5) retVal += forestTTreeSelect(argv[1], argv2Vect, std::stoi(argv[3]), std::stoi(argv[4]));
  else if(argc == 6) retVal += forestTTreeSelect(argv[1], argv2Vect, std::stoi(argv[3]), std::stoi(argv[4]), argv[5]);
  else if(argc == 7) retVal += forestTTreeSelect(argv[1], argv2Vect, std::stoi(argv[3]), std::stoi(argv[4]), argv[5], argv[6]);
  else if(argc == 8) retVal += forestTTreeSelect(argv[1], argv2Vect, std::stoi(argv[3]), std::stoi(argv[4]), argv[5], argv[6], argv[7]);
  return retVal;
}
