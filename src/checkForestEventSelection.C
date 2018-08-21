//cpp dependencies
#include <iostream>
#include <string>

//ROOT dependencies
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/mntToXRootdFileString.h"

int checkEventSelection(const std::string inFileName)
{
  if(!checkFile(inFileName) || inFileName.find(".root") == std::string::npos){
    std::cout << "Given \'" << inFileName << "\' is not a valid root file. return 1" << std::endl;
    return 1;
  }

  TFile* inFile_p = TFile::Open(mntToXRootdFileString(inFileName).c_str(), "READ");
  TTree* hiTree_p = (TTree*)inFile_p->Get("hiEvtAnalyzer/HiTree");
  TTree* skimTree_p = (TTree*)inFile_p->Get("skimanalysis/HltTree");

  Float_t vz_;
  Float_t hiHF_;

  hiTree_p->SetBranchStatus("*", 0);
  hiTree_p->SetBranchStatus("vz", 1);
  hiTree_p->SetBranchStatus("hiHF", 1);
  
  hiTree_p->SetBranchStatus("vz", &vz_);
  hiTree_p->SetBranchStatus("hiHF", &hiHF_);

  //  Int_t pcollisionEventSelection;
  Int_t HBHENoiseFilterResultRun2Loose_;
  Int_t pprimaryVertexFilter_;
  Int_t phfCoincFilter3_;
  Int_t pclusterCompatibilityFilter_;

  skimTree_p->SetBranchStatus("*", 0);
  //  skimTree_p->SetBranchStatus("pcollisionEventSelection", 1);
  skimTree_p->SetBranchStatus("HBHENoiseFilterResultRun2Loose", 1);
  skimTree_p->SetBranchStatus("pprimaryVertexFilter", 1);
  skimTree_p->SetBranchStatus("phfCoincFilter3", 1);
  skimTree_p->SetBranchStatus("pclusterCompatibilityFilter", 1);
  
  //  skimTree_p->SetBranchStatus("pcollisionEventSelection", &pcollisionEventSelection_);
  skimTree_p->SetBranchStatus("HBHENoiseFilterResultRun2Loose", &HBHENoiseFilterResultRun2Loose_);
  skimTree_p->SetBranchStatus("pprimaryVertexFilter", &pprimaryVertexFilter_);
  skimTree_p->SetBranchStatus("phfCoincFilter3", &phfCoincFilter3_);
  skimTree_p->SetBranchStatus("pclusterCompatibilityFilter", &pclusterCompatibilityFilter_);

  const Int_t nEntries = hiTree_p->GetEntries();

  Int_t vzCounter = 0;
  Int_t hiHFCounter = 0;
  //  Int_t pcollCounter = 0;
  Int_t hbheCounter = 0;
  Int_t pprimCounter = 0;
  Int_t phfCounter = 0;
  Int_t pclustCounter = 0;

  std::cout << "Processing " << nEntries << " events..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    hiTree_p->GetEntry(entry);
    skimTree_p->GetEntry(entry);

    if(TMath::Abs(vz_) > 15.) continue;
    ++vzCounter;
    
    if(hiHF_ > 5500.) continue;
    ++hiHFCounter;
    
    if(HBHENoiseFilterResultRun2Loose_ == 0) continue;
    ++hbheCounter;

    if(pprimaryVertexFilter_ == 0) continue;
    ++pprimCounter;

    if(phfCoincFilter3_ == 0) continue;
    ++phfCounter;

    if(pclusterCompatibilityFilter_ == 0) continue;
    ++pclustCounter;
  }

  std::cout << "Total Entries: " << nEntries << std::endl;
  std::cout << " Passing vz: " << vzCounter << std::endl;
  std::cout << " Passing hiHF+prev: " << hiHFCounter << std::endl;
  std::cout << " Passing hbhe+prev: " << hbheCounter << std::endl;
  std::cout << " Passing pprim+prev: " << pprimCounter << std::endl;
  std::cout << " Passing phf+prev: " << phfCounter << std::endl;
  std::cout << " Passing pclust+prev: " << pclustCounter << std::endl;

  inFile_p->Close();
  delete inFile_p;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/checkEventSelection.exe <inFileName>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += checkEventSelection(argv[1]);
  return retVal;
}
