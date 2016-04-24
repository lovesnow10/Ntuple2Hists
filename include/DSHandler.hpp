/*
  A simple DataSets Handler, support some basic funtions
*/

#include <iostream>
#include <string>
#include <map>
#include <vector>

// Root Headers
#include "TSystemDirectory.h"

using namespace std;

#ifndef DSHANDLER_HPP_
#define DSHANDLER_HPP_

class DSHandler {
private:
  std::vector<string> mPathsVec;
  std::map<int, std::vector<string>> mFilesMap;
  std::vector<string> mSampleTypesVec;


  int mTotalDS;
  int mCurrentDS;

  std::vector<string> GrabRootFiles(string path);

  std::vector<string> tmpNone;

  string GetSampleID(string path);

public:
  DSHandler(){};
  DSHandler(string InputTxt);
  virtual ~DSHandler(){};

  const std::vector<string> &GetFiles(int nDS) const;
  float GetWeight(int DSID);

  void AddPath(string path);
  void Clear();
  void DeletePath(string path);
  bool Initialize();

  string GetSampleType(int DSID);
  const std::vector<string> &Next();
  const std::vector<string> GetAllTypes()
  {
    return mSampleTypesVec;
  };
};

#endif
