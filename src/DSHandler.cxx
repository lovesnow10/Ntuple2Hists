#include "DSHandler.hpp"
#include "TString.h"
#include "TCollection.h"
#include "TSystem.h"
#include <fstream>
#include <regex>

using namespace std;

DSHandler::DSHandler(string InputTxt)
{
  ifstream in(InputTxt.c_str());
  if(!in)
  {
    printf("DSHandler:: Cannot Open File %s\n", InputTxt.c_str());
    exit(-1);
  }
  string line;
  while (getline(in, line)) {
    mPathsVec.push_back(line);
    string ID = this->GetSampleID(line);
    if (ID != "")
    {
      string tmpType;
      if (ID.length() == 8)
      {
        tmpType = "DATA";
      }
      else
      {
        tmpType = this->GetSampleType(atoi(ID.c_str()));
      }
      if (find(mSampleTypesVec.begin(), mSampleTypesVec.end(), tmpType) == mSampleTypesVec.end())
      {
        mSampleTypesVec.push_back(tmpType);
      }
    }
  }
  cout<<mSampleTypesVec.size()<<endl;
  this->Initialize();
}

void DSHandler::AddPath(string path) {
  mPathsVec.push_back(path);
//  std::vector<string> tempVec = this->GrabRootFiles(path);
//  mFilesMap.insert(make_pair(mTotalDS++, tempVec));
}

const std::vector<string> &DSHandler::GetFiles(int nDS) const {
  if (nDS < mTotalDS) {
    return mFilesMap.at(nDS);
  } else {
    printf("DSHandler:: GetFiles:: %i is Out-Of-Range!!\n", nDS);
    return tmpNone;
  }
}

string DSHandler::GetSampleType(int DSID) {
  if (DSID == 0) {
    return "DATA";
  }
  if (DSID >= 361300 && DSID <= 361371) {
    return "WJets";
  }
  if (DSID >= 361372 && DSID <= 361491) {
    return "ZJets";
  }
  if (DSID >= 361063 && DSID <= 361086) {
    return "Dibosons";
  }
  if (DSID == 410015 || DSID == 410016) {
    return "SingleTop";
  }
  if (DSID == 410000 || DSID == 410009 || DSID == 410120 || DSID == 410121) {
    return "ttbar";
  }
  if (DSID >= 341365 && DSID <= 343367) {
    return "ttH";
  }
  if (DSID >= 341541 && DSID <= 341558) {
    return "Hplus";
  }
  if (DSID == 410050 || (DSID >= 410066 && DSID <= 410116)) {
    return "Others";
  }
  return "Unknown";
}

bool DSHandler::Initialize() {
  mCurrentDS = 0;
  mFilesMap.clear();
  mTotalDS = mPathsVec.size();
  tmpNone.clear();
  int tmpID = 0;
  for (auto path : mPathsVec) {
    std::vector<string> tmpVec = GrabRootFiles(path);
    mFilesMap.insert(make_pair(tmpID++, tmpVec));
  }
  printf("DSHandler:: Initialize:: Initialization Succeed!\n");
  return true;
}

float DSHandler::GetWeight(int DSID) {
  float weight = 1.0;
  string type = GetSampleType(DSID);
  if (type == "ZJets") {
    int tmpID = DSID - 361371;
    if (DSID <= 361443) {
      int tmpType = tmpID % 3;
      if (tmpType == 1) {
        weight = 0.878;
      } else if (tmpType == 0) {
        weight = 1.135;
      }
    } else {
      int tmpType = tmpID % 2;
      if (tmpType == 1) {
        weight = 0.878;
      } else if (tmpType == 0) {
        weight = 1.135;
      }
    }
  }
  return weight;
}

std::vector<string> DSHandler::GrabRootFiles(string path) {
  TSystemDirectory dir(path.c_str(), path.c_str());
  TList *files = dir.GetListOfFiles();
  std::vector<string> rootfiles;
  if (files) {
    TSystemFile *file;
    TString fname;
    TIter next(files);
    while ((file = (TSystemFile *)next())) {
      fname = file->GetName();
      if (!file->IsDirectory() && fname.Contains(".root")) {
        rootfiles.push_back(path + "/" + fname.Data());
      }
    }
  }
  return rootfiles;
}

void DSHandler::Clear() {
  mPathsVec.clear();
  mFilesMap.clear();
  mTotalDS = 0;
  mCurrentDS = 0;
  printf("DSHandler:: Clear:: Successfully Clear All!\n");
}

void DSHandler::DeletePath(string path) {
  auto _result = find(mPathsVec.begin(), mPathsVec.end(), path);
  if (_result == mPathsVec.end()) {
    printf("DSHandler:: DeletePath:: Cannot Find %s!!\n", path.c_str());
    return;
  }
  mPathsVec.erase(_result);
  this->Initialize();
  printf("DSHandler:: DeletePath:: Successfully Delete %s!\n", path.c_str());
}

const std::vector<string> &DSHandler::Next() {
  if (mCurrentDS < mTotalDS)
    return this->GetFiles(mCurrentDS++);
  else
    return tmpNone;
}

string DSHandler::GetSampleID(string path)
{
  const regex pattern(".*\\.(\\d+)\\..*");
  std::match_results<std::string::const_iterator> result;
  bool valid = std::regex_match(path, result, pattern);
  if (!valid)
  {
    return "";
  }
  else
  {
    return result[1];
  }
}
