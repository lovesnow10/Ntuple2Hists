#include "HistStore.hpp"

using namespace std;

HistStore::HistStore(string OutName) {
  mFile = new TFile(OutName.c_str(), "RECREATE");
  mDir = (TDirectory *)mFile;
}
void HistStore::AddHist(string hname, float nBins, float xlow, float xup) {
  TH1F *hist = new TH1F(hname.c_str(), hname.c_str(), nBins, xlow, xup);
  hist->Sumw2();
  hist->SetDirectory(mDir);
  mHistMap.insert(make_pair(hname, hist));
  printf("HistStore:: AddHist:: Successfully Add Histogram: %s\n",
         hname.c_str());
}

TH1F *HistStore::GetHist(string hname) {
  if (mHistMap.find(hname) == mHistMap.end()) {
    printf("HistStore:: GetHist:: Cannot Find Histogram %s\n", hname.c_str());
    return nullptr;
  }
  return mHistMap.at(hname);
}

string HistStore::GenName(string VarName, string Region, string Sample) {
  string hname = "hist";
  hname = hname + "_" + VarName + "_" + Region + "_" + Sample;
  return hname;
}

void HistStore::SaveAllHists() {
  for (auto hist : mHistMap) {
    hist.second->Write();
    printf("HistStore:: SaveAll:: Histogram %s Successfully Saved\n",
           hist.first.c_str());
  }
}
void HistStore::Close() { mFile->Close(); }

bool HistStore::HasHist(string hname)
{
  if(mHistMap.find(hname) == mHistMap.end()) return false;
  else return true;
}
