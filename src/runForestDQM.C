///AUTHOR: CFMCGINN (2018.04.12)
///For validating forest
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TObjArray.h"
#include "TMath.h"
#include "TDatime.h"
#include "TPad.h"
#include "TStyle.h"
#include "TLine.h"
#include "TLegend.h"
#include "TLeaf.h"

#include "include/checkMakeDir.h"
#include "include/cppWatch.h"
#include "include/returnRootFileContentsList.h"
#include "include/removeVectorDuplicates.h"
#include "include/kirchnerPalette.h"
#include "include/histDefUtility.h"

void dumpTreeNames(std::string fileName, std::vector<std::string> treeNames)
{
  std::cout << "TTrees In File \'" << fileName << "\':" << std::endl;
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

void doFirstTexSlide(std::ofstream* fileTex, std::string inFileName1, std::string inFileName2, std::string inNickName1, std::string inNickName2, std::vector<std::string> goodTrees, std::vector<std::string> missingTrees1, std::vector<std::string> missingTrees2, const Int_t eventCountOverride)
{
  TDatime date;

  std::string texInFileName1 = texFriendlyString(inFileName1);
  std::string texInFileName2 = texFriendlyString(inFileName2);

  (*fileTex) << "\\RequirePackage{xspace}" << std::endl;
  (*fileTex) << "\\RequirePackage{amsmath}" << std::endl;

  (*fileTex) << std::endl;

  (*fileTex) << "\\documentclass[xcolor=dvipsnames]{beamer}" << std::endl;
  (*fileTex) << "\\usetheme{Warsaw}" << std::endl;
  (*fileTex) << "\\setbeamercolor{structure}{fg=NavyBlue!90!NavyBlue}" << std::endl;
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

  (*fileTex) << "\\author[CM]{CMSHI Forest Validatory}" << std::endl;
  (*fileTex) << "\\begin{document}" << std::endl;

  (*fileTex) << std::endl;

  std::string tempFileName1 = texInFileName1.substr(0, texInFileName1.size()/2) + "\n" + texInFileName1.substr(texInFileName1.size()/2, texInFileName1.size());
  std::string tempFileName2 = texInFileName2.substr(0, texInFileName2.size()/2) + "\n" + texInFileName2.substr(texInFileName2.size()/2, texInFileName2.size());

  (*fileTex) << "\\begin{frame}" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{\\hypertarget{TopLevel}{Forest Validation} (" << date.GetYear() << "." << date.GetMonth() << "." << date.GetDay() << ")}}" << std::endl;
  (*fileTex) << " \\begin{itemize}" << std::endl;
  (*fileTex) << "  \\fontsize{5}{5}\\selectfont" << std::endl;
  if(eventCountOverride >= 0) (*fileTex) << " \\item{WARNING: EVENT NUMBER OVERRIDE SET, TEST INVALID}" << std::endl;
  (*fileTex) << "  \\item{File 1: \'" << tempFileName1 << "\'}" << std::endl;
  (*fileTex) << "  \\item{Nickname: \'" << texFriendlyString(inNickName1) << "\'}" << std::endl;

  (*fileTex) << "  \\item{File 2: \'" << tempFileName2 << "\'}" << std::endl;
  (*fileTex) << "  \\item{Nickname: \'" << texFriendlyString(inNickName2) << "\'}" << std::endl;
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

  (*fileTex) << "  \\item{Bad File 1, " << texFriendlyString(inNickName1) << ", Trees:}" << std::endl;

  (*fileTex) << " \\begin{itemize}" << std::endl;
  (*fileTex) << "  \\fontsize{5}{5}\\selectfont" << std::endl;
  if(missingTrees1.size() != 0){
    for(unsigned int tI = 0; tI < missingTrees1.size(); ++tI){
      (*fileTex) << "  \\item{Tree " << tI << ": \'" << texFriendlyString(missingTrees1.at(tI)) << "\'}" << std::endl;
    }
  }
  else (*fileTex) << "  \\item{NO MISSING TREES}" << std::endl;
  (*fileTex) << " \\end{itemize}" << std::endl;

  (*fileTex) << "  \\item{Bad File 2, " << texFriendlyString(inNickName2) << ", Trees:}" << std::endl;

  (*fileTex) << " \\begin{itemize}" << std::endl;
  (*fileTex) << "  \\fontsize{5}{5}\\selectfont" << std::endl;
  if(missingTrees2.size() != 0){
    for(unsigned int tI = 0; tI < missingTrees2.size(); ++tI){
      (*fileTex) << "  \\item{Tree " << tI << ": \'" << texFriendlyString(missingTrees2.at(tI)) << "\'}" << std::endl;
    }
  }
  else (*fileTex) << "  \\item{NO MISSING TREES}" << std::endl;

  (*fileTex) << " \\end{itemize}" << std::endl;

  (*fileTex) << " \\end{itemize}" << std::endl;
  (*fileTex) << "\\end{frame}" << std::endl;
  (*fileTex) << std::endl;

  return;
}


