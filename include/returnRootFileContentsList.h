#ifndef RETURNROOTFILECONTENTSLIST_H
#define RETURNROOTFILECONTENTSLIST_H

//cpp dependencies
#include <string>
#include <vector>
#include <iostream>

//ROOT dependencies
#include "TFile.h"
#include "TList.h"
#include "TKey.h"
#include "TCollection.h"
#include "TDirectory.h"

//Local (Utility) dependencies
#include "include/cppWatch.h"
#include "include/doGlobalDebug.h"
#include "include/stringUtil.h"

const std::string tdirStr = "TDirectoryFile";

std::vector<std::string> returnTDirContentsList(TFile*, const std::string, const std::string, const std::string, const Int_t currentLayers, const Int_t nLayersCap, std::vector<std::string>* classList);

TList* returnTDirContentsList(TFile* inFile_p, const std::string dirName, const Int_t currentLayers, const Int_t nLayersCap);

std::vector<std::string> returnRootFileContentsList(TFile *inFile_p, const std::string classFilter = "", const std::string nameFilter = "", const Int_t nLayersCap = -1, std::vector<std::string>* classList = NULL)
{
  inFile_p->cd();

  std::vector<std::string> returnList;

  TIter next(inFile_p->GetListOfKeys());
  TKey* key;
  while((key=(TKey*)next())){
    const std::string name = key->GetName();
    const std::string className = key->GetClassName();
    
    if(isStrSame(className, tdirStr)){
      std::vector<std::string>* tempClassList = new std::vector<std::string>;
      std::vector<std::string> tempReturnList = returnTDirContentsList(inFile_p, name, classFilter, nameFilter, 0, nLayersCap, tempClassList);

      
      const Int_t nTempKeys = tempReturnList.size();
      for(Int_t tempIter = 0; tempIter < nTempKeys; tempIter++){
	returnList.push_back(tempReturnList.at(tempIter));
	if(classList != NULL) classList->push_back(tempClassList->at(tempIter));
      }

      delete tempClassList;
    }
    

    if(classFilter.size() != 0)
      if(className.size() != classFilter.size() || className.find(classFilter) == std::string::npos) continue;   
  
    if(nameFilter.size() != 0)
      if(name.find(nameFilter) == std::string::npos) continue;   
    
    returnList.push_back(name);
    if(classList != NULL) classList->push_back(className);
  }

  return returnList;
}


std::vector<std::string> returnTDirContentsList(TFile* inFile_p, const std::string dirName, const std::string classFilter, const std::string nameFilter, const Int_t currentLayers, const Int_t nLayersCap = -1, std::vector<std::string>* classList = NULL)
{
  inFile_p->cd();

  std::vector<std::string> returnList;

  if(nLayersCap > 0 && currentLayers+1 >= nLayersCap) return returnList;
  
  TDirectoryFile* dir_p = (TDirectoryFile*)inFile_p->Get(dirName.c_str());
  TIter next(dir_p->GetListOfKeys());
  TKey* key;
  while((key=(TKey*)next())){
    const std::string name = key->GetName();
    const std::string className = key->GetClassName();

    if(isStrSame(className, tdirStr)){
      std::vector<std::string>* tempClassList = new std::vector<std::string>;
      std::vector<std::string> tempReturnList = returnTDirContentsList(inFile_p, dirName + "/" + name, classFilter, nameFilter, currentLayers+1, nLayersCap, tempClassList);
      
      const Int_t nTempKeys = tempReturnList.size();
      for(Int_t tempIter = 0; tempIter < nTempKeys; tempIter++){
        returnList.push_back(tempReturnList.at(tempIter));
	if(classList != NULL) classList->push_back(tempClassList->at(tempIter));
      }

      delete tempClassList;
    }

    if(classFilter.size() != 0)
      if(className.size() != classFilter.size() || className.find(classFilter) == std::string::npos) continue;
    
    if(nameFilter.size() != 0)
      if(name.find(nameFilter) == std::string::npos && dirName.find(nameFilter) == std::string::npos) continue;


    returnList.push_back(dirName + "/" + name);
    if(classList != NULL) classList->push_back(className);
  }

  return returnList;
}



//TList versions
TList* returnRootFileContentsList(TFile *inFile_p, const Int_t nLayersCap = -1)
{
  inFile_p->cd();

  TList* returnList = inFile_p->GetListOfKeys();
  TIter next(returnList);
  TKey* key;
  while((key=(TKey*)next())){
    const std::string name = key->GetName();
    const std::string className = key->GetClassName();
    
    if(isStrSame(className, tdirStr)){
      TList* tempReturnList = returnTDirContentsList(inFile_p, name, 0, nLayersCap);
      if(tempReturnList == NULL) continue;
      TIter tempNext(tempReturnList);
      TObject* obj = NULL;

      while((obj = tempNext())){
	returnList->AddLast(obj);
      }
    }
  }

  return returnList;
}


TList* returnTDirContentsList(TFile* inFile_p, const std::string dirName, const Int_t currentLayers, const Int_t nLayersCap = -1)
{
  inFile_p->cd();

  TList* returnList = NULL;

  if(nLayersCap > 0 && currentLayers+1 >= nLayersCap) return returnList;
  
  TDirectoryFile* dir_p = (TDirectoryFile*)inFile_p->Get(dirName.c_str());
  returnList = dir_p->GetListOfKeys();
  TIter next(returnList);
  TKey* key;
  while((key=(TKey*)next())){
    const std::string name = key->GetName();
    const std::string className = key->GetClassName();

    if(isStrSame(className, tdirStr)){
      TList* tempReturnList = returnTDirContentsList(inFile_p, dirName + "/" + name, currentLayers+1, nLayersCap);
      if(tempReturnList == NULL) continue;

      TIter tempNext(tempReturnList);
      TObject* obj = NULL;

      while( (obj = tempNext()) ){
	returnList->AddLast(obj);
      }
    }
  }

  return returnList;
}

#endif
