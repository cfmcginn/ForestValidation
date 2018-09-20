//cpp dependencies
#include <iostream>
#include <string>
#include <vector>

//ROOT dependencies
#include "TFile.h"
#include "TTree.h"

//Local dependencies
#include "include/mntToXRootdFileString.h"
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"

int checkAllTreesReadable(const std::string inFileName)
{
  std::cout << "TESTING: " << mntToXRootdFileString(inFileName) << std::endl;
  TFile* inFile_p = TFile::Open(mntToXRootdFileString(inFileName).c_str(), "READ");

  if(inFile_p->IsZombie()){
    std::cout << "Open failed. return 1" << std::endl;
    return 1;
  }
  else std::cout << "Successfully opened file \'" << mntToXRootdFileString(inFileName) << "\'" << std::endl;

  std::vector<std::string> fileTrees = returnRootFileContentsList(inFile_p, "TTree");

  //remove duplicates
  unsigned int pos = 0;
  while(pos < fileTrees.size()){
    bool isGood = true;
    for(unsigned int i = pos+1; i < fileTrees.size(); ++i){
      if(isStrSame(fileTrees.at(pos), fileTrees.at(i))){
	fileTrees.erase(fileTrees.begin()+i);
	isGood = false;
	break;
      }
    }

    if(isGood) ++pos;
  }

  const Int_t nTrees = fileTrees.size();
  TTree* trees_p[nTrees];

  std::cout << "File \'" << inFileName << "\' contains " << nTrees << " trees..." << std::endl;
  
  for(Int_t i = 0; i < nTrees; ++i){
    std::cout << " TTree " << i << "/" << fileTrees.size() << ": " << fileTrees.at(i) << std::endl;
    trees_p[i]=NULL;
    trees_p[i] = (TTree*)inFile_p->Get(fileTrees.at(i).c_str());
  }

  std::cout << "Checking all entries" << std::endl;
  for(Int_t i = 0; i < nTrees; ++i){
    std::cout << " Processing " << i << "/" << nTrees << ": " << fileTrees.at(i) << std::endl;
    const Int_t nEntries = trees_p[i]->GetEntries();
  
    for(Int_t entry = 0; entry < nEntries; ++entry){
      trees_p[i]->GetEntry(entry);
    }
  }
  
  std::cout << "Check complete" << std::endl;

  inFile_p->Close();
  delete inFile_p;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: checkAllTreesReadable.exe <inFileName>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += checkAllTreesReadable(argv[1]);
  return retVal;
}
