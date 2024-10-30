///AUTHOR: CFMCGINN (2018.04.12)
///For validating forest
//cpp dependencies
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//ROOT dependencies
#include "TCanvas.h"
#include "TDatime.h"
#include "TEnv.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TLeaf.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMath.h"
#include "TObjArray.h"
#include "TPad.h"
#include "TStyle.h"
#include "TTree.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/cppWatch.h"
#include "include/getLinBins.h"
#include "include/getLogBins.h"
#include "include/histDefUtility.h"
#include "include/kirchnerPalette.h"
#include "include/mntToXRootdFileString.h"
#include "include/plotUtilities.h"
#include "include/removeVectorDuplicates.h"
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"

bool getDoLog(double minVal, double maxVal, int orderMag)
{
  if(minVal<=0) return false;
  int order = 0;
  while(minVal < maxVal){
    minVal *= 10.;
    order++;
  }

  return order > orderMag;
}

std::string removeAllChar(std::string inStr, std::string inChar)
{
  while(inStr.find(inChar) != std::string::npos){inStr.replace(inStr.find(inChar), inChar.size(), "");}
  return inStr;
}


const Int_t nCutTypes = 6;
const std::string cutTypes[nCutTypes] = {"==", "!=", ">=", ">", "<=", "<"};
const std::string cutTypesStr[nCutTypes] = {"EQ", "NE", "GE", "GT", "LE", "LT"};

int getPosOfCutType(std::string inStr)
{
  int pos = -1;

  for(Int_t cI = 0; cI < nCutTypes; ++cI){
    while(inStr.find(cutTypes[cI]) != std::string::npos){
      pos = inStr.find(cutTypes[cI]);
      break;
    }
  }

  return pos;
}

std::string findAndReplaceCutTypes(std::string inStr)
{
  for(Int_t cI = 0; cI < nCutTypes; ++cI){
    while(inStr.find(cutTypes[cI]) != std::string::npos){
      inStr.replace(inStr.find(cutTypes[cI]), cutTypes[cI].size(), cutTypesStr[cI]);
    }
  }
  return inStr;
}


std::string getCompString(std::string inStr)
{
  std::string compStr = "";

  if(inStr.find("==") != std::string::npos) compStr = "==";
  else if(inStr.find("!=") != std::string::npos) compStr = "!=";
  else if(inStr.find(">=") != std::string::npos) compStr = ">=";
  else if(inStr.find(">") != std::string::npos) compStr = ">";
  else if(inStr.find("<=") != std::string::npos) compStr = "<=";
  else if(inStr.find("<") != std::string::npos) compStr = "<";

  return compStr;
}


bool valPassesCut(Double_t val, std::string cutStr, bool isCutAbs)
{
  std::string compStr = getCompString(cutStr);
  if(compStr.find("==") != std::string::npos || compStr.find("!=") != std::string::npos){
    Int_t cutVal = std::stoi(cutStr.substr(cutStr.find(compStr) + compStr.size(), cutStr.size()));
    
    if(compStr.find("==") != std::string::npos){
      bool toReturn = val == cutVal;
      if(isCutAbs) toReturn = toReturn || val == -1*cutVal;
      return toReturn;
    }
    else{
      bool toReturn = val != cutVal;
      if(isCutAbs) toReturn = toReturn && (val != -1*cutVal); 
      return toReturn;
    }
  }
  else{
    Double_t cutVal = std::stod(cutStr.substr(cutStr.find(compStr) + compStr.size(), cutStr.size()));

    if(compStr.find(">=") != std::string::npos){
      bool toReturn = val >= cutVal;
      if(isCutAbs) toReturn = toReturn || -1.0*val >= cutVal;
      return toReturn;
    }
    else if(compStr.find(">") != std::string::npos){
      bool toReturn = val > cutVal;
      if(isCutAbs) toReturn = toReturn || -1.0*val > cutVal;
      return toReturn;
    }
    else if(compStr.find("<=") != std::string::npos){
      bool toReturn = val <= cutVal;
      if(isCutAbs) toReturn = toReturn && -1.0*val <= cutVal;
      return toReturn;
    }
    else{
      bool toReturn = val < cutVal;
      if(isCutAbs) toReturn = toReturn && -1.0*val < cutVal;
      return toReturn;
    }
  }

  std::cout << "WARNING: VALPASSESCUT FAILED RETURN FALSE" << std::endl;
  return false;
}

bool valPassesCut(Long64_t val, std::string cutStr, bool isCutAbs)
{
  std::string compStr = getCompString(cutStr);
  if(compStr.find("==") != std::string::npos || compStr.find("!=") != std::string::npos){
    Long64_t cutVal = std::stoi(cutStr.substr(cutStr.find(compStr) + compStr.size(), cutStr.size()));
    
    if(compStr.find("==") != std::string::npos){
      bool toReturn = val == cutVal;
      if(isCutAbs) toReturn = toReturn || val == -1*cutVal;      
      return toReturn;
    }
    else{
      bool toReturn = val != cutVal;
      if(isCutAbs) toReturn = toReturn && (val != -1*cutVal); 
      return toReturn;
    }
  }
  else{
    Double_t cutVal = std::stod(cutStr.substr(cutStr.find(compStr) + compStr.size(), cutStr.size()));

    if(compStr.find(">=") != std::string::npos){
      bool toReturn = val >= cutVal;
      if(isCutAbs) toReturn = toReturn || -1.0*val >= cutVal;
      return toReturn;
    }
    else if(compStr.find(">") != std::string::npos){
      bool toReturn = val > cutVal;
      if(isCutAbs) toReturn = toReturn || -1.0*val > cutVal; 
      return toReturn;
    }
    else if(compStr.find("<=") != std::string::npos){
      bool toReturn = val <= cutVal;
      if(isCutAbs) toReturn = toReturn && -1.0*val <= cutVal;
      return toReturn;
    }
    else{
      bool toReturn = val < cutVal;
      if(isCutAbs) toReturn = toReturn && -1.0*val < cutVal;      
      return toReturn;
    }
  }

  std::cout << "WARNING: VALPASSESCUT FAILED RETURN FALSE" << std::endl;
  return false;
}


void dumpTreeNames(std::string fileName, std::vector<std::string> treeNames, bool isSelected)
{
  std::string selStr = " (No Selection)";
  if(isSelected) selStr = " (After Selection)";

  std::cout << "TTrees In File \'" << fileName << "\'" << selStr << ":" << std::endl;
  for(unsigned int tI = 0; tI < treeNames.size(); ++tI){
    std::cout << " Tree " << tI << "/" << treeNames.size() << ": " << treeNames.at(tI) << std::endl;
  }

  return;
}

bool doStringsMatch(std::vector<std::string>* fileList1, std::vector<std::string>* fileList2, std::vector<std::string>* misMatch1, std::vector<std::string>* misMatch2)
{
  bool allMatch = true;
  unsigned int fI1 = 0;
  while(fI1 < fileList1->size()){
    bool matchFound = false;
    for(unsigned int fI2 = 0; fI2 < fileList2->size(); ++fI2){
      if(fileList1->at(fI1).find(fileList2->at(fI2)) != std::string::npos && fileList1->at(fI1).size() == fileList2->at(fI2).size()){
	matchFound = true;
	break;
      }
    }
    if(!matchFound){
      allMatch = false;
      std::cout << " No match found for Tree \'" << fileList1->at(fI1) << "\', removing..." << std::endl;
      misMatch1->push_back(fileList1->at(fI1));
      fileList1->erase(fileList1->begin()+fI1);
    }
    else fI1++;
  }

  fI1 = 0;
  while(fI1 < fileList2->size()){
    bool matchFound = false;
    for(unsigned int fI2 = 0; fI2 < fileList1->size(); ++fI2){
      if(fileList2->at(fI1).find(fileList1->at(fI2)) != std::string::npos && fileList2->at(fI1).size() == fileList1->at(fI2).size()){
	matchFound = true;
	break;
      }
    }
    if(!matchFound){
      allMatch = false;
      std::cout << " No match found for Tree \'" << fileList2->at(fI1) << "\', removing..." << std::endl;
      misMatch2->push_back(fileList2->at(fI1));
      fileList2->erase(fileList2->begin()+fI1);
    }
    else fI1++;
  }

  return allMatch;
}

std::string texFriendlyString(std::string inStr)
{
  std::string outStr = "";
  while(inStr.find("_") != std::string::npos){
    outStr = outStr + inStr.substr(0, inStr.find("_")) + "\\_";
    inStr.replace(0, inStr.find("_")+1, "");
  }
  outStr = outStr + inStr;

  return outStr;
}

void doFirstTexSlide(std::ofstream* fileTex, std::vector<std::string> inFileNames, std::vector<std::string> inNickNames, std::vector<std::string> goodTrees, std::vector<std::vector<std::string> > missingTrees, const Int_t eventCountOverride, const std::string footerLine)
{
  TDatime date;

  for(unsigned int fI = 0; fI < inFileNames.size(); ++fI){
    inFileNames.at(fI) = texFriendlyString(inFileNames.at(fI));
    inFileNames.at(fI) = inFileNames.at(fI).substr(0, inFileNames.at(fI).size()/2) + "\\\\" + inFileNames.at(fI).substr(inFileNames.at(fI).size()/2, inFileNames.at(fI).size());
  }

  (*fileTex) << "\\RequirePackage{xspace}" << std::endl;
  (*fileTex) << "\\RequirePackage{amsmath}" << std::endl;

  (*fileTex) << std::endl;

  (*fileTex) << "\\documentclass[xcolor=dvipsnames]{beamer}" << std::endl;
  (*fileTex) << "\\usetheme{Warsaw}" << std::endl;
  (*fileTex) << "\\setbeamercolor{structure}{}" << std::endl;
  (*fileTex) << "\\setbeamercolor{footlinecolor}{fg=white,bg=lightgray}" << std::endl;
  (*fileTex) << "\\newcommand{\\pt}{\\ensuremath{p_{\\mathrm{T}}}\\xspace}" << std::endl;
  (*fileTex) << "\\setbeamersize{text margin left=5pt,text margin right=5pt}" << std::endl;

  (*fileTex) << std::endl;

  (*fileTex) << "\\setbeamertemplate{frametitle}" << std::endl;
  (*fileTex) << "{" << std::endl;
  (*fileTex) << "  \\nointerlineskip" << std::endl;
  (*fileTex) << "  \\begin{beamercolorbox}[sep=0.3cm, ht=1.8em, wd=\\paperwidth]{frametitle}" << std::endl;
  (*fileTex) << "    \\vbox{}\\vskip-2ex%" << std::endl;
  (*fileTex) << "    \\strut\\insertframetitle\\strut" << std::endl;
  (*fileTex) << "    \\vskip-0.8ex%" << std::endl;
  (*fileTex) << "  \\end{beamercolorbox}" << std::endl;
  (*fileTex) << "}" << std::endl;

  (*fileTex) << std::endl;

  (*fileTex) << "\\setbeamertemplate{footline}{%" << std::endl;
  (*fileTex) << "  \\begin{beamercolorbox}[sep=.8em,wd=\\paperwidth,leftskip=0.5cm,rightskip=0.5cm]{footlinecolor}" << std::endl;
  (*fileTex) << "    \\hspace{0.3cm}%" << std::endl;
  (*fileTex) << "    \\hfill\\insertauthor \\hfill\\insertpagenumber" << std::endl;
  (*fileTex) << "  \\end{beamercolorbox}%" << std::endl;
  (*fileTex) << "}" << std::endl;

  (*fileTex) << std::endl;

  (*fileTex) << "\\setbeamertemplate{navigation symbols}{}" << std::endl;
  (*fileTex) << "\\setbeamertemplate{itemize item}[circle]" << std::endl;
  (*fileTex) << "\\setbeamertemplate{itemize subitem}[circle]" << std::endl;
  (*fileTex) << "\\setbeamertemplate{itemize subsubitem}[circle]" << std::endl;
  (*fileTex) << "\\setbeamercolor{itemize item}{fg=black}" << std::endl;
  (*fileTex) << "\\setbeamercolor{itemize subitem}{fg=black}" << std::endl;
  (*fileTex) << "\\setbeamercolor{itemize subsubitem}{fg=black}" << std::endl;

  (*fileTex) << std::endl;

  (*fileTex) << "\\definecolor{links}{HTML}{00BFFF}" << std::endl;
  (*fileTex) << "\\hypersetup{colorlinks=true,linkcolor=blue,urlcolor=links}" << std::endl;

  (*fileTex) << std::endl;

  (*fileTex) << "\\author[CM]{" << footerLine << "}" << std::endl;
  (*fileTex) << "\\begin{document}" << std::endl;

  (*fileTex) << std::endl;


  (*fileTex) << "\\begin{frame}" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{\\hypertarget{TopLevel}{Forest Validation} (" << date.GetYear() << "." << date.GetMonth() << "." << date.GetDay() << ")}}" << std::endl;
  (*fileTex) << " \\begin{itemize}" << std::endl;
  (*fileTex) << "  \\fontsize{5}{5}\\selectfont" << std::endl;
  if(eventCountOverride >= 0) (*fileTex) << " \\item{WARNING: EVENT NUMBER OVERRIDE SET, TEST INVALID}" << std::endl;

  for(unsigned int fI = 0; fI < inFileNames.size(); ++fI){
    (*fileTex) << "  \\item{File " << fI+1 << ": \'" << inFileNames.at(fI) << "\'}" << std::endl;
    (*fileTex) << "  \\item{Nickname: \'" << texFriendlyString(inNickNames.at(fI)) << "\'}" << std::endl;
  }

  (*fileTex) << "  \\item{Good Trees:}" << std::endl;
  (*fileTex) << " \\begin{itemize}" << std::endl;
  (*fileTex) << "  \\fontsize{5}{5}\\selectfont" << std::endl;
  if(goodTrees.size() != 0){
    for(unsigned int tI = 0; tI < goodTrees.size(); ++tI){
      (*fileTex) << "  \\item{Tree " << tI << ": \'" << texFriendlyString(goodTrees.at(tI)) << "\',      \\hyperlink{tocSummary_" << goodTrees.at(tI) << "}{To Summary TOC},      \\hyperlink{tocFull_0_" << goodTrees.at(tI) << "}{To All Branch TOC}}" << std::endl;
    }
  }
  else (*fileTex) << "  \\item{WARNING: NO GOOD TREES}" << std::endl;
  (*fileTex) << " \\end{itemize}" << std::endl;

  for(unsigned int fI = 0; fI < inNickNames.size(); ++fI){
    (*fileTex) << "  \\item{Bad File " << fI+1 << ", " << texFriendlyString(inNickNames.at(fI)) << ", Trees:}" << std::endl;

    (*fileTex) << " \\begin{itemize}" << std::endl;
    (*fileTex) << "  \\fontsize{5}{5}\\selectfont" << std::endl;
    if(missingTrees.at(fI).size() != 0){
      for(unsigned int tI = 0; tI < missingTrees.at(fI).size(); ++tI){
	(*fileTex) << "  \\item{Tree " << tI << ": \'" << texFriendlyString(missingTrees.at(fI).at(tI)) << "\'}" << std::endl;
      }
    }
    else (*fileTex) << "  \\item{NO MISSING TREES}" << std::endl;
    (*fileTex) << " \\end{itemize}" << std::endl;
  }

  (*fileTex) << " \\end{itemize}" << std::endl;
  (*fileTex) << "\\end{frame}" << std::endl;
  (*fileTex) << std::endl;

  return;
}