void doInterstitialTexSlide(std::ofstream* fileTex, const std::string interstitialString)
{
  (*fileTex) << "\\begin{frame}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{Interstitial}}" << std::endl;
  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  (*fileTex) << "\\item{" << interstitialString << "}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\end{frame}" << std::endl;

  return;
}


void doTreeTexSlide(std::ofstream* fileTex, std::string inTreeName, std::vector<std::string> listOfBadVar1, std::vector<std::string> listOfBadVar2, std::vector<std::string> warningList, std::vector<int> warningListPos)
{
  std::string treeTitle = texFriendlyString(inTreeName);
  while(treeTitle.find("/") != std::string::npos){
    treeTitle.replace(treeTitle.find("/"), 1, " ");
  }

  (*fileTex) << "\\begin{frame}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{\\hypertarget{tocSummary_" << inTreeName << "}{" << treeTitle << "}}}" << std::endl;
  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  (*fileTex) << "\\item{Tree Name \'" << texFriendlyString(inTreeName) << "\'}, (\\hyperlink{Back to Top}{TopLevel})" << std::endl;
  (*fileTex) << "\\item{Missing variables in Tree 1}" << std::endl;
  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  if(listOfBadVar1.size() != 0){
    for(unsigned int vI = 0; vI < listOfBadVar1.size(); ++vI){
      (*fileTex) << "\\item{Var " << vI << "/" << listOfBadVar1.size() << ": \'" << texFriendlyString(listOfBadVar1.at(vI)) << "\'}" << std::endl;
    }
  }
  else (*fileTex) << "\\item{No missing variables}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\item{Missing variables in Tree 2}" << std::endl;
  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  if(listOfBadVar2.size() != 0){
    for(unsigned int vI = 0; vI < listOfBadVar2.size(); ++vI){
      (*fileTex) << "\\item{Var " << vI << "/" << listOfBadVar2.size() << ": \'" << texFriendlyString(listOfBadVar2.at(vI)) << "\'}" << std::endl;
    }
  }
  else (*fileTex) << "\\item{No missing variables}" << std::endl; 
  (*fileTex) << "\\end{itemize}" << std::endl;

  (*fileTex) << "\\item{Variables deviating from 1 in ratio}" << std::endl;
  if(warningList.size() != 0){
    std::vector<std::vector<std::string> > warning1, warning2, warning3;

    int nBranches = 0;

    for(unsigned int bI = 0; bI < warningList.size(); ++bI){

      if(warning1.size() < 2){
	if(bI%36 == 0){
	  warning1.push_back({});
	  warning2.push_back({});
	  warning3.push_back({});
	}
	
	if((bI/12)%3 == 0) warning1.at(warning1.size()-1).push_back(warningList.at(bI));
	else if((bI/12)%3 == 1) warning2.at(warning2.size()-1).push_back(warningList.at(bI));
	else if((bI/12)%3 == 2) warning3.at(warning3.size()-1).push_back(warningList.at(bI));
      }
      else{
	if((bI-36)%48 == 0){
	  warning1.push_back({});
	  warning2.push_back({});
	  warning3.push_back({});
	}
	
	if(((bI-36)/16)%3 == 0) warning1.at(warning1.size()-1).push_back(warningList.at(bI));
	else if(((bI-36)/16)%3 == 1) warning2.at(warning2.size()-1).push_back(warningList.at(bI));
	else if(((bI-36)/16)%3 == 2) warning3.at(warning3.size()-1).push_back(warningList.at(bI));
      }
    }

    for(unsigned int tempI = 0; tempI < warning1.size(); ++tempI){
      if(tempI != 0){
	(*fileTex) << "\\begin{frame}" << std::endl;
	(*fileTex) << "\\fontsize{4}{4}\\selectfont" << std::endl;
	(*fileTex) << "\\frametitle{\\centerline{" << treeTitle << " (Continued)}}" << std::endl;
	(*fileTex) << "\\begin{itemize}" << std::endl;
	(*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
	(*fileTex) << "\\item{Tree Name \'" << texFriendlyString(inTreeName) << "\'}" << std::endl;
	(*fileTex) << "\\item{Variables deviating from 1 in ratio}" << std::endl;
      }

      (*fileTex) << "\\begin{columns}" << std::endl;
      (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
      
      (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
      (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
      (*fileTex) << "\\begin{itemize}" << std::endl;
      (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
      if(warning1.at(tempI).size() != 0){
	for(unsigned int vI = 0; vI < warning1.at(tempI).size(); ++vI){
	  (*fileTex) << "\\item{Var " << nBranches << "/" << warningList.size() << ": \'\\hyperlink{branch_" << inTreeName << "_" << warningListPos.at(nBranches) << "}{" << texFriendlyString(warning1.at(tempI).at(vI)) << "}\'}" << std::endl;
	  nBranches++;
	}
      }    
      else (*fileTex) << "\\item{Dummy}" << std::endl;
      (*fileTex) << "\\end{itemize}" << std::endl;
      (*fileTex) << "\\end{column}" << std::endl;
      
      (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
      (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
      (*fileTex) << "\\begin{itemize}" << std::endl;
      (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
      if(warning2.at(tempI).size() != 0){
	
	for(unsigned int vI = 0; vI < warning2.at(tempI).size(); ++vI){
	  (*fileTex) << "\\item{Var " << nBranches << "/" << warningList.size() << ": \'\\hyperlink{branch_" << inTreeName << "_" << warningListPos.at(nBranches) << "}{" << texFriendlyString(warning2.at(tempI).at(vI)) << "}\'}" << std::endl;
	  //	(*fileTex) << "\\item{Var " << nBranches << "/" << warningList.size() << ": \'" << texFriendlyString(warning2.at(tempI).at(vI)) << "\'}" << std::endl;
	  nBranches++;
	}
      }
      else (*fileTex) << "\\item{Dummy}" << std::endl;
      (*fileTex) << "\\end{itemize}" << std::endl;
      (*fileTex) << "\\end{column}" << std::endl;
      
      (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
      (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
      (*fileTex) << "\\begin{itemize}" << std::endl;
      (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
      if(warning3.at(tempI).size() != 0){
	
	for(unsigned int vI = 0; vI < warning3.at(tempI).size(); ++vI){
	  (*fileTex) << "\\item{Var " << nBranches << "/" << warningList.size() << ": \'\\hyperlink{branch_" << inTreeName << "_" << warningListPos.at(nBranches) << "}{" << texFriendlyString(warning3.at(tempI).at(vI)) << "}\'}" << std::endl;
	  //	(*fileTex) << "\\item{Var " << nBranches << "/" << warningList.size() << ": \'" << texFriendlyString(branchVect3.at(tempI).at(vI)) << "\'}" << std::endl;
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
  }
  else{
    (*fileTex) << "\\item{No deviating variables}" << std::endl;
    (*fileTex) << "\\end{itemize}" << std::endl;
    (*fileTex) << "\\end{frame}" << std::endl;
  }

  /*
    for(unsigned int vI = 0; vI < warningList.size(); ++vI){
      (*fileTex) << "\\item{Var " << vI << "/" << warningList.size() << ": \'\\hyperlink{branch_" << inTreeName << "_" << warningListPos.at(vI) << "}{" << texFriendlyString(warningList.at(vI)) << "}\'}" << std::endl;
    }
  }
 
  else (*fileTex) << "\\item{No deviating variables}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;
  */

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
    (*fileTex) << "\\item{Tree Name \'" << texFriendlyString(inTreeName) << "\', (\\hyperlink{Back to Top}{TopLevel})}" << std::endl;
    (*fileTex) << "\\item{Branches}" << std::endl;

    (*fileTex) << "\\begin{columns}" << std::endl;
    (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;

    (*fileTex) << "\\begin{column}{0.32\\textwidth}" << std::endl;
    (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
    (*fileTex) << "\\begin{itemize}" << std::endl;
    (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
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
    (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
    (*fileTex) << "\\begin{itemize}" << std::endl;
    (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
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
    (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
    (*fileTex) << "\\begin{itemize}" << std::endl;
    (*fileTex) << "\\fontsize{1}{1}\\selectfont" << std::endl;
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
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{\\hypertarget{branch_" << inTreeName << "_" << inBranchNum << "}{" << texFriendlyString(inBranchName) << "} (" << treeTitle << ")}}" << std::endl;

  (*fileTex) << "\\begin{center}" << std::endl;
  (*fileTex) << "\\includegraphics[width=0.6\\textwidth]{" << inPdfName << "}" << std::endl;
  (*fileTex) << "\\end{center}" << std::endl;

  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  (*fileTex) << "\\item{" << texFriendlyString(inBranchName) << ", \\hyperlink{tocSummary_" << inTreeName << "}{To Summary TOC}, \\hyperlink{tocFull_0_" << inTreeName << "}{To All Branch TOC}}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\end{frame}" << std::endl;

  (*fileTex) << std::endl;

  return;
}


int runForestDQM(const std::string inFileName1, const std::string inFileName2, std::string inNickName1 = "", std::string inNickName2 = "", const std::string treeSelect = "", const Int_t eventCountOverride = -1)
{
  if(inFileName1.find(".root") == std::string::npos || !checkFile(inFileName1)){
    std::cout << "Input \'" << inFileName1 << "\' is invalid return 1." << std::endl;
    return 1;
  }
  else if(inFileName2.find(".root") == std::string::npos || !checkFile(inFileName2)){
    std::cout << "Input \'" << inFileName2 << "\' is invalid return 1." << std::endl;
    return 1;
  }

  if(inNickName1.size() == 0) inNickName1 = "File1";
  if(inNickName2.size() == 0) inNickName2 = "File2";

  Double_t minDeltaVal = 0.000000001;
  kirchnerPalette col;
  TDatime date;

  checkMakeDir("pdfDir");
  checkMakeDir(("pdfDir/" + std::to_string(date.GetDate())).c_str());

  TLine line;
  line.SetLineStyle(2);

  TH1F* dummyHist1_p = new TH1F();
  dummyHist1_p->SetMarkerStyle(20);
  dummyHist1_p->SetMarkerSize(0.8);
  dummyHist1_p->SetMarkerColor(col.getColor(2));
  dummyHist1_p->SetLineColor(col.getColor(2));

  TH1F* dummyHist2_p = new TH1F();
  dummyHist2_p->SetMarkerStyle(25);
  dummyHist2_p->SetMarkerSize(0.8);
  dummyHist2_p->SetMarkerColor(col.getColor(0));
  dummyHist2_p->SetLineColor(col.getColor(0));

  TLegend* leg_p = new TLegend(.6, .6, .9, .9);
  leg_p->SetBorderSize(0);
  leg_p->SetFillStyle(0);
  leg_p->SetTextFont(43);
  leg_p->SetTextSize(12);

  leg_p->AddEntry(dummyHist1_p, inNickName1.c_str(), "P L");
  leg_p->AddEntry(dummyHist2_p, inNickName2.c_str(), "P L");

  std::vector<std::string> misMatchedTrees1;
  std::vector<std::string> misMatchedTrees2;

  std::string texFileName = inFileName1 + "_" + inFileName2;
  while(texFileName.find("/") != std::string::npos){texFileName.replace(0, texFileName.find("/")+1, "");}
  while(texFileName.find(".root") != std::string::npos){texFileName.replace(texFileName.find(".root"), 5, "_");}
  while(texFileName.substr(0,1).find("_") != std::string::npos){texFileName.replace(0,1,"");}
  while(texFileName.find("__") != std::string::npos){texFileName.replace(texFileName.find("__"),2,"_");}

  std::string fullDirName = "pdfDir/" + std::to_string(date.GetDate()) + "/" + texFileName;
  while(fullDirName.substr(fullDirName.size()-1,1).find("_") != std::string::npos){fullDirName.replace(fullDirName.size()-1, 1, "");}
  fullDirName = fullDirName + "/";

  std::cout << "File name 1: " << texFileName << std::endl;
  checkMakeDir(fullDirName.c_str());

  std::string texFileSubName = fullDirName + texFileName + "VALIDATION_SUB_" + std::to_string(date.GetDate()) + ".tex";
  std::cout << "File name 2: " << texFileSubName << std::endl;
  texFileName = fullDirName + texFileName + "VALIDATION_MAIN_" + std::to_string(date.GetDate()) + ".tex";
  std::ofstream fileTex(texFileName.c_str());
  std::ofstream fileSubTex(texFileSubName.c_str());
  doInterstitialTexSlide(&(fileSubTex), "PLOTS");

  TFile* inFile1_p = new TFile(inFileName1.c_str(), "READ");
  std::vector<std::string> file1Trees = returnRootFileContentsList(inFile1_p, "TTree", treeSelect);
  removeVectorDuplicates(&file1Trees);
  dumpTreeNames(inFileName1, file1Trees);

  TFile* inFile2_p = new TFile(inFileName2.c_str(), "READ");
  std::vector<std::string> file2Trees = returnRootFileContentsList(inFile2_p, "TTree", treeSelect);
  removeVectorDuplicates(&file2Trees);
  dumpTreeNames(inFileName2, file2Trees);

  std::cout << "Checking tree strings match..." << std::endl;
  if(doStringsMatch(&file1Trees, &file2Trees, &misMatchedTrees1, &misMatchedTrees2)) std::cout << "All trees match between files!" << std::endl;
  else std::cout << "Mismatched trees found and removed..." << std::endl;

  doFirstTexSlide(&fileTex, inFileName1, inFileName2, inNickName1, inNickName2, file1Trees, misMatchedTrees1, misMatchedTrees2, eventCountOverride);

  std::vector<int>* intVect_p=NULL;
  std::vector<float>* floatVect_p=NULL;
  std::vector<double>* doubleVect_p=NULL;

  for(unsigned int tI = 0; tI < file1Trees.size(); ++tI){
    std::cout << "Processing \'" << file1Trees.at(tI) << "\'..." << std::endl;

    TTree* tree1_p = (TTree*)inFile1_p->Get(file1Trees.at(tI).c_str());
    TObjArray* tempBranchList1 = (TObjArray*)tree1_p->GetListOfBranches();
    TObjArray* tempLeafList1 = (TObjArray*)tree1_p->GetListOfLeaves();
    std::vector<std::string> branchList1;
    std::vector<std::string> leafList1;
    std::vector<std::string> pdfList1;
    std::vector<std::string> warningList;
    std::vector<int> warningListPos;
    
    for(Int_t bI1 = 0; bI1 < tempBranchList1->GetEntries(); ++bI1){
      branchList1.push_back(tempBranchList1->At(bI1)->GetName());
      leafList1.push_back(tempLeafList1->At(bI1)->GetName());
    }

    TTree* tree2_p = (TTree*)inFile2_p->Get(file1Trees.at(tI).c_str());
    TObjArray* tempBranchList2 = (TObjArray*)tree2_p->GetListOfBranches();
    TObjArray* tempLeafList2 = (TObjArray*)tree2_p->GetListOfLeaves();
    std::vector<std::string> branchList2;
    std::vector<std::string> leafList2;
    for(Int_t bI2 = 0; bI2 < tempBranchList2->GetEntries(); ++bI2){
      branchList2.push_back(tempBranchList2->At(bI2)->GetName());
      leafList2.push_back(tempLeafList2->At(bI2)->GetName());
    }

    std::cout << "Ok we process " << branchList1.size() << " branches..." << std::endl;
    for(unsigned int bI = 0; bI < branchList1.size(); ++bI){
      std::cout << " " << bI << "/" << branchList1.size() << ": " << branchList1.at(bI) << std::endl;
    }

    std::vector<std::string> misMatchedBranches1;
    std::vector<std::string> misMatchedBranches2;

    std::vector<std::string> misMatchedLeaves1;
    std::vector<std::string> misMatchedLeaves2;

    std::cout << "Checking branch strings match..." << std::endl;
    if(doStringsMatch(&branchList1, &branchList2, &misMatchedBranches1, &misMatchedBranches2)) std::cout << "All branches match between files!" << std::endl;
    else std::cout << "Mismatched branches found and removed..." << std::endl;

    if(doStringsMatch(&leafList1, &leafList2, &misMatchedLeaves1, &misMatchedLeaves2)) std::cout << "All leaves match between files!" << std::endl;
    else std::cout << "Mismatched leaves found and removed..." << std::endl;

    //    doTreeTexSlide(&fileTex, file1Trees.at(tI), misMatchedBranches1, misMatchedBranches2);


    for(unsigned int bI1 = 0; bI1 < branchList1.size(); ++bI1){
      std::cout << " Processing \'" << branchList1.at(bI1) << "\'..." << std::endl;

      tree1_p->ResetBranchAddresses();
      tree1_p->SetBranchStatus("*", 0);
      tree1_p->SetBranchStatus(branchList1.at(bI1).c_str(), 1);

      tree2_p->ResetBranchAddresses();
      tree2_p->SetBranchStatus("*", 0);
      tree2_p->SetBranchStatus(branchList1.at(bI1).c_str(), 1);

      Double_t maxVal = TMath::Max(tree1_p->GetMaximum(branchList1.at(bI1).c_str()), tree2_p->GetMaximum(branchList1.at(bI1).c_str()));
      Double_t minVal = TMath::Min(tree1_p->GetMinimum(branchList1.at(bI1).c_str()), tree2_p->GetMinimum(branchList1.at(bI1).c_str()));
     
      TLeaf* tempLeaf = (TLeaf*)tree1_p->GetLeaf(leafList1.at(bI1).c_str());
      std::string tempClassType = tempLeaf->GetTypeName();

    
      //We have to handle vectors differently, getMax and getMin do not work from ttree
      if(tempClassType.find("vector") != std::string::npos && tempClassType.find("vector<vector") == std::string::npos){
	std::cout << "Searching for vector max/min of branch \'" << branchList1.at(bI1) << "\'" << std::endl;

	if(tempClassType.find("int") != std::string::npos){
	  std::cout << " Doing int search..." << std::endl;

	  tree1_p->SetBranchStatus("*", 0);
	  tree1_p->SetBranchStatus(branchList1.at(bI1).c_str(), 1);

	  tree1_p->SetBranchAddress(branchList1.at(bI1).c_str(), &intVect_p);

	  const Int_t nEntries1 = tree1_p->GetEntries();
	  tree1_p->GetEntry(0);

	  auto maxMin = std::minmax_element(intVect_p->begin(), intVect_p->end());
	  minVal = *(maxMin.first);
	  maxVal = *(maxMin.second);

	  for(Int_t entry = 1; entry < nEntries1; ++entry){
	    tree1_p->GetEntry(entry);

	    auto maxMin = std::minmax_element(intVect_p->begin(), intVect_p->end());
	    if(minVal > *(maxMin.first)) minVal = *(maxMin.first);
	    if(maxVal < *(maxMin.second)) maxVal = *(maxMin.second);
	  }

	  tree2_p->SetBranchStatus("*", 0);
	  tree2_p->SetBranchStatus(branchList2.at(bI1).c_str(), 1);

	  tree2_p->SetBranchAddress(branchList2.at(bI1).c_str(), &intVect_p);

	  const Int_t nEntries2 = tree2_p->GetEntries();
	  for(Int_t entry = 0; entry < nEntries2; ++entry){
	    tree2_p->GetEntry(entry);

	    auto maxMin = std::minmax_element(intVect_p->begin(), intVect_p->end());
	    if(minVal > *(maxMin.first)) minVal = *(maxMin.first);
	    if(maxVal < *(maxMin.second)) maxVal = *(maxMin.second);
	  }	 
	}
	else if(tempClassType.find("float") != std::string::npos){
	  std::cout << " Doing float search..." << std::endl;

	  tree1_p->SetBranchStatus("*", 0);
	  tree1_p->SetBranchStatus(branchList1.at(bI1).c_str(), 1);

	  tree1_p->SetBranchAddress(branchList1.at(bI1).c_str(), &floatVect_p);

	  const Int_t nEntries1 = tree1_p->GetEntries();
	  tree1_p->GetEntry(0);

	  auto maxMin = std::minmax_element(floatVect_p->begin(), floatVect_p->end());
	  minVal = *(maxMin.first);
	  maxVal = *(maxMin.second);

	  for(Int_t entry = 1; entry < nEntries1; ++entry){
	    tree1_p->GetEntry(entry);

	    auto maxMin = std::minmax_element(floatVect_p->begin(), floatVect_p->end());
	    if(minVal > *(maxMin.first)) minVal = *(maxMin.first);
	    if(maxVal < *(maxMin.second)) maxVal = *(maxMin.second);
	  }


	  tree2_p->SetBranchStatus("*", 0);
	  tree2_p->SetBranchStatus(branchList2.at(bI1).c_str(), 1);
	 
	  tree2_p->SetBranchAddress(branchList2.at(bI1).c_str(), &floatVect_p);

	  const Int_t nEntries2 = tree2_p->GetEntries();
	  for(Int_t entry = 0; entry < nEntries2; ++entry){
	    tree2_p->GetEntry(entry);

	    auto maxMin = std::minmax_element(floatVect_p->begin(), floatVect_p->end());
	    if(minVal > *(maxMin.first)) minVal = *(maxMin.first);
	    if(maxVal < *(maxMin.second)) maxVal = *(maxMin.second);
	  }	 
	}
	else if(tempClassType.find("double") != std::string::npos){
	  std::cout << " Doing double search..." << std::endl;

	  tree1_p->SetBranchStatus("*", 0);
	  tree1_p->SetBranchStatus(branchList1.at(bI1).c_str(), 1);

	  tree1_p->SetBranchAddress(branchList1.at(bI1).c_str(), &doubleVect_p);

	  const Int_t nEntries1 = tree1_p->GetEntries();
	  tree1_p->GetEntry(0);

	  auto maxMin = std::minmax_element(doubleVect_p->begin(), doubleVect_p->end());
	  minVal = *(maxMin.first);
	  maxVal = *(maxMin.second);

	  for(Int_t entry = 1; entry < nEntries1; ++entry){
	    tree1_p->GetEntry(entry);

	    auto maxMin = std::minmax_element(doubleVect_p->begin(), doubleVect_p->end());
	    if(minVal > *(maxMin.first)) minVal = *(maxMin.first);
	    if(maxVal < *(maxMin.second)) maxVal = *(maxMin.second);
	  }


	  tree2_p->SetBranchStatus("*", 0);
	  tree2_p->SetBranchStatus(branchList2.at(bI1).c_str(), 1);

	  tree2_p->SetBranchAddress(branchList2.at(bI1).c_str(), &doubleVect_p);

	  const Int_t nEntries2 = tree2_p->GetEntries();
	  for(Int_t entry = 0; entry < nEntries2; ++entry){
	    tree2_p->GetEntry(entry);

	    auto maxMin = std::minmax_element(doubleVect_p->begin(), doubleVect_p->end());
	    if(minVal > *(maxMin.first)) minVal = *(maxMin.first);
	    if(maxVal < *(maxMin.second)) maxVal = *(maxMin.second);
	  }	 
	}
	else{
	  std::cout << "Warning do not know how to handle vector \'" << tempClassType << "\'. return 1" << std::endl;
	  return 1;
	}
      }

      std::string histName1 = file1Trees.at(tI) + "_" + branchList1.at(bI1) + "_FirstFile_" + inNickName1 + "_h";
      std::string histName2 = file1Trees.at(tI) + "_" + branchList1.at(bI1) + "_SecondFile_" + inNickName2 + "_h";
      while(histName1.find("/") != std::string::npos){histName1.replace(histName1.find("/"), 1, "_");}
      while(histName2.find("/") != std::string::npos){histName2.replace(histName2.find("/"), 1, "_");}

      TCanvas* canv_p = new TCanvas("temp", "temp", 450, 400);
      canv_p->SetTopMargin(0.01);
      canv_p->SetRightMargin(0.01);
      canv_p->SetBottomMargin(0.01);
      canv_p->SetLeftMargin(0.01);
      TPad* pads_p[2];
      pads_p[0] = new TPad("pads0", "", 0.0, 0.35, 1.0, 1.0);
      canv_p->cd();
      pads_p[0]->Draw("SAME");
      pads_p[0]->SetTopMargin(0.01);
      pads_p[0]->SetRightMargin(0.01);
      pads_p[0]->SetBottomMargin(0.0);

      pads_p[1] = new TPad("pads1", "", 0.0, 0.00, 1.0, 0.35);
      canv_p->cd();
      pads_p[1]->Draw("SAME");
      pads_p[1]->SetTopMargin(0.0);
      pads_p[1]->SetRightMargin(0.01);
      pads_p[1]->SetBottomMargin(pads_p[1]->GetLeftMargin()*3.);

      if(TMath::Abs(minVal - maxVal) < 0.000000001){
	minVal -= 1;
	maxVal +=1;

	std::cout << "Resetting min/maxVal: " << branchList1.at(bI1) << std::endl;
      }
      else std::cout << "Not-resetting: " << branchList1.at(bI1) << std::endl;

      tree1_p->ResetBranchAddresses();
      tree1_p->SetBranchStatus("*", 0);
      tree1_p->SetBranchStatus(branchList1.at(bI1).c_str(), 1);

      tree2_p->ResetBranchAddresses();
      tree2_p->SetBranchStatus("*", 0);
      tree2_p->SetBranchStatus(branchList1.at(bI1).c_str(), 1);

      TH1D* tempHist1_p = new TH1D(histName1.c_str(), (";" + branchList1.at(bI1) + ";Counts").c_str(), 50, minVal, maxVal);
      TH1D* tempHist2_p = new TH1D(histName2.c_str(), (";" + branchList1.at(bI1) + ";Counts").c_str(), 50, minVal, maxVal);


      if(eventCountOverride < 0){
	std::cout << branchList1.at(bI1) << " >> " << histName1 << "(50, " << std::to_string(minVal) << ", " << std::to_string(maxVal) << ")" << std::endl;
	std::cout << branchList1.at(bI1) << " >> " << histName2 << "(50, " << std::to_string(minVal) << ", " << std::to_string(maxVal) << ")" << std::endl;

	//	tree1_p->Draw((branchList1.at(bI1) + " >> " + histName1 + "(50, " + std::to_string(minVal) + ", " + std::to_string(maxVal) + ")").c_str(), "", "");
	//	tree2_p->Draw((branchList1.at(bI1) + " >> " + histName2 + "(50, " + std::to_string(minVal) + ", " + std::to_string(maxVal) + ")").c_str(), "", "");

	tree1_p->Project(histName1.c_str(), branchList1.at(bI1).c_str(), "", "");
	tree2_p->Project(histName2.c_str(), branchList1.at(bI1).c_str(), "", "");
      }
      else{
	tree1_p->Project(histName1.c_str(), branchList1.at(bI1).c_str(), "", "", eventCountOverride);
	tree2_p->Project(histName2.c_str(), branchList1.at(bI1).c_str(), "", "", eventCountOverride);
      }

      std::cout << __LINE__ << std::endl;

      //      TH1D* tempHist1_p = (TH1D*)gDirectory->Get(histName1.c_str());
      tempHist1_p->GetXaxis()->SetTitle(branchList1.at(bI1).c_str());
      tempHist1_p->GetYaxis()->SetTitle("Counts");

      tempHist1_p->SetMarkerSize(0.8);
      tempHist1_p->SetMarkerStyle(20);
      tempHist1_p->SetMarkerColor(col.getColor(2));
      tempHist1_p->SetLineColor(col.getColor(2));

      tempHist1_p->GetXaxis()->SetTitleFont(43);
      tempHist1_p->GetYaxis()->SetTitleFont(43);
      tempHist1_p->GetXaxis()->SetLabelFont(43);
      tempHist1_p->GetYaxis()->SetLabelFont(43);

      tempHist1_p->GetXaxis()->SetTitleSize(12);
      tempHist1_p->GetYaxis()->SetTitleSize(12);
      tempHist1_p->GetXaxis()->SetLabelSize(12);
      tempHist1_p->GetYaxis()->SetLabelSize(12);
      
      tempHist1_p->GetXaxis()->SetTitleOffset(tempHist1_p->GetXaxis()->GetTitleOffset()*4.);
      tempHist1_p->GetYaxis()->SetTitleOffset(tempHist1_p->GetXaxis()->GetTitleOffset()/3.);

      std::cout << __LINE__ << std::endl;

      //      TH1D* tempHist2_p = (TH1D*)gDirectory->Get(histName2.c_str());
      tempHist2_p->GetXaxis()->SetTitle(branchList1.at(bI1).c_str());
      tempHist2_p->GetYaxis()->SetTitle("Counts");
      tempHist2_p->SetMarkerSize(0.8);
      tempHist2_p->SetMarkerStyle(25);
      tempHist2_p->SetMarkerColor(col.getColor(0));
      tempHist2_p->SetLineColor(col.getColor(0));

      centerTitles({tempHist1_p, tempHist2_p});
      setSumW2({tempHist1_p, tempHist2_p});

      std::cout << __LINE__ << std::endl;
      if(branchList1.at(bI1).find("jtphi") != std::string::npos){
	tempHist1_p->Print("ALL");
      }

      
      std::cout << __LINE__ << std::endl;

      maxVal = tempHist1_p->GetMinimum();
      minVal = tempHist1_p->GetMaximum();
      
      for(Int_t bIX = 0; bIX < tempHist1_p->GetNbinsX(); ++bIX){
	if(tempHist1_p->GetBinContent(bIX+1) != 0){
	  if(tempHist1_p->GetBinContent(bIX+1) > maxVal) maxVal = tempHist1_p->GetBinContent(bIX+1);
	  if(tempHist1_p->GetBinContent(bIX+1) < minVal) minVal = tempHist1_p->GetBinContent(bIX+1);
	}
	if(tempHist2_p->GetBinContent(bIX+1) != 0){
	  if(tempHist2_p->GetBinContent(bIX+1) > maxVal) maxVal = tempHist2_p->GetBinContent(bIX+1);
	  if(tempHist2_p->GetBinContent(bIX+1) < minVal) minVal = tempHist2_p->GetBinContent(bIX+1);
	}
      }


      std::cout << __LINE__ << std::endl;

      Double_t interval = maxVal - minVal;

      if(interval > 1000 && minVal > 0){
	maxVal *= 10;
	minVal /= 10;
      }
      else{
	maxVal += interval/10.;
	if(minVal - interval/10 > 0 || minVal < 0) minVal -= interval/10;
	else minVal = 0;
      }


      std::cout << __LINE__ << std::endl;

      tempHist1_p->SetMaximum(maxVal);
      tempHist1_p->SetMinimum(minVal);

      canv_p->cd();
      pads_p[0]->cd();

      tempHist1_p->DrawCopy("E1 P");
      tempHist2_p->DrawCopy("E1 P SAME");

      leg_p->Draw("SAME");



      std::cout << __LINE__ << std::endl;
      if(interval > 1000 && minVal > 0) gPad->SetLogy();
      gStyle->SetOptStat(0);
      gPad->RedrawAxis();
      gPad->SetTicks(0, 1);

      canv_p->cd();
      pads_p[1]->cd();


      std::cout << __LINE__ << std::endl;

      tempHist1_p->Divide(tempHist2_p);
      bool isGood = true;
      std::cout << __LINE__ << std::endl;
      for(Int_t bIX = 0; bIX < tempHist1_p->GetNbinsX(); ++bIX){
	std::cout << __LINE__ << std::endl;

	if(TMath::Abs(tempHist2_p->GetBinContent(bIX+1)) < minDeltaVal){
	  if(TMath::Abs(tempHist1_p->GetBinContent(bIX+1)) < minDeltaVal) continue;
	  else{
	    isGood = false;
	    break;
	  }
	}

	std::cout << __LINE__ << std::endl;

	if(TMath::Abs(tempHist1_p->GetBinContent(bIX+1) - 1.) > minDeltaVal){
	  isGood = false;
	  break;
	}
      }
      std::cout << __LINE__ << std::endl;

      if(!isGood){
	warningList.push_back(branchList1.at(bI1));
	warningListPos.push_back(bI1);
      }


      std::cout << __LINE__ << std::endl;

      tempHist1_p->SetMarkerColor(1);
      tempHist1_p->SetLineColor(1);
      tempHist1_p->GetYaxis()->SetTitle((inNickName1 + "/" + inNickName2).c_str());
      tempHist1_p->SetMaximum(TMath::Min(2., tempHist1_p->GetMaximum()));
      //      tempHist1_p->SetMinimum(0.7);
      tempHist1_p->DrawCopy("E1 P");
      line.DrawLine(tempHist1_p->GetBinLowEdge(1), 1, tempHist1_p->GetBinLowEdge(201), 1);
      gPad->RedrawAxis();
      gPad->SetTicks(0, 1);

      std::string saveName = histName1 + "_" + std::to_string(date.GetDate()) + ".pdf";
      pdfList1.push_back(saveName);
      canv_p->SaveAs((fullDirName + saveName).c_str());

      std::cout << __LINE__ << std::endl;

      delete tempHist1_p;
      delete tempHist2_p;

      std::cout << __LINE__ << std::endl;

      delete pads_p[0];
      delete pads_p[1];
      delete canv_p;
    }

    doTreeTexSlide(&fileTex, file1Trees.at(tI), misMatchedBranches1, misMatchedBranches2, warningList, warningListPos);
    //    doTreeTexSlide(&fileSubTex, file1Trees.at(tI), misMatchedBranches1, misMatchedBranches2, warningList, warningListPos);
    doBranchTexSlide(&fileSubTex, file1Trees.at(tI), branchList1);
    for(unsigned int bI1 = 0; bI1 < branchList1.size(); ++bI1){
      doPlotTexSlide(&fileSubTex, file1Trees.at(tI), branchList1.at(bI1), bI1, pdfList1.at(bI1));
    }
  }

  delete leg_p;
  delete dummyHist1_p;
  delete dummyHist2_p;

  inFile2_p->Close();
  delete inFile2_p;

  inFile1_p->Close();
  delete inFile1_p;

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

  if(argc < 3 || argc > 7){
    std::cout << "Usage: ./bin/runForestDQM.exe <inFileName1> <inFileName2> <inNickName1> <inNickName2> <treeSelect> <eventCountOverride>" << std::endl;
    return 1;
  }

  int retVal = 0;
  if(argc == 3) retVal += runForestDQM(argv[1], argv[2]);
  else if(argc == 4) retVal += runForestDQM(argv[1], argv[2], argv[3]);
  else if(argc == 5) retVal += runForestDQM(argv[1], argv[2], argv[3], argv[4]);
  else if(argc == 6) retVal += runForestDQM(argv[1], argv[2], argv[3], argv[4], argv[5]);
  else if(argc == 7){
    std::cout << "WARNING, EVENT COUNT OVERRIDE GIVEN, FOR TESTING ONLY, NOT VALID CHECK" << std::endl;
    retVal += runForestDQM(argv[1], argv[2], argv[3], argv[4], argv[5], std::stoi(argv[6 ]));
  }

  watch.stop();
  std::cout << "Timing: " << watch.total() << std::endl;
  
  return retVal;
}
