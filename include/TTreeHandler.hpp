/*
   A simple handler to work with a TTree or TChain.
   Basically, using TLeaf to access data members in a TTree,
   Automaticly loop all branches(leaves) in a TTree which are
   numbers or vectors.
 */
// Root Headers
#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TSystem.h"

// Cpp Headers
#include <iostream>
#include <map>
#include <vector>
#include <typeinfo>
#include <string>
#include <memory>

using namespace std;

#ifndef TTREEHANDLER_H_
#define TTREEHANDLER_H_
// declaration of main class
class TTreeHandler {
public:
  // constructer
  TTreeHandler();
  // copy constructer
  //        TTreeHandler(const TTreeHandler &handler);
  // constructor with args
  TTreeHandler(string treename, std::vector<string> files);
  //        TTreeHandler(TChain* tree);
  // destructor
  ~TTreeHandler();

  // member function
  // Parse the tree
  bool ReParseTree();
  // Update new dataset or tree to the handler
  // Remind that either of these two will re-init the entry
  // Remind call ReParseTree() by hand after either of these two
  void SetPara(string TreeName, std::vector<string> files) {
    mTreeName = TreeName;
    mFiles = files;
    this->initialize();
  };

  long GetCurrentEntry() { return mCounter; };
  bool GetEntry(long ientry);
  bool Next();

  long GetEntries();

  // BE VERY CAREFULL MAKE TYPE RIGHT!!!!
  // AT THIS POINT, NO SAFETY CONFORM!!!
  // REQUIRE EXTREMELY THE SAME TYPE!!!
  template <typename T> inline T GetValue(string valName) {
    TLeaf *fLeaf = mTree->GetLeaf(valName.c_str());
    T *vp = (T *)(fLeaf->GetValuePointer());
    return *vp;
  };

  template <typename T> T GetSumUp(string Varname) {
    T sum = 0;
    if (VarTypeMap.find(Varname) == VarTypeMap.end()) {
      printf("TTreeHandler:: GetSumUp:: No Var Named %s Here!\n",
             Varname.c_str());
    } else {
      string type = VarTypeMap.at(Varname);
      if (type.find("vector") != string::npos) {
        printf("TTreeHandler:: GetSumUp:: Var %s is a std::vector!\n",
               Varname.c_str());
      } else {
        long nentries = mTree->GetEntries();
        for (long ientry = 0; ientry < nentries; ientry++) {
          mTree->GetEntry(ientry);
          sum = sum + GetValue<T>(Varname);
        }
      }
    }
    return sum;
  };
  const TChain *GetChain() const { return mTree; };
  void ResetCounter() { mCounter = 0; };
  string GetValueType(string value) {
    if (VarTypeMap.find(value) != VarTypeMap.end())
      return VarTypeMap.at(value);
    else
      return "";
  };

private:
  TTreeHandler(TTreeHandler &T);
  vector<string> VarVec;
  map<string, string> VarTypeMap;
  TChain *mTree;

  vector<string> mFiles;
  string mTreeName;
  long mCounter;
  int mNFiles;

  // init function
  bool initialize();
};

#endif // endif TTREEHANDLER_H_