void doInterstitialTexSlide(std::ofstream* fileTex, const std::string interstitialString)
{
  (*fileTex) << "\\begin{frame}" << std::endl;
  (*fileTex) << "\\fontsize{7}{7}\\selectfont" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{Interstitial}}" << std::endl;
  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{7}{7}\\selectfont" << std::endl;
  (*fileTex) << "\\item{" << interstitialString << "}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\end{frame}" << std::endl;
  (*fileTex) << std::endl;

  return;
}


void doTreeTexSlide(std::ofstream* fileTex, std::string inTreeName, std::vector<std::string> nickNames, std::vector<std::vector<std::string > > listOfBadVar, std::vector<std::string> warningList, std::vector<int> warningListPos)
{
  std::string treeTitle = texFriendlyString(inTreeName);
  while(treeTitle.find("/") != std::string::npos){
    treeTitle.replace(treeTitle.find("/"), 1, " ");
  }

  std::vector<std::string> bigColumnVect;

  for(unsigned int fI = 0; fI < listOfBadVar.size(); ++fI){
    bigColumnVect.push_back("\\fontsize{6}{6}\\selectfont \\bf{Missing variables from \'" + texFriendlyString(nickNames.at(fI)) + "\' file:}");

    if(listOfBadVar.at(fI).size() != 0){
      for(unsigned int vI = 0; vI < listOfBadVar.at(fI).size(); ++vI){
	std::string varStr = std::to_string(vI) + "/" + std::to_string(listOfBadVar.at(fI).size()) + ": \'" + texFriendlyString(listOfBadVar.at(fI).at(vI)) + "\'";
	bigColumnVect.push_back(varStr);
      }
    }
    else bigColumnVect.push_back("\\bf{\\qquad No missing variables!}");
  }
  
  bigColumnVect.push_back("\\fontsize{6}{6}\\selectfont \\bf{Variables non-unity in ratio:}");
  if(warningList.size() == 0) bigColumnVect.push_back("\\bf{\\qquad No variables non-unity!}");
  else{
    for(unsigned int bI = 0; bI < warningList.size(); ++bI){
      std::string warnStr = "Var " + std::to_string(bI) + "/" + std::to_string(warningList.size()) + ": \'\\hyperlink{branch_" + inTreeName + "_" + std::to_string(warningListPos.at(bI)) + "}{" + texFriendlyString(warningList.at(bI)) + "}\'";
      bigColumnVect.push_back(warnStr);
    }
  }

  for(unsigned int bI = 0; bI < bigColumnVect.size(); ++bI){
    if(bI%48 == 0){
      if(bI != 0){
	(*fileTex) << "\\end{itemize}" << std::endl;
	(*fileTex) << "\\end{column}" << std::endl;
	(*fileTex) << "\\end{columns}" << std::endl;
	(*fileTex) << "\\end{itemize}" << std::endl;
	(*fileTex) << "\\end{frame}" << std::endl;
	
	(*fileTex) << std::endl;
      }

      (*fileTex) << std::endl;
      (*fileTex) << "\\begin{frame}" << std::endl;
      (*fileTex) << "\\fontsize{6}{6}\\selectfont" << std::endl;
      if(bI == 0) (*fileTex) << "\\frametitle{\\centerline{\\hypertarget{tocSummary_" << inTreeName << "}{" << treeTitle << "}}}" << std::endl;
      else (*fileTex) << "\\frametitle{\\centerline{" << treeTitle << "}}" << std::endl;
      (*fileTex) << "\\begin{itemize}" << std::endl;
      (*fileTex) << "\\fontsize{6}{6}\\selectfont" << std::endl;
      (*fileTex) << "\\item{Tree Name \'" << texFriendlyString(inTreeName) << "\'}, (\\hyperlink{TopLevel}{Back to Top})" << std::endl;

      (*fileTex) << "\\begin{columns}" << std::endl;
      (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
      
      (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
      (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
      (*fileTex) << "\\begin{itemize}" << std::endl;
      (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
    }
    else if(bI%16 == 0){
      (*fileTex) << "\\end{itemize}" << std::endl;
      (*fileTex) << "\\end{column}" << std::endl;
      (*fileTex) << std::endl;
            
      (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
      (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
      (*fileTex) << "\\begin{itemize}" << std::endl;
      (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
    }

    (*fileTex) << "\\item{" << bigColumnVect.at(bI) << "}" << std::endl;
  }
  
  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\end{column}" << std::endl;

  if(bigColumnVect.size()%48 < 32){
    (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
    (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
    (*fileTex) << "\\begin{itemize}" << std::endl;
    (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
    (*fileTex) << "\\item{Dummy}" << std::endl;
    (*fileTex) << "\\end{itemize}" << std::endl;
    (*fileTex) << "\\end{column}" << std::endl;    
  }

  if(bigColumnVect.size()%48 < 16){
    (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
    (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
    (*fileTex) << "\\begin{itemize}" << std::endl;
    (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
    (*fileTex) << "\\item{Dummy}" << std::endl;
    (*fileTex) << "\\end{itemize}" << std::endl;
    (*fileTex) << "\\end{column}" << std::endl;    
  }

  (*fileTex) << "\\end{columns}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\end{frame}" << std::endl;
  
  (*fileTex) << std::endl;
  (*fileTex) << std::endl;

  return;
}


void doBranchTexSlide(std::ofstream* fileTex, std::string inTreeName, std::vector<std::string> listOfBranches)
{
  std::string treeTitle = texFriendlyString(inTreeName);
  while(treeTitle.find("/") != std::string::npos){
    treeTitle.replace(treeTitle.find("/"), 1, " ");
  }
  
  int nBranches = 0;
  std::vector<std::vector<std::string> > branchVect1, branchVect2, branchVect3;

  for(unsigned int bI = 0; bI < listOfBranches.size(); ++bI){
    if(bI%48 == 0){
      branchVect1.push_back({});
      branchVect2.push_back({});
      branchVect3.push_back({});
    }

    if((bI/16)%3 == 0) branchVect1.at(branchVect1.size()-1).push_back(listOfBranches.at(bI));
    else if((bI/16)%3 == 1) branchVect2.at(branchVect2.size()-1).push_back(listOfBranches.at(bI));
    else if((bI/16)%3 == 2) branchVect3.at(branchVect3.size()-1).push_back(listOfBranches.at(bI));
  }

  for(unsigned int tempI = 0; tempI < branchVect1.size(); ++tempI){
    (*fileTex) << "\\begin{frame}" << std::endl;
    (*fileTex) << "\\fontsize{4}{4}\\selectfont" << std::endl;
    (*fileTex) << "\\frametitle{\\centerline{\\hypertarget{tocFull_" << tempI << "_" << inTreeName << "}{" << treeTitle << "}}}" << std::endl;
    (*fileTex) << "\\begin{itemize}" << std::endl;
    (*fileTex) << "\\fontsize{4}{4}\\selectfont" << std::endl;
    (*fileTex) << "\\item{Tree Name \'" << texFriendlyString(inTreeName) << "\', (\\hyperlink{TopLevel}{Back to Top})}" << std::endl;
    (*fileTex) << "\\item{Branches}" << std::endl;

    (*fileTex) << "\\begin{columns}" << std::endl;
    (*fileTex) << "\\fontsize{3}{3}\\selectfont" << std::endl;

    (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
    (*fileTex) << "\\fontsize{3}{3}\\selectfont" << std::endl;
    (*fileTex) << "\\begin{itemize}" << std::endl;
    (*fileTex) << "\\fontsize{3}{3}\\selectfont" << std::endl;
    if(branchVect1.at(tempI).size() != 0){

      for(unsigned int vI = 0; vI < branchVect1.at(tempI).size(); ++vI){
	(*fileTex) << "\\item{Var " << nBranches << "/" << listOfBranches.size() << ": \'\\hyperlink{branch_" << inTreeName << "_" << nBranches << "}{" << texFriendlyString(branchVect1.at(tempI).at(vI)) << "}\'}" << std::endl;
	nBranches++;
      }
    }
    else (*fileTex) << "\\item{Dummy}" << std::endl;
    (*fileTex) << "\\end{itemize}" << std::endl;
    (*fileTex) << "\\end{column}" << std::endl;

    (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
    (*fileTex) << "\\fontsize{3}{3}\\selectfont" << std::endl;
    (*fileTex) << "\\begin{itemize}" << std::endl;
    (*fileTex) << "\\fontsize{3}{3}\\selectfont" << std::endl;
    if(branchVect2.at(tempI).size() != 0){
      
      for(unsigned int vI = 0; vI < branchVect2.at(tempI).size(); ++vI){
	(*fileTex) << "\\item{Var " << nBranches << "/" << listOfBranches.size() << ": \'\\hyperlink{branch_" << inTreeName << "_" << nBranches << "}{" << texFriendlyString(branchVect2.at(tempI).at(vI)) << "}\'}" << std::endl;
	//	(*fileTex) << "\\item{Var " << nBranches << "/" << listOfBranches.size() << ": \'" << texFriendlyString(branchVect2.at(tempI).at(vI)) << "\'}" << std::endl;
	nBranches++;
      }
    }
    else (*fileTex) << "\\item{Dummy}" << std::endl;
    (*fileTex) << "\\end{itemize}" << std::endl;
    (*fileTex) << "\\end{column}" << std::endl;

    (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
    (*fileTex) << "\\fontsize{3}{3}\\selectfont" << std::endl;
    (*fileTex) << "\\begin{itemize}" << std::endl;
    (*fileTex) << "\\fontsize{3}{3}\\selectfont" << std::endl;
    if(branchVect3.at(tempI).size() != 0){

      for(unsigned int vI = 0; vI < branchVect3.at(tempI).size(); ++vI){
	(*fileTex) << "\\item{Var " << nBranches << "/" << listOfBranches.size() << ": \'\\hyperlink{branch_" << inTreeName << "_" << nBranches << "}{" << texFriendlyString(branchVect3.at(tempI).at(vI)) << "}\'}" << std::endl;
	//	(*fileTex) << "\\item{Var " << nBranches << "/" << listOfBranches.size() << ": \'" << texFriendlyString(branchVect3.at(tempI).at(vI)) << "\'}" << std::endl;
	nBranches++;
      }
    }
    else (*fileTex) << "\\item{Dummy}" << std::endl;
    (*fileTex) << "\\end{itemize}" << std::endl;
    (*fileTex) << "\\end{column}" << std::endl;

    (*fileTex) << "\\end{columns}" << std::endl;

    (*fileTex) << "\\end{itemize}" << std::endl;
    (*fileTex) << "\\end{frame}" << std::endl;
    
    (*fileTex) << std::endl;
  }

  return;
}


void doPlotTexSlide(std::ofstream* fileTex, std::string inTreeName, std::string inBranchName, int inBranchNum, std::string inPdfName)
{
  std::string treeTitle = texFriendlyString(inTreeName);
  while(treeTitle.find("/") != std::string::npos){
    treeTitle.replace(treeTitle.find("/"), 1, " ");
  }

  (*fileTex) << "\\begin{frame}" << std::endl;
  (*fileTex) << "\\fontsize{6}{6}\\selectfont" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{\\hypertarget{branch_" << inTreeName << "_" << inBranchNum << "}{" << texFriendlyString(inBranchName) << "} (" << treeTitle << ")}}" << std::endl;

  (*fileTex) << "\\begin{center}" << std::endl;

  if(inBranchName.find(":") == std::string::npos) (*fileTex) << "\\includegraphics[width=0.6\\textwidth]{" << inPdfName << "}" << std::endl;
  else (*fileTex) << "\\includegraphics[width=0.98\\textwidth]{" << inPdfName << "}" << std::endl;
  (*fileTex) << "\\end{center}" << std::endl;

  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{6}{6}\\selectfont" << std::endl;
  (*fileTex) << "\\item{" << texFriendlyString(inBranchName) << ", \\hyperlink{tocSummary_" << inTreeName << "}{To Summary TOC}, \\hyperlink{tocFull_0_" << inTreeName << "}{To All Branch TOC}}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\end{frame}" << std::endl;

  (*fileTex) << std::endl;

  return;
}


double getNonZeroTH2Min(TH2* inHist_p)
{
  double nonZeroTH2Min = inHist_p->GetBinContent(1, 1);

  for(Int_t bIX = 0; bIX < inHist_p->GetNbinsX()+1; ++bIX){
    for(Int_t bIY = 0; bIY < inHist_p->GetNbinsY()+1; ++bIY){
      if(inHist_p->GetBinContent(bIX+1, bIY+1) <= 0) continue;
      if(nonZeroTH2Min > inHist_p->GetBinContent(bIX+1, bIY+1)) nonZeroTH2Min = inHist_p->GetBinContent(bIX+1, bIY+1);
    }
  }

  return nonZeroTH2Min;
}

//2023.09.21 switching to doing this with a nice tenv instead of infinite args
//int runForestDQM(std::vector<std::string> inFileNames, const std::string additionalNickName, std::vector<std::string> inNickNames, const std::string treeSelect = "", const Bool_t doEventNorm = false, const Int_t eventCountOverride = -1, const std::string commaSeparatedCutList = "", const std::string commaSeparatedPairList = "")

int runForestDQM(std::string inConfigName = "")
{
  //Switch to using the checkMakeDir class later
  //Check the inConfigName is good
  if(!checkFile(inConfigName)){
    std::cout << "Given config \'" << inConfigName << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  //Grab the configuration with a TEnv
  TEnv* config_p = new TEnv(inConfigName.c_str());
  //Start grabbing config variables
  //Grab global nickname to distinguish slides
  std::string additionalNickName = config_p->GetValue("GLOBALNICKNAME", "");

  //Grab file names and file nicknames (for legends)
  const int nMaxFiles = 8;
  std::vector<std::string> inFileNames, inNickNames;
  for(int fI = 0; fI < nMaxFiles; ++fI){
    std::string configStr1 = "INFILENAME." + std::to_string(fI);
    std::string configStr2 = "NICKNAME." + std::to_string(fI);

    std::string tempFileName = config_p->GetValue(configStr1.c_str(), "");
    std::string tempNickName = config_p->GetValue(configStr2.c_str(), "");      

    if(tempFileName.size() == 0) continue;

    if(tempNickName.size() == 0){
      std::cout << "Nickname for file \'" << tempFileName << "\' doesn't exist; skipping both" << std::endl;
      continue;
    }

    inFileNames.push_back(tempFileName);
    inNickNames.push_back(tempNickName);
  }

  //FOOTER LINE FOR AUTO SLIDES
  const std::string footerLine = config_p->GetValue("FOOTERLINE", "DEFAULT FOOTER LINE (SET IN CONFIG)");

  //Get doWeight, weightTree, etc.
  bool doWeight = config_p->GetValue("DOWEIGHT", false);
  std::string weightTree = config_p->GetValue("WEIGHTTREE", "");
  std::string weightVar = config_p->GetValue("WEIGHTVAR", "");
  
  //Check the weight parameters before proceeding if doWeight is true
  if(doWeight){
    if(weightTree.size() == 0){
      std::cout << "WARNING: DOWEIGHT is true but weightTree string is empty. Setting doWeight to false, no weights will be applied" << std::endl;
      doWeight = false;
    }
    if(weightVar.size() == 0){
      std::cout << "WARNING: DOWEIGHT is true but weightVar string is empty. Setting doWeight to false, no weights will be applied" << std::endl;
      doWeight = false;
    }    
  }
  
  //Keep treeselect as comma seperated list for now but switch to direct vector soon
  std::string treeSelect = "";
  for(unsigned int tI = 0; tI < 100; ++tI){
    std::string configStr = "TREESELECT." + std::to_string(tI);
    std::string tempTreeStr = config_p->GetValue(configStr.c_str(), "");
    if(tempTreeStr.size() == 0) continue;
    treeSelect = treeSelect + tempTreeStr + "," ;
  }
   
  //Default to doing event norm.
  const Bool_t doEventNorm = config_p->GetValue("DOEVENTNORM", 1);
  //Default no override i.e. < 0
  const Int_t eventCountOverride = config_p->GetValue("EVENTCOUNTOVERRIDE", -1);

  //Switch this to be not comma based, i.e. vector instead but later
  std::vector<std::string> commaSeparatedCutList;
  std::string commaSeparatedPairList = "";
  
  //Just check some arbitrarily high number of cuts
  for(unsigned int i = 0; i < 1000; ++i){
    std::string configStr1 = "CUT." + std::to_string(i);
    std::string configStr2 = "PAIR." + std::to_string(i);

    std::string tempCut = config_p->GetValue(configStr1.c_str(), "");
    if(tempCut.size() != 0) commaSeparatedCutList.push_back(tempCut);
      //commaSeparatedCutList = commaSeparatedCutList + tempCut + ",";

    std::string tempPair = config_p->GetValue(configStr2.c_str(), "");
    if(tempPair.size() != 0){
      while(tempPair.find("\"") != std::string::npos){tempPair.replace(tempPair.find("\""), 1, "");}
      commaSeparatedPairList = commaSeparatedPairList + tempPair + ",";
    }
  }

  //New Div min/max override
  std::map<std::string, double> divMinOverrideMap;
  std::map<std::string, double> divMaxOverrideMap;
  for(unsigned int i = 0; i < 1000; ++i){
    std::string configStr1 = "DIVMIN." + std::to_string(i);
    std::string configStr2 = "DIVMAX." + std::to_string(i);

    std::string tempDivMinVal = config_p->GetValue(configStr1.c_str(), "");
    if(tempDivMinVal.size() != 0){
      std::string varStr = tempDivMinVal.substr(0, tempDivMinVal.find(":"));
      std::string valStr = tempDivMinVal.substr(tempDivMinVal.find(":")+1, tempDivMinVal.size());
      divMinOverrideMap[varStr] = std::stod(valStr);
    }

    std::string tempDivMaxVal = config_p->GetValue(configStr2.c_str(), "");
    if(tempDivMaxVal.size() != 0){
      std::string varStr = tempDivMaxVal.substr(0, tempDivMaxVal.find(":"));
      std::string valStr = tempDivMaxVal.substr(tempDivMaxVal.find(":")+1, tempDivMaxVal.size());
      divMaxOverrideMap[varStr] = std::stod(valStr);
    }
  }

  //NEW: NBIN OVERRIDE FOR NUMBER OF BINS
  std::map<std::string, int> nBinOverrideMap;
  for(unsigned int i = 0; i < 1000; ++i){
    std::string configStr = "NBIN." + std::to_string(i);

    std::string nBinOverrideVal = config_p->GetValue(configStr.c_str(), "");
    if(nBinOverrideVal.size() != 0){
      std::string varStr = nBinOverrideVal.substr(0, nBinOverrideVal.find(":"));
      std::string valStr = nBinOverrideVal.substr(nBinOverrideVal.find(":")+1, nBinOverrideVal.size());
      nBinOverrideMap[varStr] = std::stod(valStr);
    }
  }

  //DOLOGX, LOGY Overrides
  std::map<std::string, bool> doLogXOverrideMap, doLogYOverrideMap;
  for(unsigned int i = 0; i < 1000; ++i){
    std::string configStrX = "DOLOGX." + std::to_string(i);
    std::string configStrY = "DOLOGY." + std::to_string(i);

    std::string doLogXOverrideVal = config_p->GetValue(configStrX.c_str(), "");
    if(doLogXOverrideVal.size() != 0){
      std::string varStr = doLogXOverrideVal.substr(0, doLogXOverrideVal.find(":"));
      std::string valStr = doLogXOverrideVal.substr(doLogXOverrideVal.find(":")+1, doLogXOverrideVal.size());
      doLogXOverrideMap[varStr] = std::stod(valStr);
    }
    
    std::string doLogYOverrideVal = config_p->GetValue(configStrX.c_str(), "");
    if(doLogYOverrideVal.size() != 0){
      std::string varStr = doLogYOverrideVal.substr(0, doLogYOverrideVal.find(":"));
      std::string valStr = doLogYOverrideVal.substr(doLogYOverrideVal.find(":")+1, doLogYOverrideVal.size());
      doLogYOverrideMap[varStr] = std::stod(valStr);
    }
  }
  
  //New - skippable variables - not gonna make them ttree specific but that will likely need adding
  std::map<std::string, bool> mapOfVarToSkip;
  const int nMaxVarSkip = 10000;
  for(unsigned int vI = 0; vI < nMaxVarSkip; ++vI){
    std::string configStr = "VARTOSKIP." + std::to_string(vI);

    std::string tempVarToSkip = config_p->GetValue(configStr.c_str(), "");
    if(tempVarToSkip.size() == 0) continue;

    mapOfVarToSkip[tempVarToSkip] = true;
  }
    
  //Start using the input parameters
  std::string globalYStrTemp = "Counts";
  if(doWeight) globalYStrTemp = "Weighted Counts";
  if(doEventNorm){
    if(doWeight) globalYStrTemp = "#frac{1}{N_{evt}^{Weighted}} " + globalYStrTemp;
    else globalYStrTemp = "#frac{1}{N_{evt}} " + globalYStrTemp;
  }

  const std::string globalYStr = globalYStrTemp;

  const double divMax = 1.5;
  const double divMin = 0.5;

  const Int_t nFiles = inFileNames.size();
  const Int_t fileCap = nMaxFiles;
  if(nFiles > fileCap){
    std::cout << "Number of files given, " << nFiles << ", is greater than file cap, " << fileCap << ", for this macro. return 1" << std::endl;
    return 1;
  }

  for(Int_t fI = 0; fI < nFiles; ++fI){
    if(inFileNames.at(fI).find(".root") == std::string::npos || !checkFile(inFileNames.at(fI))){
      std::cout << "Input \'" << inFileNames.at(fI) << "\' is invalid return 1." << std::endl;
      return 1;
    }
    else if(inNickNames.at(fI).size() == 0) inNickNames.at(fI) = "File" + std::to_string(fI+1);
  }

  std::vector<std::string> pairVars1;
  std::vector<std::string> pairVars2;
  std::vector<std::string> pairCutVars;
  std::vector<std::string> pairCutTypes;
  std::vector<std::string> pairCutVals;

  std::vector<double> pairVarsMin1;
  std::vector<double> pairVarsMin2;
  std::vector<double> pairVarsMax1;
  std::vector<double> pairVarsMax2;

  if(commaSeparatedPairList.size() != 0){
    std::string commaSeparatedPairListCopy = commaSeparatedPairList;

    if(commaSeparatedPairListCopy.substr(commaSeparatedPairListCopy.size()-1, 1).find(",") == std::string::npos) commaSeparatedPairListCopy = commaSeparatedPairListCopy + ",";
    while(commaSeparatedPairListCopy.find(",") != std::string::npos){
      std::string varStr = commaSeparatedPairListCopy.substr(0, commaSeparatedPairListCopy.find(","));
      if(varStr.find(":") == std::string::npos){
	std::cout << "Potential variable pair \'" << varStr << "\' from \'" << commaSeparatedPairList << "\' is not valid syntax. Please input as \'var1:var2\'. skipping..." << std::endl;
      }
      else{
	std::string var1 = varStr.substr(0, varStr.find(":"));
	varStr.replace(0, varStr.find(":")+1, "");
	std::string var2 = varStr.substr(0, varStr.find(":"));
	varStr.replace(0, varStr.find(":")+1, "");

	std::string var3 = "";
	std::string cut3 = "";
	std::string cutVal3 = "";

	if(varStr.size() != 0){
	  cut3 = getCompString(varStr);
	  
	  if(cut3.size() != 0){
	    var3 = varStr.substr(0, varStr.find(cut3));
	    varStr.replace(0, varStr.find(cut3)+cut3.size(), "");
	    cutVal3 = varStr;
	  }
	  else std::cout << "WARNING: 2-D GIVEN BUT NO VALID COMPARISON STRING. PROCEEDING UNCUT" << std::endl;
	}

	pairVars1.push_back(var1);
	pairVars2.push_back(var2);	

	std::cout << "PAIR CUT VAR: " << var3 << ", " << cut3 << ", " << cutVal3 << std::endl;
	 
	pairCutVars.push_back(var3);
	pairCutTypes.push_back(cut3);
	pairCutVals.push_back(cutVal3);

	pairVarsMin1.push_back(-1);
	pairVarsMax1.push_back(-1);
	pairVarsMin2.push_back(-1);
	pairVarsMax2.push_back(-1);
      }

      commaSeparatedPairListCopy.replace(0, commaSeparatedPairListCopy.find(",")+1, "");
    }
  }

  std::map<std::string, std::vector<std::string> > mapFromVarToCut;
  std::map<std::string, std::vector<std::string> > mapFromVarToCutVar;
  std::map<std::string, std::vector<bool> > mapFromVarToIsCutAbs;
  std::map<std::string, std::vector<bool> > mapFromVarToIsCutLT;
  
  if(commaSeparatedCutList.size() != 0){
    for(unsigned int cI = 0; cI < commaSeparatedCutList.size(); ++cI){
      std::string varStr = commaSeparatedCutList[cI];

      if(varStr.find(":") == std::string::npos){
	std::cout << "Potential cut \'" << varStr << "\' is not valid syntax. Please input as \'var:cut1,cut2,cut3....\'. skipping..." << std::endl;
      }      

      std::string var1 = varStr.substr(0, varStr.find(":"));
      if(mapFromVarToCut.count(var1) > 0) continue;

      mapFromVarToCut[var1] = {};
      mapFromVarToCutVar[var1] = {};
      mapFromVarToIsCutAbs[var1] = {};
      mapFromVarToIsCutLT[var1] = {};
      
      varStr.replace(0, varStr.find(":")+1, "");

      if(varStr.substr(varStr.size()-1,1).find(",") == std::string::npos) varStr = varStr+",";
      while(varStr.find(",") != std::string::npos){
	std::string varStrCopy = varStr.substr(0, varStr.find(","));

	if(varStrCopy.find("abs(") != std::string::npos){
	  mapFromVarToIsCutAbs[var1].push_back(true);
	  varStrCopy.replace(0,4,"");
	  varStrCopy.replace(varStrCopy.find(")"), 1, "");
	}
	else mapFromVarToIsCutAbs[var1].push_back(false);

	if(varStrCopy.find("<") != std::string::npos){
	  mapFromVarToIsCutLT[var1].push_back(true);
	}
	else mapFromVarToIsCutLT[var1].push_back(false);

	std::string var2 = varStrCopy;
	
	mapFromVarToCut[var1].push_back(var2);
	std::string var3 = var2.substr(0, var2.find(getCompString(var2)));
	mapFromVarToCutVar[var1].push_back(var3);

	//	break;
	varStr.replace(0, varStr.find(",")+1, "");
      }

      
    }
  }

  std::cout << "Processing the following potential pairs in all TTree: " << std::endl;
  for(unsigned int i = 0; i < pairVars1.size(); ++i){
    std::cout << " " << i << "/" << pairVars1.size() << ": " << pairVars1.at(i) << ":" << pairVars2.at(i) << ":" << pairCutVars.at(i) << std::endl;
  }

  /*
  std::cout << "Full cut list: " << std::endl;
  for(auto const& cut : mapFromVarToCut){
    std::cout << " \'" << cut.first << "\', \'" << cut.second << "\', \'" << mapFromVarToCutVar[cut.first] << "\'" << std::endl;
  }
  */
  //  return 1;

  Double_t minDeltaVal = 0.000000001;
  kirchnerPalette col;
  TDatime date;
  const std::string dateStr = std::to_string(date.GetDate());
  const Int_t nMaxBins = 100;

  const Int_t colors[fileCap] = {1, col.getColor(2), col.getColor(0), col.getColor(3), col.getColor(2), col.getColor(0), col.getColor(3), col.getColor(2)};
  const Int_t styles[fileCap] = {20, 25, 28, 27, 46, 24, 42, 44};

  checkMakeDir("pdfDir");
  checkMakeDir(("pdfDir/" + dateStr).c_str());

  TLine line;
  line.SetLineStyle(2);

  TH1F* dummyHists_p[nFiles];
  for(Int_t fI = 0; fI < nFiles; ++fI){
    dummyHists_p[fI] = new TH1F();
    dummyHists_p[fI]->SetMarkerStyle(styles[fI]);
    dummyHists_p[fI]->SetMarkerSize(1.0);
    dummyHists_p[fI]->SetMarkerColor(colors[fI]);
    dummyHists_p[fI]->SetLineColor(colors[fI]);    
  }

  const double textSize = 0.045;
  const int textFont = 42;
  
  TLegend* leg_p = new TLegend(.6, .6, .9, .9);
  leg_p->SetBorderSize(0);
  leg_p->SetFillStyle(0);
  leg_p->SetTextFont(textFont);
  leg_p->SetTextSize(textSize);

  for(Int_t fI = 0; fI < nFiles; ++fI){leg_p->AddEntry(dummyHists_p[fI], inNickNames.at(fI).c_str(), "P L");}

  std::vector<std::vector<std::string > > misMatchedTrees;

  std::string texFileName = "forestDQM_" + additionalNickName;
  for(Int_t fI = 0; fI < nFiles; ++fI){
    texFileName = texFileName + "_" + inNickNames.at(fI);
  }
  texFileName = texFileName + "_" + dateStr;

  std::string texTreeStr = "AllTrees";
  if(treeSelect.size() != 0) texTreeStr = "NotAllTrees";
  //  while(texTreeStr.find("/") != std::string::npos){texTreeStr.replace(texTreeStr.find("/"), 1, "_");}

  std::string fullDirName = "pdfDir/" + dateStr + "/" + texFileName;
  while(fullDirName.substr(fullDirName.size()-1,1).find("_") != std::string::npos){fullDirName.replace(fullDirName.size()-1, 1, "");}
  fullDirName = fullDirName + "/";

  std::cout << "File name 1: " << texFileName << std::endl;
  checkMakeDir(fullDirName.c_str());

  std::string texFileSubName = fullDirName + texFileName + "VALIDATION_SUB_" + texTreeStr + "_" + dateStr + ".tex";
  std::cout << "File name 2: " << texFileSubName << std::endl;
  texFileName = fullDirName + texFileName + "VALIDATION_MAIN_" + texTreeStr + "_" + dateStr + ".tex";
  std::ofstream fileTex(texFileName.c_str());
  std::ofstream fileSubTex(texFileSubName.c_str());
  doInterstitialTexSlide(&(fileSubTex), "PLOTS");

  std::string treeSelectCopy = treeSelect;
  std::vector<std::string> treesToSelect;
  while(treeSelectCopy.find(",") != std::string::npos){
    treesToSelect.push_back(treeSelectCopy.substr(0, treeSelectCopy.find(",")));
    treeSelectCopy.replace(0, treeSelectCopy.find(",")+1, "");
  }
  if(treeSelectCopy.size() != 0) treesToSelect.push_back(treeSelectCopy);

  TFile* inFiles_p[nFiles];
  Double_t inFilesWeightedCounts[nFiles];
  std::vector<std::vector<std::string > > fileTrees;
  for(Int_t fI = 0; fI < nFiles; ++fI){
    inFiles_p[fI] = TFile::Open(mntToXRootdFileString(inFileNames.at(fI)).c_str(), "READ");
    std::vector<std::string> tempTrees = returnRootFileContentsList(inFiles_p[fI], "TTree");
    inFilesWeightedCounts[fI] = 0.0;
    
    removeVectorDuplicates(&tempTrees);
 
    //    dumpTreeNames(inFileNames.at(fI), tempTrees);
    fileTrees.push_back(tempTrees);
    misMatchedTrees.push_back({});
  }
  
  std::cout << "Checking tree strings match..." << std::endl;
  for(Int_t fI = 0; fI < nFiles-1; ++fI){
    for(Int_t fI2 = fI+1; fI2 < nFiles; ++fI2){
      if(doStringsMatch(&(fileTrees.at(fI)), &(fileTrees.at(fI2)), &(misMatchedTrees.at(fI)), &(misMatchedTrees.at(fI2)))) std::cout << "All trees match between files!" << std::endl;
      else std::cout << "Mismatched trees found and removed..." << std::endl;
    }
  }
  
  //At this point all ttrees should be matched across tfiles
  //Check the weighttree exists
  if(doWeight){
    bool weightTreeFound = false;
    //Just scan the first file vector   
    for(unsigned int tI = 0; tI < fileTrees[0].size(); ++tI){
      if(isStrSame(fileTrees[0][tI], weightTree)){
	weightTreeFound = true;
	break;
      }
    }

    //If weight TTree not
    if(!weightTreeFound){
      std::cout << "WARNING: Given weightTree \'" << weightTree << "\' is not found in selected files given. Turning off doWeight, no weights will be applied" << std::endl;
      doWeight = false;
    }
    else{//Check the weightVar exists in the ttree
      TTree* checkTree_p = (TTree*)inFiles_p[0]->Get(weightTree.c_str());
      //Just check branches - you should turn your branch/leaf checker into a proper function later
      TObjArray* checkBranchList = (TObjArray*)checkTree_p->GetListOfBranches();

      //Scan branch list for weightVar
      bool weightVarExists = false;
      for(Int_t bI = 0; bI < checkBranchList->GetEntries(); ++bI){
	std::string name = checkBranchList->At(bI)->GetName();
	if(isStrSame(name, weightVar)){
	  weightVarExists = true;
	  break;
	}
      }

      //If weightVar doesn't exist cout a warning and turn weights off
      if(!weightVarExists){
	std::cout << "WARNING: WEIGHTVAR \'" << weightVar << "\' does not exist in given WEIGHTTREE \'" << weightTree << "\'. Recommend checking var. spelling and ttree you think it is in. DOWEIGHT turned to false" << std::endl;
	doWeight = false;
      }
      else{//Lets get the weightVar datatype      
	TLeaf* tempLeaf = (TLeaf*)checkTree_p->GetLeaf(weightVar.c_str());
	std::string tempClassType = tempLeaf->GetTypeName();

	bool varIsVect = tempClassType.find("vector") != std::string::npos;
	if(varIsVect){
	  std::cout << "WARNING: Given weight var \'" << weightVar << "\' is a vector, this feature is only intended to work w/ event by event weights - doWeight set to false" << std::endl;
	  doWeight = false;
	}	
      }
    }

    if(doWeight){//doWeight is still valid - get each files effective event counts
      bool leafHasLength1 = true;

      for(unsigned int fI = 0; fI < fileTrees.size(); ++fI){
	TTree* checkTree_p = (TTree*)inFiles_p[fI]->Get(weightTree.c_str());
	checkTree_p->SetBranchStatus("*", 0);
	checkTree_p->SetBranchStatus(weightVar.c_str(), 1);

	ULong64_t nEntries = checkTree_p->GetEntries();
	if(eventCountOverride > 0){
	  if(((ULong64_t)eventCountOverride) < nEntries) nEntries = eventCountOverride;
	}

	for(ULong64_t entry = 0; entry < nEntries; ++entry){
	  checkTree_p->GetEntry(entry);

	  //Just check the leaf length
	  Int_t currLeafLength = checkTree_p->GetLeaf(weightVar.c_str())->GetLen();
	  leafHasLength1 = currLeafLength == 1;	  
	  if(!leafHasLength1) break;

	  //Leaf length good, process
	  Double_t cutValDouble = checkTree_p->GetLeaf(weightVar.c_str())->GetValue(0);

	  inFilesWeightedCounts[fI] += cutValDouble;
	}
	
	if(!leafHasLength1) break;
      }

      if(!leafHasLength1){
	std::cout << "WARNING: weightVar \'" << weightVar << "\' has leaf length greater than 1. Setting doWeight to false " << std::endl;
	doWeight = false;
      }
    }
  }
  
  //Finally - check treesToSelect is greater than zero, if yes then start applying selections
  if(treesToSelect.size() > 0){
    unsigned int pos = 0;
    while(fileTrees[0].size() > pos){
      bool isGood = false;
      for(unsigned int tI = 0; tI < treesToSelect.size(); ++tI){
	if(isStrSame(treesToSelect[tI], fileTrees[0][pos])){
	  isGood = true;
	  break;
	}
      }
      
      if(isGood) ++pos;
      else{
	for(unsigned int fI = 0; fI < fileTrees.size(); ++fI){
	  fileTrees[fI].erase(fileTrees[fI].begin()+pos);
	}
      }
    }
  }    


  //Check the fileTrees stuff works as expected
  for(unsigned int fI = 0; fI < fileTrees.size(); ++fI){
    dumpTreeNames(inFileNames.at(fI), fileTrees[fI], treesToSelect.size() > 0);
  }

  //Create the opening tex slide - lot of metadata handling
  doFirstTexSlide(&fileTex, inFileNames, inNickNames, fileTrees.at(0), misMatchedTrees, eventCountOverride, footerLine);

  std::vector<int>* intVect_p=NULL;
  std::vector<bool>* boolVect_p=NULL;
  std::vector<char>* charVect_p=NULL;
  std::vector<short>* shortVect_p=NULL;
  std::vector<float>* floatVect_p=NULL;
  std::vector<double>* doubleVect_p=NULL;

  std::vector<std::vector<int>*> intVectCut_p;
  std::vector<std::vector<bool>*> boolVectCut_p;
  std::vector<std::vector<char>*> charVectCut_p;
  std::vector<std::vector<short>*> shortVectCut_p;
  std::vector<std::vector<float>*> floatVectCut_p;
  std::vector<std::vector<double>*> doubleVectCut_p;

  for(unsigned int tI = 0; tI < fileTrees.at(0).size(); ++tI){
    std::cout << "Processing \'" << fileTrees.at(0).at(tI) << "\'..." << std::endl;

    TTree* tree_p[nFiles];
    TObjArray* tempBranchList[nFiles];
    TObjArray* tempLeafList[nFiles];
    std::vector<std::vector<std::string > > branchList;
    std::vector<std::vector<std::string > > leafList;
    std::vector<std::string > pdfList;
    std::vector<std::string> warningList;
    std::vector<int> warningListPos;       

    for(Int_t fI = 0; fI < nFiles; ++fI){
      tree_p[fI] = (TTree*)inFiles_p[fI]->Get(fileTrees.at(0).at(tI).c_str());
      tempBranchList[fI] = (TObjArray*)(tree_p[fI]->GetListOfBranches()->Clone());
      tempLeafList[fI] = (TObjArray*)(tree_p[fI]->GetListOfLeaves()->Clone());

      std::vector<std::string> uncleanedBranches;
      std::vector<std::string> uncleanedLeaves;

      for(Int_t tempI = 0; tempI < tempBranchList[fI]->GetEntries(); ++tempI){
	uncleanedBranches.push_back(tempBranchList[fI]->At(tempI)->GetName());
      }

      for(Int_t tempI = 0; tempI < tempLeafList[fI]->GetEntries(); ++tempI){
	uncleanedLeaves.push_back(tempLeafList[fI]->At(tempI)->GetName());
      }

      unsigned int tempPos = 0;
      while(tempPos < uncleanedBranches.size()){
	TBranch* tempBranch = (TBranch*)tree_p[fI]->GetBranch(uncleanedBranches.at(tempPos).c_str());

	if(tempBranch->GetListOfBranches()->GetEntries() != 0){
	  std::cout << "WARNING: Branch \'" << tempBranch->GetName() << "\' in TTree \'" << fileTrees.at(0).at(tI) << "\' has nSubBranch = " << tempBranch->GetListOfBranches()->GetEntries() << " not equal to 0. runForestDQM.exe cannot handle this branch, will be removed" << std::endl;
	  
	  for(Int_t rbI = 0; rbI < tempBranch->GetListOfBranches()->GetEntries()+1; ++rbI){
	    std::cout << "Removing \'" << uncleanedLeaves.at(tempPos) << "\' from leaflist at " << tempPos << std::endl;
	    uncleanedLeaves.erase(uncleanedLeaves.begin()+tempPos);
	  }
	  uncleanedBranches.erase(uncleanedBranches.begin()+tempPos);
	}
	else tempPos++;
      }
    
      if(uncleanedBranches.size() != uncleanedLeaves.size()){
	std::cout << "WARNING: branchList in TTree \'" << fileTrees.at(0).at(tI) << "\' has nBranches = " << uncleanedBranches.size() << " after cleaning, differing from nLeaves = " << uncleanedLeaves.size() << "." << std::endl;
      }

      branchList.push_back({});
      leafList.push_back({});
    
      for(unsigned int bI1 = 0; bI1 < uncleanedBranches.size(); ++bI1){
	std::string branchName = uncleanedBranches.at(bI1);
	std::string leafName = uncleanedLeaves.at(bI1);

	if(!isStrSame(branchName, leafName)){
	  std::cout << "WARNING: Branch \'" << branchName << "\' differs from matched lea  \'" << leafName << "\'." << std::endl;
	}

	if(mapOfVarToSkip.count(branchName) > 0) continue;

	branchList.at(fI).push_back(branchName);
	leafList.at(fI).push_back(leafName);
      }
    }

    for(unsigned int bI = 0; bI < branchList.at(0).size(); ++bI){
      std::cout << " " << bI << "/" << branchList.at(0).size() << ": " << branchList.at(0).at(bI) << std::endl;
    }

    std::vector<std::string> misMatchedNickNames;
    std::vector<std::vector<std::string > > misMatchedBranches;
    std::vector<std::vector<std::string > > misMatchedLeaves;

    for(Int_t fI = 0; fI < nFiles; ++fI){
      misMatchedBranches.push_back({});
      misMatchedLeaves.push_back({});
    }

    std::cout << "Checking branch strings match..." << std::endl;
  
    for(Int_t fI = 0; fI < nFiles-1; ++fI){
      for(Int_t fI2 = fI+1; fI2 < nFiles; ++fI2){
	if(doStringsMatch(&(branchList.at(fI)), &(branchList.at(fI2)), &(misMatchedBranches.at(fI)), &(misMatchedBranches.at(fI2)))) std::cout << "All branches match between files!" << std::endl;
	else std::cout << "Mismatched branches found and removed..." << std::endl;

	if(doStringsMatch(&(leafList.at(fI)), &(leafList.at(fI2)), &(misMatchedLeaves.at(fI)), &(misMatchedLeaves.at(fI2)))) std::cout << "All leaves match between files!" << std::endl;
	else std::cout << "Mismatched leaves found and removed..." << std::endl;
      }
    }
    
    std::vector<bool> pairVars1Found;
    std::vector<bool> pairVars2Found;
    std::vector<bool> pairCutVarsFound;
  
    for(unsigned int i = 0; i < pairVars1.size(); ++i){
      pairVars1Found.push_back(false);
      pairVars2Found.push_back(false);
      pairCutVarsFound.push_back(false);
    }
  
    for(unsigned int bI1 = 0; bI1 < branchList.at(0).size(); ++bI1){
      std::cout << " Processing \'" << branchList.at(0).at(bI1) << "\'..." << std::endl;

      tree_p[0]->ResetBranchAddresses();
      tree_p[0]->SetBranchStatus("*", 0);
      tree_p[0]->SetBranchStatus(branchList.at(0).at(bI1).c_str(), 1);

      TLeaf* tempLeaf = (TLeaf*)tree_p[0]->GetLeaf(leafList.at(0).at(bI1).c_str());
      std::string tempClassType = tempLeaf->GetTypeName();
      bool varIsVect = tempClassType.find("vector") != std::string::npos;

      std::cout << "  Branch class: " << tempClassType << std::endl;
      
      bool maxValIsSet = false;      
      bool minValIsSet = false;      
      Double_t maxVal = -999999;
      Double_t minVal = 999999;

      bool isBranchCut = mapFromVarToCut.count(branchList.at(0).at(bI1)) != 0;
      bool firstFill = true;
      std::vector<std::string> cutStr;
      std::vector<std::string> cutVar;
      std::vector<bool> isCutAbs;
      std::vector<int> cutVarLastPos;      
      std::vector<bool> doLong;

      if(isBranchCut){
	cutStr = mapFromVarToCut[branchList.at(0).at(bI1)];
	cutVar = mapFromVarToCutVar[branchList.at(0).at(bI1)];
	isCutAbs = mapFromVarToIsCutAbs[branchList.at(0).at(bI1)];
      
	for(unsigned int cI = 0; cI < cutStr.size(); ++cI){
	  if(cutStr[cI].find("==") != std::string::npos || cutStr[cI].find("==") != std::string::npos) doLong.push_back(true);
	  else doLong.push_back(false);

	  cutVarLastPos.push_back(cI);

	  for(unsigned int cI2 = cI; cI2 < cutVar.size(); ++cI2){
	    if(isStrSame(cutVar[cI], cutVar[cI2])){
	      cutVarLastPos[cI] = cI2;
	    }
	  }
      
	  //	  std::cout << "BRANCHLIST, CUTVAR: " << branchList.at(0).at(bI1) << ", " << cutVar[cI] << std::endl;
	  if(isStrSame(branchList.at(0).at(bI1), cutVar[cI])){
	    std::cout << branchList.at(0).at(bI1) << ", " << cutVar[cI] << ", " << cutStr[cI] << std::endl;

	    std::string tempCutStr = cutStr[cI];
	    if(!isCutAbs[cI]){
	      tempCutStr.replace(tempCutStr.find(cutVar[cI]), cutVar[cI].size(), "");

	      if(tempCutStr.find(">") != std::string::npos){
		tempCutStr.replace(0, tempCutStr.find(">")+1, "");
		if(tempCutStr.substr(0,1).find("=") != std::string::npos) tempCutStr.replace(0,1,"");
		minVal = std::stod(tempCutStr);
		minValIsSet = true;
	      }
	      else if(tempCutStr.find("<") != std::string::npos){
		tempCutStr.replace(0, tempCutStr.find("<")+1, "");
		if(tempCutStr.substr(0,1).find("=") != std::string::npos) tempCutStr.replace(0,1,"");
		maxVal = std::stod(tempCutStr);
		maxValIsSet = true;
	      }
	    }
	    else if(tempCutStr.find("<") != std::string::npos){
	      tempCutStr.replace(0, tempCutStr.find("<")+1, "");
	      if(tempCutStr.substr(0,1).find("=") != std::string::npos) tempCutStr.replace(0,1,"");
	      maxVal = std::stod(tempCutStr);
	      maxValIsSet = true;
	      minVal = -maxVal;
	      minValIsSet = true;
	    }	    
	  }
	}
      }
   
      std::string maxValFile = inFileNames.at(0);
      std::string minValFile = inFileNames.at(0);  

      if(!minValIsSet || !maxValIsSet){
	if(!varIsVect){	
	  for(Int_t fI = 0; fI < nFiles; ++fI){
	    tree_p[fI]->ResetBranchAddresses();
	    tree_p[fI]->SetBranchStatus("*", 0);
	    tree_p[fI]->SetBranchStatus(branchList.at(0).at(bI1).c_str(), 1);
	    
	    if(!isBranchCut){
	      Double_t tempMaxVal = tree_p[fI]->GetMaximum(branchList.at(0).at(bI1).c_str());
	      Double_t tempMinVal = tree_p[fI]->GetMinimum(branchList.at(0).at(bI1).c_str());
	      
	      if(tempMaxVal > maxVal){
		maxVal = tempMaxVal;
		maxValFile = inFileNames.at(fI);
	      }
	      
	      if(tempMinVal < minVal){
		minVal = tempMinVal;
		minValFile = inFileNames.at(fI);
	      }
	    }
	    else{
	      Int_t nEntriesTemp = tree_p[fI]->GetEntries();
	      if(eventCountOverride > 0) nEntriesTemp =  eventCountOverride;
	      
	      for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		tree_p[fI]->SetBranchStatus(cutVar[cI].c_str(), 1);
	      }
	      
	      for(Int_t entry = 0; entry < nEntriesTemp; ++entry){
		tree_p[fI]->GetEntry(entry);
		
		for(Int_t lI = 0; lI < tree_p[fI]->GetLeaf(cutVar[0].c_str())->GetLen(); ++lI){
		  bool cutPass = true;
		  
		  for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		    Long64_t cutValLong64 = tree_p[fI]->GetLeaf(cutVar[cI].c_str())->GetValueLong64(lI);
		    Double_t cutValDouble = tree_p[fI]->GetLeaf(cutVar[cI].c_str())->GetValue(lI);
		    
		    if(doLong[cI]) cutPass = valPassesCut(cutValLong64, cutStr[cI], isCutAbs[cI]);
		    else cutPass = valPassesCut(cutValDouble, cutStr[cI], isCutAbs[cI]);
		    
		    if(!cutPass) break;		  
		  }
		  
		  if(!cutPass) continue;
		  
		  Double_t varVal = tree_p[fI]->GetLeaf(branchList.at(0).at(bI1).c_str())->GetValue(lI);	
		  if(firstFill == true){
		    maxVal = varVal;
		    minVal = varVal;
		    firstFill = false;
		  }
		  else{	 
		    if(maxVal < varVal) maxVal = varVal;
		    if(minVal > varVal) minVal = varVal;
		  }
		}
	      }
	    }
	  }
	}
	else if(varIsVect && tempClassType.find("vector<vector") == std::string::npos){
	  //	std::cout << "Searching for vector max/min of branch \'" << branchList.at(0).at(bI1) << "\'" << std::endl;
	  
	  std::vector<bool> cutIsInt;
	  std::vector<bool> cutIsBool;
	  std::vector<bool> cutIsChar;
	  std::vector<bool> cutIsShort;
	  std::vector<bool> cutIsDouble;
	  std::vector<bool> cutIsFloat;
	  
	  for(unsigned int cI = 0; cI < cutStr.size(); ++cI){
	    if(isBranchCut){
	      if(!isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
		TLeaf* tempLeafCut = (TLeaf*)tree_p[0]->GetLeaf(cutVar[cI].c_str());
		std::string tempClassTypeCut = tempLeafCut->GetTypeName();
		cutIsInt.push_back(tempClassTypeCut.find("int") != std::string::npos);
		cutIsBool.push_back(tempClassTypeCut.find("bool") != std::string::npos);
		cutIsChar.push_back(tempClassTypeCut.find("char") != std::string::npos);
		cutIsShort.push_back(tempClassTypeCut.find("short") != std::string::npos);
		cutIsFloat.push_back(tempClassTypeCut.find("float") != std::string::npos);
		cutIsDouble.push_back(tempClassTypeCut.find("double") != std::string::npos);
	      }
	      else{
		cutIsInt.push_back(false);
		cutIsBool.push_back(false);
		cutIsChar.push_back(false);
		cutIsShort.push_back(false);
		cutIsFloat.push_back(false);
		cutIsDouble.push_back(false);
	      }
	      
	      intVectCut_p.push_back(nullptr);
	      boolVectCut_p.push_back(nullptr);
	      charVectCut_p.push_back(nullptr);
	      shortVectCut_p.push_back(nullptr);
	      floatVectCut_p.push_back(nullptr);
	      doubleVectCut_p.push_back(nullptr);
	    }
	  }
		  
	  bool isFirstFound = false;
	  if(tempClassType.find("int") != std::string::npos){
	    for(Int_t fI = 0; fI < nFiles; ++fI){
	      tree_p[fI]->SetBranchStatus("*", 0);
	      tree_p[fI]->SetBranchStatus(branchList.at(0).at(bI1).c_str(), 1);
	      
	      tree_p[fI]->SetBranchAddress(branchList.at(0).at(bI1).c_str(), &intVect_p);
	   
	      if(isBranchCut){
		for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		  if(!isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
		    tree_p[fI]->SetBranchStatus(cutVar[cI].c_str(), 1);       
		    
		    if(cutIsInt[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(intVectCut_p[cI]));
		    else if(cutIsBool[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(boolVectCut_p[cI]));
		    else if(cutIsChar[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(charVectCut_p[cI]));
		    else if(cutIsShort[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(shortVectCut_p[cI]));
		    else if(cutIsDouble[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(doubleVectCut_p[cI]));
		    else if(cutIsFloat[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(floatVectCut_p[cI]));
		  }
		}
	      }	   
	      
	      Int_t nEntries1 = tree_p[fI]->GetEntries();
	      if(eventCountOverride > 0) nEntries1 = eventCountOverride;
	      
	      int startPos = 0;
	      while(!isFirstFound && startPos < nEntries1){
		tree_p[fI]->GetEntry(startPos);
		
		++startPos;
		
		for(unsigned int vI = 0; vI < intVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut((Long64_t)intVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]){
			  passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			}
		      }
		      
		      if(!passes) break;		    
		    }
		  }
		  
		  if(!passes) continue;

		  if(!isFirstFound){
		    minVal = intVect_p->at(vI);
		    maxVal = intVect_p->at(vI);
		    minValFile = inFileNames.at(fI);
		    maxValFile = inFileNames.at(fI);
		  }
		  else{
		    if(intVect_p->at(vI) > maxVal){
		      maxVal = intVect_p->at(vI);
		      maxValFile = inFileNames.at(fI);		  
		    }
		    if(intVect_p->at(vI) < minVal){
		      minVal = intVect_p->at(vI);
		      minValFile = inFileNames.at(fI);		  
		    }
		  }
		  
		  isFirstFound = true;
		}
	      }
	      
	      
	      for(Int_t entry = startPos; entry < nEntries1; ++entry){
		tree_p[fI]->GetEntry(entry);
		
		for(unsigned int vI = 0; vI < intVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){		  
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut((Long64_t)intVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;
		    }
		  }
		  
		  if(!passes) continue;

		  if(intVect_p->at(vI) > maxVal){
		    maxVal = intVect_p->at(vI);
		    maxValFile = inFileNames.at(fI);		  
		  }
		  if(intVect_p->at(vI) < minVal){
		    minVal = intVect_p->at(vI);
		    minValFile = inFileNames.at(fI);		  
		  }
		}
	      }
	    }
	  }
	  else if(tempClassType.find("bool") != std::string::npos){
	    for(Int_t fI = 0; fI < nFiles; ++fI){
	      tree_p[fI]->SetBranchStatus("*", 0);
	      tree_p[fI]->SetBranchStatus(branchList.at(0).at(bI1).c_str(), 1);
	      
	      tree_p[fI]->SetBranchAddress(branchList.at(0).at(bI1).c_str(), &boolVect_p);
	      
	      if(isBranchCut){ 
		for(unsigned int cI = 0; cI < cutVar.size(); ++cI){		  
		  
		  if(!isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
		    tree_p[fI]->SetBranchStatus(cutVar[cI].c_str(), 1);
		    
		    if(cutIsInt[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(intVectCut_p[cI]));
		    else if(cutIsBool[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(boolVectCut_p[cI]));
		    else if(cutIsChar[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(charVectCut_p[cI]));
		    else if(cutIsShort[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(shortVectCut_p[cI]));
		    else if(cutIsDouble[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(doubleVectCut_p[cI]));
		    else if(cutIsFloat[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(floatVectCut_p[cI]));
		  }
		}
	      }
	      
	      Int_t nEntries1 = tree_p[fI]->GetEntries();
	      if(eventCountOverride > 0) nEntries1 = eventCountOverride;
	      
	      int startPos = 0;
	      while(!isFirstFound && startPos < nEntries1){
		tree_p[fI]->GetEntry(startPos);
		
		++startPos;
		
		for(unsigned int vI = 0; vI < boolVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		      
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut((Long64_t)boolVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;
		    }
		  }
		  
		  if(!passes) continue;
		  
		  if(!isFirstFound){
		    minVal = boolVect_p->at(vI);
		    maxVal = boolVect_p->at(vI);
		    minValFile = inFileNames.at(fI);
		    maxValFile = inFileNames.at(fI);
		  }
		  else{
		    if(boolVect_p->at(vI) > maxVal){
		      maxVal = boolVect_p->at(vI);
		      maxValFile = inFileNames.at(fI);		  
		    }
		    if(boolVect_p->at(vI) < minVal){
		      minVal = boolVect_p->at(vI);
		    minValFile = inFileNames.at(fI);		  
		    }
		  }
		  
		  isFirstFound = true;
		}
	      }
	      
	      for(Int_t entry = startPos; entry < nEntries1; ++entry){
		tree_p[fI]->GetEntry(entry);
		
		for(unsigned int vI = 0; vI < boolVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut((Long64_t)boolVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;
		    }
		  }
		  
		  if(!passes) continue;
		  
		  if(boolVect_p->at(vI) > maxVal){
		    maxVal = boolVect_p->at(vI);
		    maxValFile = inFileNames.at(fI);		  
		  }
		  if(boolVect_p->at(vI) < minVal){
		    minVal = boolVect_p->at(vI);
		    minValFile = inFileNames.at(fI);		  
		  }
		}
	      }
	    }
	  }
	  else if(tempClassType.find("char") != std::string::npos){
	    for(Int_t fI = 0; fI < nFiles; ++fI){
	      tree_p[fI]->SetBranchStatus("*", 0);
	      tree_p[fI]->SetBranchStatus(branchList.at(0).at(bI1).c_str(), 1);
	      
	      tree_p[fI]->SetBranchAddress(branchList.at(0).at(bI1).c_str(), &charVect_p);
	      
	      if(isBranchCut){
		
		for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		  if(!isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
		    tree_p[fI]->SetBranchStatus(cutVar[cI].c_str(), 1);
		    
		    if(cutIsInt[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(intVectCut_p[cI]));
		    else if(cutIsBool[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(boolVectCut_p[cI]));
		    else if(cutIsChar[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(charVectCut_p[cI]));
		    else if(cutIsShort[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(shortVectCut_p[cI]));
		    else if(cutIsDouble[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(doubleVectCut_p[cI]));
		    else if(cutIsFloat[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(floatVectCut_p[cI]));
		  }
		}
	      }
	      
	      Int_t nEntries1 = tree_p[fI]->GetEntries();
	      if(eventCountOverride > 0) nEntries1 = eventCountOverride;
	      
	      int startPos = 0;
	      while(!isFirstFound && startPos < nEntries1){
		tree_p[fI]->GetEntry(startPos);
		
		++startPos;
		
		for(unsigned int vI = 0; vI < charVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		      
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut((Long64_t)charVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;
		    }
		  }
		  
		  if(!passes) continue;

		  if(!isFirstFound){
		    minVal = charVect_p->at(vI);
		    maxVal = charVect_p->at(vI);
		    minValFile = inFileNames.at(fI);
		    maxValFile = inFileNames.at(fI);
		  }
		  else{
		    if(charVect_p->at(vI) > maxVal){
		      maxVal = charVect_p->at(vI);
		      maxValFile = inFileNames.at(fI);		  
		    }
		    if(charVect_p->at(vI) < minVal){
		      minVal = charVect_p->at(vI);
		      minValFile = inFileNames.at(fI);		  
		    }
		  }
		  
		  isFirstFound = true;
		}
	      }
	      
	      for(Int_t entry = startPos; entry < nEntries1; ++entry){
		tree_p[fI]->GetEntry(entry);
		
		for(unsigned int vI = 0; vI < charVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut((Long64_t)charVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;		    
		    }
		  }
		  
		  if(!passes) continue;

		  if(charVect_p->at(vI) > maxVal){
		    maxVal = charVect_p->at(vI);
		    maxValFile = inFileNames.at(fI);		  
		  }
		  if(charVect_p->at(vI) < minVal){
		    minVal = charVect_p->at(vI);
		    minValFile = inFileNames.at(fI);		  
		  }
		}
	      }
	    }
	  }
	  else if(tempClassType.find("short") != std::string::npos){
	    for(Int_t fI = 0; fI < nFiles; ++fI){
	      tree_p[fI]->SetBranchStatus("*", 0);
	      tree_p[fI]->SetBranchStatus(branchList.at(0).at(bI1).c_str(), 1);
	      
	      tree_p[fI]->SetBranchAddress(branchList.at(0).at(bI1).c_str(), &shortVect_p);
	      
	      if(isBranchCut){
		
		for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		  if(!isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
		    tree_p[fI]->SetBranchStatus(cutVar[cI].c_str(), 1);
		    
		    if(cutIsInt[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(intVectCut_p[cI]));
		    else if(cutIsBool[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(boolVectCut_p[cI]));
		    else if(cutIsChar[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(charVectCut_p[cI]));
		    else if(cutIsShort[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(shortVectCut_p[cI]));
		    else if(cutIsDouble[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(doubleVectCut_p[cI]));
		    else if(cutIsFloat[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(floatVectCut_p[cI]));
		  }
		}
	      }
	      
	      Int_t nEntries1 = tree_p[fI]->GetEntries();
	      if(eventCountOverride > 0) nEntries1 = eventCountOverride;
	      
	      int startPos = 0;
	      while(!isFirstFound && startPos < nEntries1){
		tree_p[fI]->GetEntry(startPos);
		
		++startPos;
		
		for(unsigned int vI = 0; vI < shortVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		      
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut((Long64_t)shortVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;
		    }
		  }
		  
		  if(!passes) continue;

		  if(!isFirstFound){
		    minVal = shortVect_p->at(vI);
		    maxVal = shortVect_p->at(vI);
		    minValFile = inFileNames.at(fI);
		    maxValFile = inFileNames.at(fI);
		  }
		  else{
		    if(shortVect_p->at(vI) > maxVal){
		      maxVal = shortVect_p->at(vI);
		      maxValFile = inFileNames.at(fI);		  
		    }
		    if(shortVect_p->at(vI) < minVal){
		      minVal = shortVect_p->at(vI);
		      minValFile = inFileNames.at(fI);		  
		    }
		  }
		  
		  isFirstFound = true;
		}
	      }
	      
	      for(Int_t entry = startPos; entry < nEntries1; ++entry){
		tree_p[fI]->GetEntry(entry);
		
		for(unsigned int vI = 0; vI < shortVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut((Long64_t)shortVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;		    
		    }
		  }
		  
		  if(!passes) continue;

		  if(shortVect_p->at(vI) > maxVal){
		    maxVal = shortVect_p->at(vI);
		    maxValFile = inFileNames.at(fI);		  
		  }
		  if(shortVect_p->at(vI) < minVal){
		    minVal = shortVect_p->at(vI);
		    minValFile = inFileNames.at(fI);		  
		  }
		}
	      }
	    }
	  }
	  else if(tempClassType.find("float") != std::string::npos){
	    for(Int_t fI = 0; fI < nFiles; ++fI){
	      tree_p[fI]->SetBranchStatus("*", 0);
	      tree_p[fI]->SetBranchStatus(branchList.at(0).at(bI1).c_str(), 1);
	    
	      tree_p[fI]->SetBranchAddress(branchList.at(0).at(bI1).c_str(), &floatVect_p);
	      
	      if(isBranchCut){
		for(unsigned int cI = 0; cI < cutVar.size(); ++cI){
		  if(!isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
		    tree_p[fI]->SetBranchStatus(cutVar[cI].c_str(), 1);
		    
		    if(cutIsInt[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(intVectCut_p[cI]));
		    else if(cutIsBool[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(boolVectCut_p[cI]));
		    else if(cutIsChar[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(charVectCut_p[cI]));
		    else if(cutIsShort[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(shortVectCut_p[cI]));
		    else if(cutIsDouble[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(doubleVectCut_p[cI]));
		    else if(cutIsFloat[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(floatVectCut_p[cI]));
		  }
		}
	      }
	      
	      Int_t nEntries1 = tree_p[fI]->GetEntries();
	      if(eventCountOverride > 0) nEntries1 = eventCountOverride;
	      
	      int startPos = 0;
	      while(!isFirstFound && startPos < nEntries1){
		tree_p[fI]->GetEntry(startPos);
		
		++startPos;
		
		for(unsigned int vI = 0; vI < floatVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){		  
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut((Double_t)floatVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;
		    }
		  }
		  
		  if(!passes) continue;

		  //Try to avoid infs - some infs can be stored in TTrees
		  if(std::isnan(floatVect_p->at(vI))) continue;
		  if(std::isinf(floatVect_p->at(vI))) continue;
		  
		  if(!isFirstFound){
		    minVal = floatVect_p->at(vI);
		    maxVal = floatVect_p->at(vI);
		    minValFile = inFileNames.at(fI);
		    maxValFile = inFileNames.at(fI);
		  }
		  else{
		    if(floatVect_p->at(vI) > maxVal){
		      maxVal = floatVect_p->at(vI);
		      maxValFile = inFileNames.at(fI);		  
		    }
		    if(floatVect_p->at(vI) < minVal){
		      minVal = floatVect_p->at(vI);
		      minValFile = inFileNames.at(fI);		  
		    }
		  }
		  
		  isFirstFound = true;
		}
	      }
	      
	      for(Int_t entry = startPos; entry < nEntries1; ++entry){
		tree_p[fI]->GetEntry(entry);
		
		for(unsigned int vI = 0; vI < floatVect_p->size(); ++vI){
		  bool passes = true;

		  if(isBranchCut){
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){		  
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut((Double_t)floatVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;
		    }				  
		  }
		  
		  if(!passes) continue;

		  //Try to avoid infs - some infs can be stored in TTrees
		  if(std::isnan(floatVect_p->at(vI))) continue;
		  if(std::isinf(floatVect_p->at(vI))) continue;
		  
		  if(floatVect_p->at(vI) > maxVal){
		    maxVal = floatVect_p->at(vI);
		    maxValFile = inFileNames.at(fI);		  
		  }
		  if(floatVect_p->at(vI) < minVal){
		    minVal = floatVect_p->at(vI);
		    minValFile = inFileNames.at(fI);		  
		  }
		}	      
	      }
	    }	 
	  }
	  else if(tempClassType.find("double") != std::string::npos){
	    for(Int_t fI = 0; fI < nFiles; ++fI){
	      tree_p[fI]->SetBranchStatus("*", 0);
	      tree_p[fI]->SetBranchStatus(branchList.at(0).at(bI1).c_str(), 1);
	      
	      tree_p[fI]->SetBranchAddress(branchList.at(0).at(bI1).c_str(), &doubleVect_p);
	      
	      if(isBranchCut){
		
		for(unsigned int cI = 0; cI < cutVar.size(); ++cI){		  
		  if(!isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
		    tree_p[fI]->SetBranchStatus(cutVar[cI].c_str(), 1);
		    
		    if(cutIsInt[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(intVectCut_p[cI]));
		    else if(cutIsBool[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(boolVectCut_p[cI]));
		    else if(cutIsChar[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(charVectCut_p[cI]));
		    else if(cutIsShort[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(shortVectCut_p[cI]));
		    else if(cutIsDouble[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(doubleVectCut_p[cI]));
		    else if(cutIsFloat[cI]) tree_p[fI]->SetBranchAddress(cutVar[cI].c_str(), &(floatVectCut_p[cI]));
		  }
		}
	      }
	      
	      Int_t nEntries1 = tree_p[fI]->GetEntries();
	      if(eventCountOverride > 0) nEntries1 = eventCountOverride;
	      
	      int startPos = 0;
	      while(!isFirstFound && startPos < nEntries1){
		tree_p[fI]->GetEntry(startPos);
		
		++startPos;
		
		for(unsigned int vI = 0; vI < doubleVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){		  
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut(doubleVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;
		    }
		  }
		  
		  if(!passes) continue;

		  //Try to avoid infs - some infs can be stored in TTrees
		  if(std::isnan(doubleVect_p->at(vI))) continue;
		  if(std::isinf(doubleVect_p->at(vI))) continue;
		  
		  if(!isFirstFound){
		    minVal = doubleVect_p->at(vI);
		    maxVal = doubleVect_p->at(vI);
		    minValFile = inFileNames.at(fI);
		    maxValFile = inFileNames.at(fI);
		  }
		  else{
		    if(doubleVect_p->at(vI) > maxVal){
		      maxVal = doubleVect_p->at(vI);
		      maxValFile = inFileNames.at(fI);		  
		    }
		    if(doubleVect_p->at(vI) < minVal){
		      minVal = doubleVect_p->at(vI);
		      minValFile = inFileNames.at(fI);		  
		    }
		  }
		  
		  isFirstFound = true;
		}
	      }
	      
	      for(Int_t entry = startPos; entry < nEntries1; ++entry){
		tree_p[fI]->GetEntry(entry);
		
		for(unsigned int vI = 0; vI < doubleVect_p->size(); ++vI){
		  bool passes = true;
		  
		  if(isBranchCut){
		    for(unsigned int cI = 0; cI < cutVar.size(); ++cI){		  
		      if(isStrSame(cutVar[cI], branchList.at(0).at(bI1))){
			passes = valPassesCut(doubleVect_p->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      else{
			if(cutIsInt[cI]) passes = valPassesCut((Long64_t)intVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsBool[cI]) passes = valPassesCut((Long64_t)boolVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsChar[cI]) passes = valPassesCut((Long64_t)charVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsShort[cI]) passes = valPassesCut((Long64_t)shortVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsDouble[cI]) passes = valPassesCut(doubleVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
			else if(cutIsFloat[cI]) passes = valPassesCut((Double_t)floatVectCut_p[cutVarLastPos[cI]]->at(vI), cutStr[cI], isCutAbs[cI]);
		      }
		      
		      if(!passes) break;
		    }
		  }
		  
		  if(!passes) continue;

		  //Try to avoid infs - some infs can be stored in TTrees
		  if(std::isnan(doubleVect_p->at(vI))) continue;
		  if(std::isinf(doubleVect_p->at(vI))) continue;
		  
		  if(doubleVect_p->at(vI) > maxVal){
		    maxVal = doubleVect_p->at(vI);
		    maxValFile = inFileNames.at(fI);		  
		  }
		  if(doubleVect_p->at(vI) < minVal){
		    minVal = doubleVect_p->at(vI);
		    minValFile = inFileNames.at(fI);		  
		  }
		}
	      }
	    }
	  }
	}
	else{
	  std::cout << "WARNING: do not know how to handle vector \'" << tempClassType << "\'. return 1" << std::endl;
	  return 1;
	}      
      }
    
    
      if(branchList.at(0).at(bI1).find("jteta") != std::string::npos){
	std::cout << "CHECK 2 BRANCH MIN-MAX: " << branchList.at(0).at(bI1) << ", " << minVal << "-" << maxVal << std::endl;
      }

      
      std::vector<std::string> histNames;
      for(Int_t fI = 0; fI < nFiles; ++fI){
	std::string histName = fileTrees.at(0).at(tI) + "_" + branchList.at(0).at(bI1) + "_" + inNickNames.at(fI);
	while(histName.find("/") != std::string::npos){histName.replace(histName.find("/"), 1, "_");}
	histNames.push_back(histName);
      }

      const Float_t padSplit = 0.35;
      const Float_t padLeftMargin = 0.12;
      const Float_t titleOffset = 1.2;
      
      TCanvas* canv_p = new TCanvas("temp", "temp", 900, 800);
      canv_p->SetTopMargin(0.01);
      canv_p->SetRightMargin(0.01);
      canv_p->SetBottomMargin(0.01);
      canv_p->SetLeftMargin(0.01);
      TPad* pads_p[2];
      pads_p[0] = new TPad("pads0", "", 0.0, padSplit, 1.0, 1.0);
      canv_p->cd();
      pads_p[0]->Draw("SAME");
      pads_p[0]->SetTopMargin(0.4*padLeftMargin/(1.0-padSplit));
      pads_p[0]->SetLeftMargin(padLeftMargin);
      pads_p[0]->SetRightMargin(0.5*padLeftMargin);
      pads_p[0]->SetBottomMargin(0.0);

      pads_p[1] = new TPad("pads1", "", 0.0, 0.00, 1.0, padSplit);
      canv_p->cd();
      pads_p[1]->Draw("SAME");
      pads_p[1]->SetTopMargin(0.0);
      pads_p[1]->SetLeftMargin(padLeftMargin);
      pads_p[1]->SetRightMargin(0.5*padLeftMargin);
      pads_p[1]->SetBottomMargin(0.8*padLeftMargin/padSplit);

      if(TMath::Abs(minVal - maxVal) < 0.000000001){
	minVal -= 1;
	maxVal +=1;
      }
    
      TH1D* tempHistForCanvas_p[nFiles];
      TH1D* tempHist_p[nFiles];

      Double_t bins[nMaxBins+1];
      //      bool doLogX = maxVal - minVal > 300 && minVal > 0;

      for(unsigned int pI = 0; pI < pairVars1.size(); ++pI){
	if(isStrSame(branchList.at(0).at(bI1), pairVars1.at(pI))){
	  pairVars1Found.at(pI) = true;
	  pairVarsMax1.at(pI) = maxVal;
	  pairVarsMin1.at(pI) = minVal;

	  if(false){
	    if(isStrSame(pairVars1.at(pI), "pfPt")){
	      pairVarsMax1.at(pI) = 10.0;
	    }
	    else if(isStrSame(pairVars1.at(pI), "pfE")){
	      pairVarsMax1.at(pI) = 10.0;
	    }
	    else if(isStrSame(pairVars1.at(pI), "pfEta")){
	      pairVarsMax1.at(pI) = 3.0;
	      pairVarsMin1.at(pI) = -3.0;
	    }
	    else if(isStrSame(pairVars1.at(pI), "jtpt")){
	      pairVarsMax1.at(pI) = 100.0;
	    }
	    else if(isStrSame(pairVars1.at(pI), "jteta")){
	      pairVarsMax1.at(pI) = 3.0;
	      pairVarsMin1.at(pI) = -3.0;
	    }
	  }
	}
	else if(isStrSame(branchList.at(0).at(bI1), pairVars2.at(pI))){
	  pairVars2Found.at(pI) = true;
	  pairVarsMax2.at(pI) = maxVal;
	  pairVarsMin2.at(pI) = minVal;

	  if(false){
	    if(isStrSame(pairVars2.at(pI), "pfPt")){
	      pairVarsMax2.at(pI) = 10.0;
	    }
	    else if(isStrSame(pairVars2.at(pI), "pfE")){
	      pairVarsMax2.at(pI) = 10.0;
	    }
	    else if(isStrSame(pairVars2.at(pI), "pfEta")){
	      pairVarsMax2.at(pI) = 3.0;
	      pairVarsMin2.at(pI) = -3.0;
	    }
	    else if(isStrSame(pairVars1.at(pI), "jtpt")){
	      pairVarsMax1.at(pI) = 100.0;
	    }
	    else if(isStrSame(pairVars1.at(pI), "jteta")){
	      pairVarsMax1.at(pI) = 3.0;
	      pairVarsMin1.at(pI) = -3.0;
	    }
	  }
	}
	else if(isStrSame(branchList.at(0).at(bI1), pairCutVars.at(pI))) pairCutVarsFound.at(pI) = true;
      }

      bool doLogX = getDoLog(minVal, maxVal, 2);
      if(doLogXOverrideMap.count(branchList.at(0).at(bI1)) > 0) doLogX = doLogXOverrideMap[branchList.at(0).at(bI1)];
      
      Float_t minValXForHist = minVal;
      Float_t maxValXForHist = maxVal;

      if(doLogX){
	maxValXForHist *= 2;
	minValXForHist /= 2;
      }      
      else{	
	Double_t interval = maxVal - minVal;
	maxValXForHist += interval/10.;
	minValXForHist -= interval/10.;	
      }

      Int_t nBinsActual = nMaxBins;
      if(tempClassType.find("int") != std::string::npos || tempClassType.find("char") != std::string::npos || tempClassType.find("short") != std::string::npos || tempClassType.find("bool") != std::string::npos || tempClassType == "Int_t"){
	maxVal += 0.5;
	minVal -= 0.5;

	//set it to the values of the ints, centered on bin values
	nBinsActual = maxVal - minVal;

	if(maxValXForHist < maxVal) maxValXForHist = maxVal;
	if(minValXForHist > minVal) minValXForHist = minVal;

      }    

      //Check bin override map first
      if(nBinOverrideMap.count(branchList.at(0).at(bI1)) > 0) nBinsActual = nBinOverrideMap[branchList.at(0).at(bI1)];

      //check if int or Int_t can be reduced to something reasonable that maintains even int-like coverage by dividing cleanly
      if(nBinsActual > nMaxBins){
	if(tempClassType.find("int") != std::string::npos || tempClassType.find("Int_t") != std::string::npos){
	  for(unsigned int divVal = 2; divVal <= 10; ++divVal){
	    if(nBinsActual%divVal == 0){
	      if(nBinsActual/divVal <= nMaxBins){
		nBinsActual /= divVal;
		break;
	      }
	    }
	  }	
	}
      }

      //If we still exceed nMaxBins, set to nMaxBins
      if(nBinsActual > nMaxBins){
	std::cout << "REQUESTED NUMBER OF BINS, " << nBinsActual << ", FOR \'" << branchList.at(0).at(bI1) << "\' EXCEEDS MAXIMUM \'" << nMaxBins << "\' - REVERT TO " << nMaxBins << std::endl;
	nBinsActual = nMaxBins;
      }
    
      
      if(doLogX) getLogBins(minVal, maxVal, nBinsActual, bins);
      else getLinBins(minVal, maxVal, nBinsActual, bins);
        
      std::string cutString1 = "";
      std::string cutString2 = "NoCut";
      bool isIn = false;
    
      for(Int_t fI = 0; fI < nFiles; ++fI){
	tree_p[fI]->ResetBranchAddresses();
	tree_p[fI]->SetBranchStatus("*", 0);
	tree_p[fI]->SetBranchStatus(branchList.at(0).at(bI1).c_str(), 1);
     
      
	TTree* weightTree_p = nullptr;
	if(doWeight){
	  if(isStrSame(weightTree, fileTrees.at(0).at(tI))){
	    //Tree is same as weight tree, just turn on weight var
	    tree_p[fI]->SetBranchStatus(weightVar.c_str(), 1);
	  }
	  else{//Weight Tree differs - add friend
	    weightTree_p = (TTree*)inFiles_p[fI]->Get(weightTree.c_str());
	    weightTree_p->SetBranchStatus("*", 0);
	    weightTree_p->SetBranchStatus(weightVar.c_str(), 1);
	    
	    tree_p[fI]->AddFriend(weightTree_p);
	  }
	}
	
	if(mapFromVarToCut.count(branchList.at(0).at(bI1)) != 0){
	  //	  std::cout << "Gets into cuts: " << branchList.at(0).at(bI1) << std::endl;
	  int pos = getPosOfCutType(mapFromVarToCut[branchList.at(0).at(bI1)][0]);
	  if(pos != -1){
	    std::string varName = mapFromVarToCutVar[branchList.at(0).at(bI1)][0].substr(0, pos);
	    for(unsigned int bI2 = 0; bI2 < branchList.at(0).size(); ++bI2){
	      if(isStrSame(varName, branchList.at(0).at(bI2))){
		isIn = true;
		break;
	      }
	    }
	 	 
	    //SETTING ALWAYS TRUE FIX ME LATER
	    isIn = true;
	    if(isIn){
	      tree_p[fI]->SetBranchStatus(varName.c_str(), 1);
	      
	      for(unsigned int cI = 0; cI < mapFromVarToCut[branchList.at(0).at(bI1)].size(); ++cI){
		std::string cutVar = mapFromVarToCutVar[branchList.at(0).at(bI1)][cI];
		tree_p[fI]->SetBranchStatus(cutVar.c_str(), 1);
	      }
	      
	      if(fI == 0){
		for(unsigned int cI = 0; cI < mapFromVarToCut[branchList.at(0).at(bI1)].size(); ++cI){
		  bool isCutAbs = mapFromVarToIsCutAbs[branchList.at(0).at(bI1)][cI];
		  
		  if(!isCutAbs){
		    cutString1 = cutString1 + mapFromVarToCut[branchList.at(0).at(bI1)][cI] + " && ";
		    cutString2 = cutString2 + mapFromVarToCut[branchList.at(0).at(bI1)][cI] + " && ";
		  }
		  else{
		    std::string cutVar = mapFromVarToCutVar[branchList.at(0).at(bI1)][cI];
		    std::string cutStr = mapFromVarToCut[branchList.at(0).at(bI1)][cI];

		    cutStr.replace(cutStr.find(cutVar), cutVar.size(), ("TMath::Abs(" + cutVar + ")").c_str());
		    cutString1 = cutString1 + cutStr + " && ";

		    cutString2 = cutString2 + "Abs" + mapFromVarToCut[branchList.at(0).at(bI1)][cI] + " && ";
		  }
		}
		if(cutString1.size() != 0) cutString1.replace(cutString1.size()-4, 4, "");
		if(cutString2.size() != 0) cutString2.replace(cutString2.size()-4, 4, "");

		//Nice formatting for cutstring2
		cutString2 = findAndReplaceCutTypes(cutString2);
		while(cutString2.find(" && ") != std::string::npos){
		  cutString2.replace(cutString2.find(" && "), 4, "_");
		}
		while(cutString2.find(".") != std::string::npos){cutString2.replace(cutString2.find("."), 1, "p");}		
	      
		if(doWeight){
		  std::string treeName = fileTrees.at(0).at(tI);
		  //HACK, TEMP
		  if(treeName.substr(treeName.size()-2, 2).find("/t") != std::string::npos){
		    cutString1 = cutString1 + " && pthat > 0.35*jtpt[0]";
		  }
		  //END HACK
		  
		  cutString1 = "(" + cutString1 + ")*" + weightVar;
		}
	      }
	    }
	  }
	}

	if(doWeight && cutString1.size() == 0){
	  cutString1 = "(" + branchList.at(0).at(bI1) + " >= " + prettyString(minVal-0.01,2,false) + ")*" + weightVar;
	  cutString2 = branchList.at(0).at(bI1) + "GE" + prettyString(minVal-0.01,2,true);	  
	}

	
	std::string treeName = fileTrees.at(0).at(tI);	
	if(treeName.substr(treeName.size()-2, 2).find("/t") != std::string::npos){
	  tree_p[fI]->SetBranchStatus("jtpt", 1);
	  if(doWeight) tree_p[fI]->SetBranchStatus("pthat", 1);		   
	}

	std::cout << "CUTSTRING1: " << cutString1 << std::endl;
	histNames.at(fI) = histNames.at(fI) + "_" + cutString2 + "_h";
	std::string histNameForCanvas = histNames.at(fI) + "_" + cutString2 + "_ForCanvas_h";
	tempHistForCanvas_p[fI] = new TH1D(histNameForCanvas.c_str(), (";" + branchList.at(0).at(bI1) + ";" + globalYStr).c_str(), 10, minValXForHist, maxValXForHist);
	
	tempHist_p[fI] = new TH1D(histNames.at(fI).c_str(), (";" + branchList.at(0).at(bI1) + ";" + globalYStr).c_str(), nBinsActual, bins);

	if(branchList.at(0).at(bI1).find("jteta") != std::string::npos){
	  std::cout << " BIN CHECK: " << branchList.at(0).at(bI1) << std::endl;
	  for(Int_t bCheck = 0; bCheck < nBinsActual; ++bCheck){
	    std::cout << " " << bCheck << "/" << nBinsActual << ": " << bins[bCheck] << "-" << bins[bCheck+1] << std::endl;
	  }
	}

	
	if(doWeight) setSumW2(tempHist_p[fI]);

	if(eventCountOverride < 0) tree_p[fI]->Project(histNames.at(fI).c_str(), branchList.at(0).at(bI1).c_str(), cutString1.c_str(), "");
	else tree_p[fI]->Project(histNames.at(fI).c_str(), branchList.at(0).at(bI1).c_str(), cutString1.c_str(), "", eventCountOverride);

	std::vector<TH1D*> histsToFormat = {tempHistForCanvas_p[fI], tempHist_p[fI]};

	for(unsigned int hI = 0; hI < histsToFormat.size(); ++hI){
	  histsToFormat[hI]->GetXaxis()->SetTitle(branchList.at(0).at(bI1).c_str());
	  histsToFormat[hI]->GetYaxis()->SetTitle(globalYStr.c_str());
	  
	  histsToFormat[hI]->SetMarkerSize(1.0);
	  histsToFormat[hI]->SetMarkerStyle(styles[fI]);
	  histsToFormat[hI]->SetMarkerColor(colors[fI]);
	  histsToFormat[hI]->SetLineColor(colors[fI]);
	  
	  histsToFormat[hI]->GetXaxis()->SetTitleFont(textFont);
	  histsToFormat[hI]->GetYaxis()->SetTitleFont(textFont);
	  histsToFormat[hI]->GetXaxis()->SetLabelFont(textFont);
	  histsToFormat[hI]->GetYaxis()->SetLabelFont(textFont);
	  
	  histsToFormat[hI]->GetXaxis()->SetTitleSize(textSize);
	  histsToFormat[hI]->GetYaxis()->SetTitleSize(textSize);
	  histsToFormat[hI]->GetXaxis()->SetLabelSize(textSize);
	  histsToFormat[hI]->GetYaxis()->SetLabelSize(textSize);
	  
	  histsToFormat[hI]->GetYaxis()->SetTitleOffset(titleOffset);
	}


	if(doEventNorm){
	  if(!doWeight){
	    setSumW2(tempHist_p[fI]);

	    Double_t scaleVal = (Double_t)tree_p[fI]->GetEntries();
	    if(eventCountOverride > 0 && eventCountOverride < scaleVal) scaleVal = (Double_t)eventCountOverride;
	    
	    tempHist_p[fI]->Scale(1.0/scaleVal);
	  }
	  else{
	    tempHist_p[fI]->Scale(1.0/inFilesWeightedCounts[fI]);
	  }	  
	}
      }


      //FIXME YOU ARE MIXING MAXVAL AND MINVAL
      maxVal = tempHist_p[0]->GetMinimum();
      minVal = tempHist_p[0]->GetMaximum();
      double minValLogY = tempHist_p[0]->GetMaximum();
      
      for(Int_t fI = 0; fI < nFiles; ++fI){
	for(Int_t bIX = 0; bIX < tempHist_p[fI]->GetNbinsX(); ++bIX){
	  if(tempHist_p[fI]->GetBinContent(bIX+1) != 0){
	    if(tempHist_p[fI]->GetBinContent(bIX+1) > maxVal) maxVal = tempHist_p[fI]->GetBinContent(bIX+1);
	    if(tempHist_p[fI]->GetBinContent(bIX+1) < minVal){
	      minVal = tempHist_p[fI]->GetBinContent(bIX+1);

	      if(minVal > TMath::Power(10,-100)) minValLogY = minVal;
	    }
	  }
	}
      }

      Double_t interval = maxVal - minVal;

      bool doLogY = getDoLog(minVal, maxVal, 2);
      if(doLogYOverrideMap.count(branchList.at(0).at(bI1)) > 0) doLogY = doLogYOverrideMap[branchList.at(0).at(bI1)];      

      if(doLogY){ 
	maxVal *= 10;
	minValLogY /= 10;
      }
      else{
	maxVal += interval/10.;
	if(minVal - interval/10 > 0 || minVal < 0) minVal -= interval/10;
	else minVal = 0;
      }
     
      
      tempHistForCanvas_p[0]->SetMaximum(maxVal);
      if(doLogY) tempHistForCanvas_p[0]->SetMinimum(minValLogY);
      else tempHistForCanvas_p[0]->SetMinimum(minVal);

      canv_p->cd();
      pads_p[0]->cd();
    
      for(Int_t fI = 0; fI < nFiles; ++fI){
	if(fI == 0){
	  if(cutString1.size() != 0) tempHistForCanvas_p[fI]->SetTitle(cutString1.c_str());
	  else tempHistForCanvas_p[fI]->SetTitle("No cut");

	  tempHistForCanvas_p[fI]->DrawCopy();
	  
	  tempHist_p[fI]->DrawCopy("E1 P SAME");
	}
	else tempHist_p[fI]->DrawCopy("E1 P SAME");

	if(branchList.at(0).at(bI1).find("jteta") != std::string::npos){
	  std::cout << "PRINT" << std::endl;
	  tempHist_p[fI]->Print("ALL");
	}
      }
          
      leg_p->Draw("SAME");
    
      
      if(doLogY) gPad->SetLogy();
      if(doLogX) gPad->SetLogx();
      gStyle->SetOptStat(0);
      gPad->RedrawAxis();
      //      gPad->SetTicks(0, 1);
      gPad->SetTicks();

      canv_p->cd();
      pads_p[1]->cd();

      for(Int_t fI = 1; fI < nFiles; ++fI){
	if(!doWeight && !doEventNorm) setSumW2(tempHist_p[fI]);

	tempHist_p[fI]->Divide(tempHist_p[0]);
	tempHist_p[fI]->GetXaxis()->SetTitleSize(textSize*(1.0 - padSplit)/padSplit);
	tempHist_p[fI]->GetYaxis()->SetTitleSize(textSize*(1.0 - padSplit)/padSplit);
	tempHist_p[fI]->GetXaxis()->SetLabelSize(textSize*(1.0 - padSplit)/padSplit);
	tempHist_p[fI]->GetYaxis()->SetLabelSize(textSize*(1.0 - padSplit)/padSplit);

	tempHist_p[fI]->GetYaxis()->SetNdivisions(505);
	tempHist_p[fI]->GetYaxis()->SetTitleOffset(titleOffset*padSplit/(1.0 - padSplit));
 
	tempHistForCanvas_p[fI]->GetXaxis()->SetTitleSize(textSize*(1.0 - padSplit)/padSplit);
	tempHistForCanvas_p[fI]->GetYaxis()->SetTitleSize(textSize*(1.0 - padSplit)/padSplit);
	tempHistForCanvas_p[fI]->GetXaxis()->SetLabelSize(textSize*(1.0 - padSplit)/padSplit);
	tempHistForCanvas_p[fI]->GetYaxis()->SetLabelSize(textSize*(1.0 - padSplit)/padSplit);

	tempHistForCanvas_p[fI]->GetYaxis()->SetNdivisions(505);
	tempHistForCanvas_p[fI]->GetYaxis()->SetTitleOffset(titleOffset*padSplit/(1.0 - padSplit));
      }



      bool isGood = true;

      for(Int_t bIX = 0; bIX < tempHist_p[0]->GetNbinsX(); ++bIX){

	for(Int_t fI = 1; fI < nFiles; ++fI){

	  if(TMath::Abs(tempHist_p[fI]->GetBinContent(bIX+1)) < minDeltaVal){
	    if(TMath::Abs(tempHist_p[0]->GetBinContent(bIX+1)) < minDeltaVal) continue;
	    else{
	      isGood = false;
	      break;
	    }
	  }
	  
	  if(!isGood) break;

	  if(TMath::Abs(tempHist_p[fI]->GetBinContent(bIX+1) - 1.) > minDeltaVal){
	    isGood = false;
	    break;
	  }
	}
      }

      if(!isGood){
	warningList.push_back(branchList.at(0).at(bI1));
	warningListPos.push_back(bI1);
      }
    
      for(Int_t fI = 1; fI < nFiles; ++fI){
	tempHistForCanvas_p[fI]->GetYaxis()->SetTitle(("All/" + inNickNames.at(0)).c_str());

	double localDivMin = divMin;
	double localDivMax = divMax;

	if(divMinOverrideMap.count(branchList.at(0).at(bI1)) > 0){
	  localDivMin = divMinOverrideMap[branchList.at(0).at(bI1)];
	}

	if(divMaxOverrideMap.count(branchList.at(0).at(bI1)) > 0){
	  localDivMax = divMaxOverrideMap[branchList.at(0).at(bI1)];
	}

	double localDivInt = localDivMax - localDivMin;
	
	tempHistForCanvas_p[fI]->SetMaximum(localDivMax);
	tempHistForCanvas_p[fI]->SetMinimum(localDivMin);

	if(fI == 1) tempHistForCanvas_p[fI]->DrawCopy();	

	tempHist_p[fI]->DrawCopy("E1 P SAME");

	for(Int_t bIX = 0; bIX < tempHist_p[fI]->GetXaxis()->GetNbins(); ++bIX){
	  Double_t val = tempHist_p[fI]->GetBinContent(bIX+1);
	  Double_t denom = tempHist_p[0]->GetBinContent(bIX+1);

	  if(denom > TMath::Power(10, -20)){
	    if(val < localDivMin){
	      tempHist_p[fI]->SetBinContent(bIX+1, localDivMin + 0.00001);
	      Double_t err = localDivInt*0.05*fI;
	      tempHist_p[fI]->SetBinError(bIX+1, err);
	    }
	    else if(val > localDivMax){
	      tempHist_p[fI]->SetBinContent(bIX+1, localDivMax - 0.00001);
	      Double_t err = localDivInt*0.05*fI;
	      tempHist_p[fI]->SetBinError(bIX+1, err);
	    }
	  }
	}

	tempHist_p[fI]->SetMarkerSize(0.000001);
	tempHist_p[fI]->DrawCopy("E1 P SAME");
      }

      if(doLogX) gPad->SetLogx();

      line.DrawLine(tempHistForCanvas_p[0]->GetBinLowEdge(1), 1, tempHistForCanvas_p[0]->GetBinLowEdge(tempHistForCanvas_p[0]->GetNbinsX()+1), 1);
      gPad->RedrawAxis();
      //      gPad->SetTicks(0, 1);
      gPad->SetTicks();

      std::string saveName = histNames.at(0) + "_" + additionalNickName;
      for(Int_t fI = 1; fI < nFiles; ++fI){
	saveName = saveName + "_" + inNickNames.at(fI);
      }
      saveName = saveName + "_" + dateStr + ".png";
      pdfList.push_back(saveName);
      quietSaveAs(canv_p, fullDirName + saveName);

      for(Int_t fI = 0; fI < nFiles; ++fI){
	delete tempHist_p[fI];
	delete tempHistForCanvas_p[fI];
      }

      delete pads_p[0];
      delete pads_p[1];
      delete canv_p;
    }

    for(unsigned int pI = 0; pI < pairVars1Found.size(); ++pI){
      if(!pairVars1Found.at(pI)) continue;
      if(!pairVars2Found.at(pI)) continue;
    
      tree_p[0]->ResetBranchAddresses();
      tree_p[0]->SetBranchStatus("*", 0);
      tree_p[0]->SetBranchStatus(pairVars1.at(pI).c_str(), 1);
      tree_p[0]->SetBranchStatus(pairVars2.at(pI).c_str(), 1);

      std::string cutString1 = "";
      std::string cutString2 = "NoCut";
      if(pairCutVarsFound.at(pI)){
	tree_p[0]->SetBranchStatus(pairCutVars.at(pI).c_str(), 1);
	cutString1 = pairCutVars.at(pI) + pairCutTypes.at(pI) + pairCutVals.at(pI);
	cutString2 = findAndReplaceCutTypes(cutString1);
	while(cutString2.find(".") != std::string::npos){cutString2.replace(cutString2.find("."), 1, "p");}
      }


      std::string histNameDenom = fileTrees.at(0).at(tI) + "_" + pairVars1.at(pI) + "_" + pairVars2.at(pI) + "_" + cutString2 + "_" + inNickNames.at(0) + "_h";
      while(histNameDenom.find("/") != std::string::npos){histNameDenom.replace(histNameDenom.find("/"), 1, "_");}
      TH2D* histDenom_p = new TH2D(histNameDenom.c_str(), (";" + pairVars2.at(pI) + ";"+ pairVars1.at(pI)).c_str(), nMaxBins, pairVarsMin2.at(pI), pairVarsMax2.at(pI), nMaxBins, pairVarsMin1.at(pI), pairVarsMax1.at(pI));

	  
      if(eventCountOverride < 0) tree_p[0]->Project(histNameDenom.c_str(), (pairVars1.at(pI) + ":" + pairVars2.at(pI)).c_str(), cutString1.c_str(), "");
      else tree_p[0]->Project(histNameDenom.c_str(), (pairVars1.at(pI) + ":" + pairVars2.at(pI)).c_str(), cutString1.c_str(), "", eventCountOverride);
	  
      histDenom_p->SetTitleFont(textFont);
      histDenom_p->SetTitleSize(textSize);

      histDenom_p->GetXaxis()->SetTitleFont(textFont);
      histDenom_p->GetYaxis()->SetTitleFont(textFont);
      histDenom_p->GetXaxis()->SetLabelFont(textFont);
      histDenom_p->GetYaxis()->SetLabelFont(textFont);
      
      histDenom_p->GetXaxis()->SetTitleSize(textSize);
      histDenom_p->GetYaxis()->SetTitleSize(textSize);
      histDenom_p->GetXaxis()->SetLabelSize(textSize);
      histDenom_p->GetYaxis()->SetLabelSize(textSize);

      if(cutString1.size() != 0) histDenom_p->SetTitle((inNickNames.at(0) + ", " + cutString1).c_str());
      else histDenom_p->SetTitle((inNickNames.at(0) + ", No cut").c_str());
      
      
      centerTitles(histDenom_p);
      setSumW2(histDenom_p);
    
      if(doEventNorm){
	if(eventCountOverride > 0) histDenom_p->Scale(1./(Double_t)eventCountOverride);
	else histDenom_p->Scale(1./(Double_t)tree_p[0]->GetEntries());
      }
      
      Double_t getNonZeroTH2MinDenom = getNonZeroTH2Min(histDenom_p);
      Double_t getMaxDenom = histDenom_p->GetMaximum();
      
      if(nFiles <= 1){
	std::cout << "Single file not yet implemented" << std::endl;
      }
      else{
	for(Int_t hI = 1; hI < nFiles; ++hI){

	  std::string histName = fileTrees.at(0).at(tI) + "_" + pairVars1.at(pI) + "_" + pairVars2.at(pI) + "_" + cutString2 + "_" + inNickNames.at(hI) + "_h";
	  while(histName.find("/") != std::string::npos){histName.replace(histName.find("/"), 1, "_");}
	  TH2D* hist_p = new TH2D(histName.c_str(), (";" + pairVars2.at(pI) + ";"+ pairVars1.at(pI)).c_str(), nMaxBins, pairVarsMin2.at(pI), pairVarsMax2.at(pI), nMaxBins, pairVarsMin1.at(pI), pairVarsMax1.at(pI));
	  	
	  tree_p[hI]->ResetBranchAddresses();
	  tree_p[hI]->SetBranchStatus("*", 0);
	  tree_p[hI]->SetBranchStatus(pairVars1.at(pI).c_str(), 1);
	  tree_p[hI]->SetBranchStatus(pairVars2.at(pI).c_str(), 1);
	  
	  if(pairCutVarsFound.at(pI)){
	    tree_p[hI]->SetBranchStatus(pairCutVars.at(pI).c_str(), 1);
	  }

	  if(eventCountOverride < 0) tree_p[hI]->Project(histName.c_str(), (pairVars1.at(pI) + ":" + pairVars2.at(pI)).c_str(), cutString1.c_str(), "");
	  else tree_p[hI]->Project(histName.c_str(), (pairVars1.at(pI) + ":" + pairVars2.at(pI)).c_str(), cutString1.c_str(), "", eventCountOverride);

	  centerTitles(hist_p);
	  setSumW2(hist_p);
	  
	  if(doEventNorm){
	    if(eventCountOverride > 0) hist_p->Scale(1./(Double_t)eventCountOverride);
	    else hist_p->Scale(1./(Double_t)tree_p[hI]->GetEntries());
	  }

	  TCanvas* canv_p = new TCanvas("tempCanv_p", "tempCanv_p", 900*3, 900);	 
	  canv_p->SetTopMargin(0.01);
	  canv_p->SetBottomMargin(0.01);
	  canv_p->SetLeftMargin(0.01);
	  canv_p->SetRightMargin(0.01);

	  const Int_t nPads = 3;
	  TPad* pads[nPads];
	  for(Int_t i = 0; i < nPads; ++i){
	    double xLow = 0.0 + i/3.;
	    double xHi = 0.0 + (i+1)/3.;

	    pads[i] = new TPad(("pads" + std::to_string(i)).c_str(), "", xLow, 0.0, xHi, 1.0);
	    pads[i]->SetTopMargin(0.12);
	    pads[i]->SetBottomMargin(0.12);
	    pads[i]->SetLeftMargin(0.14);
	    pads[i]->SetRightMargin(0.12);
	    canv_p->cd();
	    pads[i]->Draw("SAME");
	    pads[i]->cd();
	  }
	
	  
	  Double_t getNonZeroTH2MinHist = getNonZeroTH2Min(hist_p);
	  Double_t getMax = hist_p->GetMaximum();

	  getMax = TMath::Max(getMax, getMaxDenom);
	  getNonZeroTH2MinHist = TMath::Min(getNonZeroTH2MinHist, getNonZeroTH2MinDenom);

	  getMax *= 1.1;
	  getNonZeroTH2MinHist /= 1.1;

	  histDenom_p->SetMaximum(getMax);
	  histDenom_p->SetMinimum(getNonZeroTH2MinHist);

	  hist_p->SetMaximum(getMax);
	  hist_p->SetMinimum(getNonZeroTH2MinHist);

	  canv_p->cd();
	  pads[0]->cd();
	  histDenom_p->DrawCopy("COLZ");	 
	  //	  gPad->SetLogz();
	  gPad->RedrawAxis();
	  //	  gPad->SetTicks(0, 1);
	  gPad->SetTicks();
	  
	  
	  hist_p->SetTitleFont(textFont);
	  hist_p->SetTitleSize(textSize);
	  
	  hist_p->GetXaxis()->SetTitleFont(textFont);
	  hist_p->GetYaxis()->SetTitleFont(textFont);
	  hist_p->GetXaxis()->SetLabelFont(textFont);
	  hist_p->GetYaxis()->SetLabelFont(textFont);
	  
	  hist_p->GetXaxis()->SetTitleSize(textSize);
	  hist_p->GetYaxis()->SetTitleSize(textSize);
	  hist_p->GetXaxis()->SetLabelSize(textSize);
	  hist_p->GetYaxis()->SetLabelSize(textSize);
	  

	  //	  Double_t getNonZeroTH2MinNum = getNonZeroTH2Min(hist_p);
	  //	  hist_p->SetMinimum(getNonZeroTH2MinNum/3.);

	  canv_p->cd();
	  pads[1]->cd();

	  if(cutString1.size() != 0) hist_p->SetTitle((inNickNames.at(hI) + ", " + cutString1).c_str());
	  else hist_p->SetTitle((inNickNames.at(hI) + ", No cut").c_str());

	  hist_p->DrawCopy("COLZ");
	  //	  gPad->SetLogz();
	  gPad->RedrawAxis();
	  //	  gPad->SetTicks(0, 1);
	  gPad->SetTicks();

	  canv_p->cd();
	  pads[2]->cd();
	  hist_p->Divide(histDenom_p);
	  hist_p->SetMinimum(0.0);
	  hist_p->SetMaximum(2.0);
	  gPad->SetLogz(0);
	  if(cutString1.size() != 0) hist_p->SetTitle((inNickNames.at(hI) + "/" + inNickNames.at(0) + ", " + cutString1).c_str());
	  else hist_p->SetTitle((inNickNames.at(hI) + "/" + inNickNames.at(0) + ", No cut").c_str());
	  hist_p->DrawCopy("COLZ");
	  gPad->RedrawAxis();
	  //	  gPad->SetTicks(0, 1);
	  gPad->SetTicks();
	  
	  std::string saveName = histName + "_" + additionalNickName;
	  saveName = saveName + "_" + dateStr + ".png";
	  pdfList.push_back(saveName);
	  quietSaveAs(canv_p, fullDirName + saveName);
	  
	  branchList.at(0).push_back(pairVars1.at(pI) + ":" + pairVars2.at(pI) + "," + inNickNames.at(hI) + "/" + inNickNames.at(0));
	  
	  delete hist_p;
	  for(Int_t i = 0; i < nPads; ++i){
	    delete pads[i];
	  }
	  delete canv_p;
	}

	delete histDenom_p;
      }
    }
  
  
    doTreeTexSlide(&fileTex, fileTrees.at(0).at(tI), inNickNames, misMatchedBranches, warningList, warningListPos);
    //    doTreeTexSlide(&fileSubTex, file1Trees.at(tI), misMatchedBranches1, misMatchedBranches2, warningList, warningListPos);
    doBranchTexSlide(&fileSubTex, fileTrees.at(0).at(tI), branchList.at(0));
    for(unsigned int bI1 = 0; bI1 < branchList.at(0).size(); ++bI1){
      doPlotTexSlide(&fileSubTex, fileTrees.at(0).at(tI), branchList.at(0).at(bI1), bI1, pdfList.at(bI1));
    }
  }

  delete leg_p;

  for(Int_t fI = 0; fI < nFiles; ++fI){
    delete dummyHists_p[fI];
    inFiles_p[fI]->Close();
    delete inFiles_p[fI];
  }

  if(texFileSubName.find(fullDirName) != std::string::npos){
    texFileSubName.replace(texFileSubName.find(fullDirName), fullDirName.size(), "");
  }

  fileTex << "\\input{" << texFileSubName << "}" << std::endl;
  fileTex << "\\end{document}" << std::endl;

  return 0;
}

int main(int argc, char* argv[])
{
  cppWatch watch;
  watch.start();

  //OLD VERSION
  /*
  if(argc < 3 || argc > 9){
    std::cout << "Usage: ./bin/runForestDQM.exe <inFileNames> <additionalNickName> <inNickNames> <treeSelect> <doEventNorm> <eventCountOverride> <commaSeparatedCutList> <commaSeparatedPairList>" << std::endl;
    std::cout << " inFileNames, inNickNames a comma separated list of arbitrarily many files" << std::endl;
    return 1;
  }

  std::vector<std::string> inFiles;
  std::string argv1 = argv[1];
  while(argv1.find(",") != std::string::npos){
    inFiles.push_back(argv1.substr(0, argv1.find(",")));
    argv1.replace(0, argv1.find(",")+1, "");
  }
  if(argv1.size() != 0) inFiles.push_back(argv1);

  std::vector<std::string> inNickNames;

  if(argc >= 4){
    std::string argv3 = argv[3];
    while(argv3.find(",") != std::string::npos){
      if(argv3.substr(0, argv3.find(",")).size() == 0) inNickNames.push_back("");
      else inNickNames.push_back(argv3.substr(0, argv3.find(",")));
      argv3.replace(0, argv3.find(",")+1, "");
    }
    if(argv3.size() != 0) inNickNames.push_back(argv3);
  }
  else{
    for(unsigned int fI = 0; fI < inFiles.size(); ++fI){
      inNickNames.push_back("");
    }
  }

  if(inFiles.size() != inNickNames.size() && argc >= 3){
    std::cout << "Warning: number of files, " << inFiles.size() << ", given by \'" << argv[1] << "\', is not equal to number of nicknames, " << inNickNames.size() << ", given by \'" << argv[2] << "\'. return 1" << std::endl;
    return 1;
  }

  int retVal = 0;
  if(argc == 3 || argc == 4) retVal += runForestDQM(inFiles, argv[2], inNickNames);
  else if(argc == 5) retVal += runForestDQM(inFiles, argv[2], inNickNames, argv[4]);
  else if(argc == 6) retVal += runForestDQM(inFiles, argv[2], inNickNames, argv[4], std::stoi(argv[5]));
  else if(argc == 7) retVal += runForestDQM(inFiles, argv[2], inNickNames, argv[4], std::stoi(argv[5]), std::stoi(argv[6]));
  else if(argc == 8) retVal += runForestDQM(inFiles, argv[2], inNickNames, argv[4], std::stoi(argv[5]), std::stoi(argv[6]), argv[7]);
  else if(argc == 9) retVal += runForestDQM(inFiles, argv[2], inNickNames, argv[4], std::stoi(argv[5]), std::stoi(argv[6]), argv[7], argv[8]);
  */

  //2023.09.21 NEW VERSION WITH CONFIG FILE

  if(argc != 2){
    std::cout << "Usage: ./bin/runForestDQM.exe <inConfigFile>. return 1" << std::endl;    
    return 1;
  }

  int retVal = 0;
  retVal += runForestDQM(argv[1]);
  
  watch.stop();
  std::cout << "Timing: " << watch.total() << std::endl;
  
  return retVal;
}
