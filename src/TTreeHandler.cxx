#include <TTreeHandler.hpp>
#include "TSystemDirectory.h"
#include "TTree.h"
#include "TChain.h"

using namespace std;

TTreeHandler::TTreeHandler()
    : mTree(nullptr), mTreeName(""), mCounter(0), mNFiles(-1) {}

TTreeHandler::TTreeHandler(string treename, std::vector<string> files)
    : mTree(nullptr), mTreeName(treename), mCounter(0), mNFiles(0) {
  mFiles = files;
  if (!initialize()) {
    printf("TTreeHandler:: initialize:: Initial Failed!\n");
  }
}

bool TTreeHandler::initialize() {
  if (mTree == nullptr)
  delete mTree;
    mTree = new TChain();
  mNFiles = mFiles.size();
  mTree->SetName(mTreeName.c_str());
  // If have both Tree and Files, init the Tree
  if (!(mTreeName == "") && !(mNFiles == 0)) {
    ReParseTree();
  }
  return true;
}

bool TTreeHandler::ReParseTree() {
  if (!(mTreeName == "") && !(mNFiles == 0)) {
    mTree->Reset();
    for (auto fn : mFiles) {
      mTree->Add(fn.c_str());
    }
    mTree->SetDirectory(0);
    VarVec.clear();
    VarTypeMap.clear();
    TObjArray *leaves = mTree->GetListOfLeaves();
    int Nleaves = leaves->GetEntries();
    VarVec.resize(Nleaves);
    for (size_t i = 0; i < Nleaves; i++) {
      TLeaf *leaf = dynamic_cast<TLeaf *>(leaves->At(i));
      string name = leaf->GetName();
      VarVec.push_back(name);
      VarTypeMap[name] = leaf->GetTypeName();
    }
    printf("TTreeHandler:: Totally initialed %i leaves of TTree/TChain %s.\n",
           Nleaves, mTreeName.c_str());
    return true;
  } else {
    printf("TTreeHandler:: ReParseTree:: No Enough Para!\n");
    return false;
  }
}

bool TTreeHandler::GetEntry(long ientry) {
  if (!(mCounter < 0))
    mCounter = ientry;
  int fState = mTree->GetEntry(ientry);
  if (fState > 0) {
    return true;
  } else {
    return false;
  }
}

bool TTreeHandler::Next() {
  bool bState = GetEntry(mCounter);
  if (bState)
    mCounter++;
  return bState;
}

long TTreeHandler::GetEntries() { return mTree->GetEntries(); }
