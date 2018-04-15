///AUTHOR: CFMCGINN (2018.04.12)
///For validating forest
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TObjArray.h"
#include "TMath.h"
#include "TDatime.h"
#include "TPad.h"
#include "TStyle.h"
#include "TLine.h"
#include "TLegend.h"

#include "include/checkMakeDir.h"
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

void doFirstTexSlide(std::ofstream* fileTex, std::string inFileName1, std::string inFileName2, std::vector<std::string> goodTrees, std::vector<std::string> missingTrees1, std::vector<std::string> missingTrees2, const Int_t eventCountOverride)
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
  (*fileTex) << "\\hypersetup{colorlinks,linkcolor=,urlcolor=links}" << std::endl;

  (*fileTex) << std::endl;

  (*fileTex) << "\\author[CM]{CMSHI Forest Validatory}" << std::endl;
  (*fileTex) << "\\begin{document}" << std::endl;

  (*fileTex) << std::endl;

  (*fileTex) << "\\begin{frame}" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{Forest Validation (" << date.GetYear() << "." << date.GetMonth() << "." << date.GetDay() << ")}}" << std::endl;
  (*fileTex) << " \\begin{itemize}" << std::endl;
  (*fileTex) << "  \\fontsize{5}{5}\\selectfont" << std::endl;
  if(eventCountOverride >= 0) (*fileTex) << " \\item{WARNING: EVENT NUMBER OVERRIDE SET, TEST INVALID}" << std::endl;
  (*fileTex) << "  \\item{File 1: \'" << texInFileName1 << "\'}" << std::endl;
  (*fileTex) << "  \\item{File 2: \'" << texInFileName2 << "\'}" << std::endl;
  (*fileTex) << "  \\item{Good Trees:}" << std::endl;

  (*fileTex) << " \\begin{itemize}" << std::endl;
  (*fileTex) << "  \\fontsize{5}{5}\\selectfont" << std::endl;
  if(goodTrees.size() != 0){
    for(unsigned int tI = 0; tI < goodTrees.size(); ++tI){
      (*fileTex) << "  \\item{Tree " << tI << ": \'" << goodTrees.at(tI) << "\'}" << std::endl;
    }
  }
  else (*fileTex) << "  \\item{WARNING: NO GOOD TREES}" << std::endl;
  (*fileTex) << " \\end{itemize}" << std::endl;

  (*fileTex) << "  \\item{Bad File 1 Trees:}" << std::endl;

  (*fileTex) << " \\begin{itemize}" << std::endl;
  (*fileTex) << "  \\fontsize{5}{5}\\selectfont" << std::endl;
  if(missingTrees1.size() != 0){
    for(unsigned int tI = 0; tI < missingTrees1.size(); ++tI){
      (*fileTex) << "  \\item{Tree " << tI << ": \'" << missingTrees1.at(tI) << "\'}" << std::endl;
    }
  }
  else (*fileTex) << "  \\item{NO MISSING TREES}" << std::endl;
  (*fileTex) << " \\end{itemize}" << std::endl;

  (*fileTex) << "  \\item{Bad File 2 Trees:}" << std::endl;

  (*fileTex) << " \\begin{itemize}" << std::endl;
  (*fileTex) << "  \\fontsize{5}{5}\\selectfont" << std::endl;
  if(missingTrees2.size() != 0){
    for(unsigned int tI = 0; tI < missingTrees2.size(); ++tI){
      (*fileTex) << "  \\item{Tree " << tI << ": \'" << missingTrees2.at(tI) << "\'}" << std::endl;
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


void doTreeTexSlide(std::ofstream* fileTex, std::string inTreeName, std::vector<std::string> listOfBadVar1, std::vector<std::string> listOfBadVar2, std::vector<std::string> warningList)
{
  std::string treeTitle = texFriendlyString(inTreeName);
  while(treeTitle.find("/") != std::string::npos){
    treeTitle.replace(0, treeTitle.find("/")+1, "");
  }

  (*fileTex) << "\\begin{frame}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{" << treeTitle << "}}" << std::endl;
  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  (*fileTex) << "\\item{Tree Name \'" << texFriendlyString(inTreeName) << "\'}" << std::endl;
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
  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  if(warningList.size() != 0){
    for(unsigned int vI = 0; vI < warningList.size(); ++vI){
      (*fileTex) << "\\item{Var " << vI << "/" << warningList.size() << ": \'" << texFriendlyString(warningList.at(vI)) << "\'}" << std::endl;
    }
  }
  else (*fileTex) << "\\item{No deviating variables}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;

  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\end{frame}" << std::endl;

  (*fileTex) << std::endl;

  return;
}


void doBranchTexSlide(std::ofstream* fileTex, std::string inTreeName, std::vector<std::string> listOfBranches)
{
  std::string treeTitle = texFriendlyString(inTreeName);
  while(treeTitle.find("/") != std::string::npos){
    treeTitle.replace(0, treeTitle.find("/")+1, "");
  }
  
  (*fileTex) << "\\begin{frame}" << std::endl;
  (*fileTex) << "\\fontsize{4}{4}\\selectfont" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{" << treeTitle << "}}" << std::endl;
  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{4}{4}\\selectfont" << std::endl;
  (*fileTex) << "\\item{Tree Name \'" << texFriendlyString(inTreeName) << "\'}" << std::endl;
  (*fileTex) << "\\item{Branches}" << std::endl;
  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{4}{4}\\selectfont" << std::endl;
  if(listOfBranches.size() != 0){
    for(unsigned int vI = 0; vI < listOfBranches.size(); ++vI){
      (*fileTex) << "\\item{Var " << vI << "/" << listOfBranches.size() << ": \'" << texFriendlyString(listOfBranches.at(vI)) << "\'}" << std::endl;
    }
  }
  else (*fileTex) << "\\item{No branches!}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\end{frame}" << std::endl;

  (*fileTex) << std::endl;

  return;
}


void doPlotTexSlide(std::ofstream* fileTex, std::string inTreeName, std::string inBranchName, std::string inPdfName)
{
  std::string treeTitle = texFriendlyString(inTreeName);
  while(treeTitle.find("/") != std::string::npos){
    treeTitle.replace(0, treeTitle.find("/")+1, "");
  }

  (*fileTex) << "\\begin{frame}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  (*fileTex) << "\\frametitle{\\centerline{" << texFriendlyString(inBranchName) << "(" << treeTitle << ")}}" << std::endl;

  (*fileTex) << "\\begin{center}" << std::endl;
  (*fileTex) << "\\includegraphics[width=0.6\\textwidth]{" << inPdfName << "}" << std::endl;
  (*fileTex) << "\\end{center}" << std::endl;

  (*fileTex) << "\\begin{itemize}" << std::endl;
  (*fileTex) << "\\fontsize{5}{5}\\selectfont" << std::endl;
  (*fileTex) << "\\item{" << texFriendlyString(inBranchName) << "}" << std::endl;
  (*fileTex) << "\\end{itemize}" << std::endl;
  (*fileTex) << "\\end{frame}" << std::endl;

  (*fileTex) << std::endl;

  return;
}


int runForestDQM(const std::string inFileName1, const std::string inFileName2, const Int_t eventCountOverride = -1)
{
  if(inFileName1.find(".root") == std::string::npos || !checkFile(inFileName1)){
    std::cout << "Input \'" << inFileName1 << "\' is invalid return 1." << std::endl;
    return 1;
  }
  else if(inFileName2.find(".root") == std::string::npos || !checkFile(inFileName2)){
    std::cout << "Input \'" << inFileName2 << "\' is invalid return 1." << std::endl;
    return 1;
  }

  Double_t minDeltaVal = 0.000000001;
  checkMakeDir("pdfDir");
  kirchnerPalette col;
  TDatime date;
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

  leg_p->AddEntry(dummyHist1_p, "File1", "P L");
  leg_p->AddEntry(dummyHist2_p, "File2", "P L");

  std::vector<std::string> misMatchedTrees1;
  std::vector<std::string> misMatchedTrees2;

  std::string texFileName = inFileName1 + "_" + inFileName2;
  while(texFileName.find("/") != std::string::npos){texFileName.replace(texFileName.find("/"), 1, "_");}
  while(texFileName.find(".root") != std::string::npos){texFileName.replace(texFileName.find(".root"), 5, "_");}
  while(texFileName.substr(0,1).find("_") != std::string::npos){texFileName.replace(0,1,"");}
  while(texFileName.find("__") != std::string::npos){texFileName.replace(texFileName.find("__"),2,"_");}

  std::string texFileSubName = "pdfDir/" + texFileName + "VALIDATION_SUB_" + std::to_string(date.GetDate()) + ".tex";
  texFileName = "pdfDir/" + texFileName + "VALIDATION_MAIN_" + std::to_string(date.GetDate()) + ".tex";
  std::ofstream fileTex(texFileName.c_str());
  std::ofstream fileSubTex(texFileSubName.c_str());
  doInterstitialTexSlide(&(fileSubTex), "PLOTS");

  TFile* inFile1_p = new TFile(inFileName1.c_str(), "READ");
  std::vector<std::string> file1Trees = returnRootFileContentsList(inFile1_p, "TTree", "");
  removeVectorDuplicates(&file1Trees);
  dumpTreeNames(inFileName1, file1Trees);

  TFile* inFile2_p = new TFile(inFileName2.c_str(), "READ");
  std::vector<std::string> file2Trees = returnRootFileContentsList(inFile2_p, "TTree", "");
  removeVectorDuplicates(&file2Trees);
  dumpTreeNames(inFileName2, file2Trees);

  file2Trees.push_back("Dummy");

  std::cout << "Checking tree strings match..." << std::endl;
  if(doStringsMatch(&file1Trees, &file2Trees, &misMatchedTrees1, &misMatchedTrees2)) std::cout << "All trees match between files!" << std::endl;
  else std::cout << "Mismatched trees found and removed..." << std::endl;

  doFirstTexSlide(&fileTex, inFileName1, inFileName2, file1Trees, misMatchedTrees1, misMatchedTrees2, eventCountOverride);

  for(unsigned int tI = 0; tI < file1Trees.size(); ++tI){
    std::cout << "Processing \'" << file1Trees.at(tI) << "\'..." << std::endl;

    TTree* tree1_p = (TTree*)inFile1_p->Get(file1Trees.at(tI).c_str());
    TObjArray* tempBranchList1 = (TObjArray*)tree1_p->GetListOfBranches();
    std::vector<std::string> branchList1;
    std::vector<std::string> pdfList1;
    std::vector<std::string> warningList;
    for(Int_t bI1 = 0; bI1 < tempBranchList1->GetEntries(); ++bI1){
      branchList1.push_back(tempBranchList1->At(bI1)->GetName());
    }


    TTree* tree2_p = (TTree*)inFile2_p->Get(file1Trees.at(tI).c_str());
    TObjArray* tempBranchList2 = (TObjArray*)tree2_p->GetListOfBranches();
    std::vector<std::string> branchList2;
    for(Int_t bI2 = 0; bI2 < tempBranchList2->GetEntries(); ++bI2){
      branchList2.push_back(tempBranchList2->At(bI2)->GetName());
    }
    
    std::vector<std::string> misMatchedBranches1;
    std::vector<std::string> misMatchedBranches2;

    std::cout << "Checking branch strings match..." << std::endl;
    if(doStringsMatch(&branchList1, &branchList2, &misMatchedBranches1, &misMatchedBranches2)) std::cout << "All branches match between files!" << std::endl;
    else std::cout << "Mismatched branches found and removed..." << std::endl;

    //    doTreeTexSlide(&fileTex, file1Trees.at(tI), misMatchedBranches1, misMatchedBranches2);


    for(unsigned int bI1 = 0; bI1 < branchList1.size(); ++bI1){
      std::cout << " Processing \'" << branchList1.at(bI1) << "\'..." << std::endl;
      Double_t maxVal = TMath::Max(tree1_p->GetMaximum(branchList1.at(bI1).c_str()), tree2_p->GetMaximum(branchList1.at(bI1).c_str()));
      Double_t minVal = TMath::Min(tree1_p->GetMinimum(branchList1.at(bI1).c_str()), tree2_p->GetMinimum(branchList1.at(bI1).c_str()));
     
      std::string histName1 = file1Trees.at(tI) + "_" + branchList1.at(bI1) + "_FirstFile_h";
      std::string histName2 = file1Trees.at(tI) + "_" + branchList1.at(bI1) + "_SecondFile_h";
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

      TH1D* tempHist1_p = new TH1D(histName1.c_str(), (";" + branchList1.at(bI1) + ";Counts").c_str(), 200, minVal, maxVal);
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

      TH1D* tempHist2_p = new TH1D(histName2.c_str(), (";" + branchList1.at(bI1) + ";Counts").c_str(), 200, minVal, maxVal);
      tempHist2_p->SetMarkerSize(0.8);
      tempHist2_p->SetMarkerStyle(25);
      tempHist2_p->SetMarkerColor(col.getColor(0));
      tempHist2_p->SetLineColor(col.getColor(0));

      centerTitles({tempHist1_p, tempHist2_p});
      setSumW2({tempHist1_p, tempHist2_p});

      if(eventCountOverride < 0){
	tree1_p->Project(histName1.c_str(), branchList1.at(bI1).c_str(), "", "");
	tree2_p->Project(histName2.c_str(), branchList1.at(bI1).c_str(), "", "");
      }
      else{
	tree1_p->Project(histName1.c_str(), branchList1.at(bI1).c_str(), "", "", eventCountOverride);
	tree2_p->Project(histName2.c_str(), branchList1.at(bI1).c_str(), "", "", eventCountOverride);
      }

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

      tempHist1_p->SetMaximum(maxVal);
      tempHist1_p->SetMinimum(minVal);

      canv_p->cd();
      pads_p[0]->cd();

      tempHist1_p->DrawCopy("E1 P");
      tempHist2_p->DrawCopy("E1 P SAME");

      leg_p->Draw("SAME");

      if(interval > 1000 && minVal > 0) gPad->SetLogy();
      gStyle->SetOptStat(0);
      gPad->RedrawAxis();
      gPad->SetTicks(0, 1);

      canv_p->cd();
      pads_p[1]->cd();

      tempHist1_p->Divide(tempHist2_p);
      bool isGood = true;
      for(Int_t bIX = 0; bIX < tempHist1_p->GetNbinsX(); ++bIX){
	if(TMath::Abs(tempHist2_p->GetBinContent(bIX+1)) < minDeltaVal){
	  if(TMath::Abs(tempHist1_p->GetBinContent(bIX+1)) < minDeltaVal) continue;
	  else{
	    isGood = false;
	    break;
	  }
	}

	if(TMath::Abs(tempHist1_p->GetBinContent(bIX+1) - 1.) > minDeltaVal){
	  isGood = false;
	  break;
	}
      }

      if(!isGood) warningList.push_back(branchList1.at(bI1));

      tempHist1_p->SetMarkerColor(1);
      tempHist1_p->SetLineColor(1);
      tempHist1_p->GetYaxis()->SetTitle("File1/File2");
      tempHist1_p->SetMaximum(1.3);
      tempHist1_p->SetMinimum(0.7);
      tempHist1_p->DrawCopy("E1 P");
      line.DrawLine(tempHist1_p->GetBinLowEdge(1), 1, tempHist1_p->GetBinLowEdge(201), 1);
      gPad->RedrawAxis();
      gPad->SetTicks(0, 1);

      std::string saveName = histName1 + "_" + std::to_string(date.GetDate()) + ".pdf";
      pdfList1.push_back(saveName);
      canv_p->SaveAs(("pdfDir/" + saveName).c_str());

      delete tempHist1_p;
      delete tempHist2_p;
      delete pads_p[0];
      delete pads_p[1];
      delete canv_p;
    }

    doTreeTexSlide(&fileTex, file1Trees.at(tI), misMatchedBranches1, misMatchedBranches2, warningList);
    doTreeTexSlide(&fileSubTex, file1Trees.at(tI), misMatchedBranches1, misMatchedBranches2, warningList);
    doBranchTexSlide(&fileSubTex, file1Trees.at(tI), branchList1);
    for(unsigned int bI1 = 0; bI1 < branchList1.size(); ++bI1){
      doPlotTexSlide(&fileSubTex, file1Trees.at(tI), branchList1.at(bI1), pdfList1.at(bI1));
    }
  }

  

  delete leg_p;
  delete dummyHist1_p;
  delete dummyHist2_p;

  inFile2_p->Close();
  delete inFile2_p;

  inFile1_p->Close();
  delete inFile1_p;

  if(texFileSubName.find("pdfDir/") != std::string::npos){
    texFileSubName.replace(texFileSubName.find("pdfDir/"), 7, "");
  }

  fileTex << "\\input{" << texFileSubName << "}" << std::endl;
  fileTex << "\\end{document}" << std::endl;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 3 && argc != 4){
    std::cout << "Usage: ./bin/runForestDQM.exe <inFileName1> <inFileName2> <eventCountOverride>" << std::endl;
    return 1;
  }

  int retVal = 0;
  if(argc == 3) retVal += runForestDQM(argv[1], argv[2]);
  else if(argc == 4){
    std::cout << "WARNING, EVENT COUNT OVERRIDE GIVEN, FOR TESTING ONLY, NOT VALID CHECK" << std::endl;
    retVal += runForestDQM(argv[1], argv[2], std::stoi(argv[3]));
  }
  return retVal;
}
