/*
  A simple class to store histograms(TH1F)
*/

#include <iostream>
#include <string>
#include <vector>
#include <map>

// Root Headers
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "TDirectory.h"

using namespace std;

#ifndef HISTSTORE_HPP_
#define HISTSTORE_HPP_

class HistStore {
private:
  std::map<string, TH1F *> mHistMap;
  std::map<string, TH2F *> mHist2DMap;
  TFile *mFile;
  TDirectory *mDir;
  string mOutName;

public:
  HistStore(string OutName);
  virtual ~HistStore(){};

  void AddHist(string hname, float nBins, float xlow, float xup);
  void AddHist2D(string hname, float nBinsx, float xlow, float xup,
                 float nBinsy, float ylow, float yup);
  TH1F *GetHist(string hname);
  TH2F *GetHist2D(string hname);

  string GenName(string VarName, string Region, string Sample);
  bool HasHist(string hname);

  void SaveAllHists();
  void Close();
};

#endif
