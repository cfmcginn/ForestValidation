//cpp dependencies
#include <iostream>
#include <string>
#include <vector>

//ROOT dependencies
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TH1D.h"
#include "TDatime.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TLatex.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/getLinBins.h"
#include "include/mntToXRootdFileString.h"
#include "include/histDefUtility.h"
#include "include/plotUtilities.h"
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"

int checkEventSelection(const std::string inFileName, const std::string optionalTag = "")
{
  if(!checkFile(inFileName) || inFileName.find(".root") == std::string::npos){
    std::cout << "Given \'" << inFileName << "\' is not a valid root file. return 1" << std::endl;
    return 1;
  }
  
  TFile* inFile_p = TFile::Open(mntToXRootdFileString(inFileName).c_str(), "READ");
  std::vector<std::string> treeList = returnRootFileContentsList(inFile_p, "TTree", "");
  
  bool hasHITree = false;
  bool hasSkimTree = false;
  
  for(unsigned int tI = 0; tI < treeList.size(); ++tI){
    if(isStrSame("hiEvtAnalyzer/HiTree", treeList.at(tI))) hasHITree = true;
    else if(isStrSame("skimanalysis/HltTree", treeList.at(tI))) hasSkimTree = true;
  }

  if(!hasHITree || !hasSkimTree){
    if(!hasHITree) std::cout << "Given inFile \'" << inFileName << "\' does not contain HiTree. return 1" << std::endl;
    if(!hasSkimTree) std::cout << "Given inFile \'" << inFileName << "\' does not contain skimTree. return 1" << std::endl;
    
    inFile_p->Close();
    delete inFile_p;
    return 1;
  }
  
  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);

  checkMakeDir("pdfDir");
  checkMakeDir("pdfDir/" + dateStr);

  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  if(outFileName.find(".root") != std::string::npos) outFileName.replace(outFileName.find(".root"), outFileName.size(), "");
  outFileName = "output/" + dateStr + "/" + outFileName + "_CheckForestEventSelection_" + dateStr + ".root";
  
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TH1D* hiBinPre_h = new TH1D("hiBinPre_h", ";hiBin;Counts/0.5%", 200, -0.5, 199.5);
  TH1D* hiBinPost_h = new TH1D("hiBinPost_h", ";hiBin;Counts/0.5%", 200, -0.5, 199.5);
  centerTitles({hiBinPre_h, hiBinPost_h});

  inFile_p->cd();
  TTree* hiTree_p = (TTree*)inFile_p->Get("hiEvtAnalyzer/HiTree");
  TTree* skimTree_p = (TTree*)inFile_p->Get("skimanalysis/HltTree");

  Float_t vz_;
  Float_t hiHF_;
  Int_t hiBin_;

  hiTree_p->SetBranchStatus("*", 0);
  hiTree_p->SetBranchStatus("vz", 1);
  hiTree_p->SetBranchStatus("hiHF", 1);
  hiTree_p->SetBranchStatus("hiBin", 1);
  
  hiTree_p->SetBranchAddress("vz", &vz_);
  hiTree_p->SetBranchAddress("hiHF", &hiHF_);
  hiTree_p->SetBranchAddress("hiBin", &hiBin_);

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
  
  //  skimTree_p->SetBranchAddress("pcollisionEventSelection", &pcollisionEventSelection_);
  skimTree_p->SetBranchAddress("HBHENoiseFilterResultRun2Loose", &HBHENoiseFilterResultRun2Loose_);
  skimTree_p->SetBranchAddress("pprimaryVertexFilter", &pprimaryVertexFilter_);
  skimTree_p->SetBranchAddress("phfCoincFilter3", &phfCoincFilter3_);
  skimTree_p->SetBranchAddress("pclusterCompatibilityFilter", &pclusterCompatibilityFilter_);

  const Int_t nEntries = hiTree_p->GetEntries();

  Int_t vzCounter = 0;
  Int_t hiHFCounter = 0;
  //  Int_t pcollCounter = 0;
  Int_t hbheCounter = 0;
  Int_t pprimCounter = 0;
  Int_t phfCounter = 0;
  Int_t pclustCounter = 0;

  std::cout << "Processing " << nEntries << " events..." << optionalTag << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    hiTree_p->GetEntry(entry);
    skimTree_p->GetEntry(entry);

    hiBinPre_h->Fill(hiBin_);

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

    hiBinPost_h->Fill(hiBin_);
  }

  std::cout << "Total Entries: " << nEntries << std::endl;
  std::cout << " Passing vz: " << vzCounter << " (" << prettyString(vzCounter*100./(Double_t)nEntries, 2, false) << ")" << std::endl;
  std::cout << " Passing hiHF+prev: " << hiHFCounter << " (" << prettyString(hiHFCounter*100./(Double_t)nEntries, 2, false) << ")" << std::endl;
  std::cout << " Passing hbhe+prev: " << hbheCounter << " (" << prettyString(hbheCounter*100./(Double_t)nEntries, 2, false) << ")" << std::endl;
  std::cout << " Passing pprim+prev: " << pprimCounter << " (" << prettyString(pprimCounter*100./(Double_t)nEntries, 2, false) << ")" << std::endl;
  std::cout << " Passing phf+prev: " << phfCounter << " (" << prettyString(phfCounter*100./(Double_t)nEntries, 2, false) << ")" << std::endl;
  std::cout << " Passing pclust+prev: " << pclustCounter << " (" << prettyString(pclustCounter*100./(Double_t)nEntries, 2, false) << ")" << std::endl;

  outFile_p->cd();

  hiBinPre_h->SetMinimum(0.0);
  hiBinPost_h->SetMinimum(0.0);

  Double_t max = TMath::Max(hiBinPre_h->GetMaximum(), hiBinPost_h->GetMaximum());
  max += TMath::Sqrt(max);
  if(max < 2.* hiBinPre_h->GetBinContent(1) || max < 2.* hiBinPost_h->GetBinContent(1)) max *= 2;

  hiBinPre_h->SetMaximum(max);
  hiBinPost_h->SetMaximum(max);

  TCanvas* canv_p = new TCanvas("canv_p", "", 450, 450);
  canv_p->SetTopMargin(0.01);
  canv_p->SetRightMargin(0.01);
  canv_p->SetLeftMargin(0.14);
  canv_p->SetBottomMargin(0.14);

  hiBinPre_h->SetMarkerColor(1);
  hiBinPre_h->SetMarkerSize(0.01);
  hiBinPre_h->SetMarkerStyle(20);
  hiBinPre_h->SetLineColor(1);
  
  hiBinPost_h->SetMarkerColor(kRed);
  hiBinPost_h->SetMarkerSize(0.01);
  hiBinPost_h->SetMarkerStyle(20);
  hiBinPost_h->SetLineColor(kRed);

  hiBinPre_h->DrawCopy("HIST E1");
  hiBinPost_h->DrawCopy("HIST E1 SAME");

  TLegend* leg_p = new TLegend(0.7, 0.8, 0.95, 0.9);
  leg_p->SetBorderSize(0);
  leg_p->SetFillColor(0);
  leg_p->SetFillStyle(0);
  leg_p->SetTextFont(43);
  leg_p->SetTextSize(14);

  leg_p->AddEntry(hiBinPre_h, "Pre-cut", "L");
  leg_p->AddEntry(hiBinPost_h, "Post-cut", "L");

  leg_p->Draw("SAME");


  TLatex* label_p = new TLatex();
  label_p->SetTextFont(43);
  label_p->SetTextSize(12);
  
  const Int_t nBins = 20;
  Double_t bins[nBins+1];
  getLinBins(0, max, nBins, bins);

  std::string cutStr = "Cuts";
  if(optionalTag.size() != 0) cutStr = cutStr + " (" + optionalTag + ")";
  cutStr = cutStr + ":";

  const std::string startString = "Pre-cut events: " + std::to_string(nEntries);
  const std::string endString = "Post-cut events: " + std::to_string(pclustCounter) + " (" + prettyString(pclustCounter*100./(Double_t)nEntries, 2, false) + "%)";

  label_p->DrawLatex(10, bins[19], cutStr.c_str());
  label_p->DrawLatex(10, bins[18], "  |v_{z}| < 15.");
  label_p->DrawLatex(10, bins[17], "  hiHF < 5500 GeV");
  label_p->DrawLatex(10, bins[16], "  HBHENoiseFilterResultRun2Loose"); 
  label_p->DrawLatex(10, bins[15], "  pprimaryVertexFilter");
  label_p->DrawLatex(10, bins[14], "  phfCoincFilter3");
  label_p->DrawLatex(10, bins[13], "  pclusterCompatibilityFilter");
  label_p->DrawLatex(10, bins[12], startString.c_str());
  label_p->DrawLatex(10, bins[11], endString.c_str());
  gStyle->SetOptStat(0);

  std::string saveName = outFileName;
  while(saveName.find("/") != std::string::npos){saveName.replace(0, saveName.find("/")+1, "");}
  if(saveName.find(".root") != std::string::npos) saveName.replace(saveName.find(".root"), saveName.size(), "");
  saveName = "pdfDir/" + dateStr + "/" + saveName + ".pdf";
  
  quietSaveAs(canv_p, saveName);
  delete canv_p;
  delete leg_p;
  delete label_p;

  hiBinPre_h->Write("", TObject::kOverwrite);
  hiBinPost_h->Write("", TObject::kOverwrite);

  delete hiBinPre_h;
  delete hiBinPost_h;

  outFile_p->Close();
  delete outFile_p;

  inFile_p->Close();
  delete inFile_p;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2 && argc != 3){
    std::cout << "Usage: ./bin/checkEventSelection.exe <inFileName> <optionalTag>" << std::endl;
    return 1;
  }

  int retVal = 0;
  if(argc == 2) retVal += checkEventSelection(argv[1]);
  if(argc == 3) retVal += checkEventSelection(argv[1], argv[2]);
  return retVal;
}
