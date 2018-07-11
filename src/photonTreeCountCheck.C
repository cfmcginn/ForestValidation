#include <iostream>
#include <string>

#include "TFile.h"
#include "TTree.h"

#include "include/checkMakeDir.h"

int photonTreeCountCheck(const std::string inFileName, const Int_t nEvt)
{
  if(inFileName.find(".root") == std::string::npos){
    std::cout << "inFileName \'" << inFileName << "\' is not root file. return 1" << std::endl;
    return 1;
  }
  if(!checkFile(inFileName)){
    std::cout << "inFileName \'" << inFileName << "\' is not valid file. return 1" << std::endl;
    return 1;
  }

  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  TTree* phoTree_p = (TTree*)inFile_p->Get("ggHiNtuplizer/EventTree");

  if(phoTree_p->GetEntries() != nEvt){
    std::cout << inFileName << ", " << phoTree_p->GetEntries() << ", " << nEvt << std::endl;
  }

  inFile_p->Close();
  delete inFile_p;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: ./bin/photonTreeCountCheck.exe <inFileName> <nEvt>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += photonTreeCountCheck(argv[1], std::stoi(argv[2]));
  return retVal;
}
