/*
  A simple class to store histograms(TH1F)
*/

#include <iostream>
#include <string>
#include <vector>
#include <map>

// Root Headers
#include "TH1F.h"
#include "TFile.h"
#include "TDirectory.h"

using namespace std;

#ifndef HISTSTORE_HPP_
#define HISTSTORE_HPP_

class HistStore {
private:
  std::map<string, TH1F *> mHistMap;
  TFile *mFile;
  TDirectory *mDir;

public:
  HistStore(string OutName);
  virtual ~HistStore(){};

  void AddHist(string hname, float nBins, float xlow, float xup);
  TH1F *GetHist(string hname);

  string GenName(string VarName, string Region, string Sample);
  bool HasHist(string hname);

  void SaveAllHists();
  void Close();
};

#endif
