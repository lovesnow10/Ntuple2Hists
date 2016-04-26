#include "HistStore.hpp"

using namespace std;

HistStore::HistStore(string OutName) {
//  mFile = new TFile(OutName.c_str(), "RECREATE");
//  mDir = (TDirectory *)mFile;
mOutName = OutName;
}
void HistStore::AddHist(string hname, float nBins, float xlow, float xup) {
  TH1F *hist = new TH1F(hname.c_str(), hname.c_str(), nBins, xlow, xup);
  hist->Sumw2();
  hist->SetDirectory(0);
  mHistMap.insert(make_pair(hname, hist));
//  printf("HistStore:: AddHist:: Successfully Add Histogram: %s\n",
//         hname.c_str());
}

void HistStore::AddHist2D(string hname, float nBinsx, float xlow, float xup, float nBinsy, float ylow, float yup)
{
  TH2F *hist = new TH2F(hname.c_str(), hname.c_str(), nBinsx, xlow, xup, nBinsy, ylow, yup);
  hist->SetDirectory(0);
  mHist2DMap.insert(make_pair(hname, hist));
}

TH1F *HistStore::GetHist(string hname) {
  if (mHistMap.find(hname) == mHistMap.end()) {
    printf("HistStore:: GetHist:: Cannot Find Histogram %s\n", hname.c_str());
    return nullptr;
  }
  return mHistMap.at(hname);
}

TH2F *HistStore::GetHist2D(string hname)
{
  if (mHist2DMap.find(hname) == mHist2DMap.end())
  {
    printf("HistStore:: GetHist2D:: Cannot Find Historam %s\n", hname.c_str());
    return nullptr;
  }
  return mHist2DMap.at(hname);
}

string HistStore::GenName(string VarName, string Region, string Sample) {
  string hname = "hist";
  hname = hname + "_" + VarName + "_" + Region + "_" + Sample;
  return hname;
}

void HistStore::SaveAllHists() {
  mFile = new TFile(mOutName.c_str(), "RECREATE");
  mDir = (TDirectory*)mFile;
  for (auto hist : mHistMap) {
    hist.second->SetDirectory(mDir);
    hist.second->Write();
    printf("HistStore:: SaveAll:: Histogram %s Successfully Saved\n",
           hist.first.c_str());
  }
  for (auto hist : mHist2DMap)
  {
    hist.second->SetDirectory(mDir);
    hist.second->Write();
    printf("HistStore:: SaveAll:: Histogram %s Successfully Saved\n", hist.first.c_str());
  }
}
void HistStore::Close() { mFile->Close(); }

bool HistStore::HasHist(string hname)
{
  if(mHistMap.find(hname) == mHistMap.end()) return false;
  else return true;
}
