#include "ConfigParser.hpp"
#include <sstream>

ConfigParser::ConfigParser(string configfile) : mConfigFileName(configfile) {
  if (!initialize()) {
    std::cout << "Cannot Parse the File!" << std::endl;
    exit(-1);
  }
  std::cout << "ConfigParser:: "
            << "Initialization Successed!" << std::endl;
}

string ConfigParser::FixString(string s) {
  string ss = s.substr(0, s.find_first_of('#'));
  replace(ss.begin(), ss.end(), '"', ' ');
  if (ss == "")
    return ss;
  if (ss.find_first_not_of(' ')) {
    ss = ss.substr(ss.find_first_not_of(' '), ss.find_last_not_of(' '));
  } else {
    ss = ss.substr(ss.find_first_not_of(' '), ss.find_last_not_of(' ') + 1);
  }
  return ss;
}

bool ConfigParser::initialize() {
  ifstream in(mConfigFileName.c_str());
  if (!in) {
    std::cout << "ConfigParser:: initialize:: "
              << "Cannot Open the File!" << std::endl;
    return false;
  }
  string line;
  string CurrentRegion;
  while (getline(in, line)) {
    line = FixString(line);
    if (line == "")
      continue;
    stringstream ss(line);
    string s1, s2, s3, s4, s5, s6;
    ss >> s1 >> s2 >> s3 >> s4 >> s5 >> s6;
    // Deal with COMMON
    if (s1 == "COMMON" && s2 != "") {
      mCommonSettingMap[s2] = s3;
      continue;
    } else if (s1 == "COMMON" && s2 == "") {
      std::cout << "ConfigParser:: initialize:: "
                << "Find COMMON But without Any Setting" << std::endl;
      return false;
    }
    // Deal with REGION
    if (s1 == "REGION" && s2 != "") {
      mRegions.push_back(s2);
      CurrentRegion = s2;
      continue;
    } else if (s1 == "REGION" && s2 != "") {
      std::cout << "ConfigParser:: initialize:: "
                << "Find Region without Name!" << std::endl;
      return false;
    }
    // Deal with Vars
    if (s1 == "VAR" && s5 == "") {
      std::cout << "ConfigParser:: initialize:: "
                << "Find VAR with Lack of Args" << std::endl;
      return false;
    } else if (s1 == "VAR") {
      mRegionVarMap[CurrentRegion].push_back(s2);
      std::vector<float> bins;
      bins.push_back(atof(s3.c_str()));
      bins.push_back(atof(s4.c_str()));
      bins.push_back(atof(s5.c_str()));
      mRegionVarsMap[CurrentRegion][s2] = bins;
      continue;
    }
    // Deal with CUT
    if (s1 == "CUT" && s2 != "") {
      mRegionCutMap[CurrentRegion] = s2;
      continue;
    } else {
      std::cout << "ConfigParser:: initialize:: "
                << "Find CUT without Content!" << std::endl;
      return false;
    }
  }
  return true;
}

std::vector<string> ConfigParser::GetRegions() { return mRegions; }

string ConfigParser::GetRegionCut(string RegionName) {
  if (mRegionCutMap.find(RegionName) != mRegionCutMap.end()) {
    return mRegionCutMap.at(RegionName);
  } else {
    printf("ConfigParser:: GetRegions:: Cannot find REGION %s!!\n",
           RegionName.c_str());
    return "";
  }
}

std::vector<float> ConfigParser::GetVarBinning(std::string RegionName,
                                               std::string VarName) {
  std::vector<float> bins;
  bins.clear();
  if (mRegionVarsMap.find(RegionName) != mRegionVarsMap.end()) {
    std::map<string, std::vector<float>> var = mRegionVarsMap.at(RegionName);
    if (var.find(VarName) != var.end()) {
      bins = var.at(VarName);
    } else {
      printf(
          "ConfigParser:: GetVarBinning:: Cannot Find VAR %s in REGION %s!!\n",
          VarName.c_str(), RegionName.c_str());
    }
  } else {
    printf("ConfigParser:: GetVarBinning:: Cannot find REGION %s!!\n",
           RegionName.c_str());
  }
  return bins;
}

string ConfigParser::GetCommonSetting(string settingname) {
  if (mCommonSettingMap.find(settingname) != mCommonSettingMap.end()) {
    return mCommonSettingMap.at(settingname);
  } else {
    printf("ConfigParser:: GetCommonSetting:: Cannot find COMMON %s\n",
           settingname.c_str());
    return "";
  }
}

std::vector<string> ConfigParser::GetRegionVars(string RegionName)
{
  std::vector<string> var;
  if (mRegionVarMap.find(RegionName) != mRegionVarMap.end())
  {
    var =  mRegionVarMap.at(RegionName);
  }
  else
  {
    printf("ConfigParser:: GetRegionVars:: Connot find REGION %s\n", RegionName.c_str());
  }
  return var;
}
