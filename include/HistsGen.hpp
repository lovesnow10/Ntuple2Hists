/*
  A class inherit from AnalyseTemplate, loop over entries and generate hists
*/

//My Headers
#include "AnalyseTemplate.hpp"
#include "HistStore.hpp"
#include "TTreeHandler.hpp"
#include "ttbbNLO_syst.hpp"
#include "ConfigParser.hpp"
#include "XSHelper.hpp"
#include "DSHandler.hpp"


//Common Headers
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>


using namespace std;

#ifndef HISTSGEN_HPP_
#define HISTSGEN_HPP_

class HistsGen {
private:
  TTreeHandler *mWorker;
  TTreeHandler *mHelpWorker;
  HistStore *hs;
  ConfigParser *config;
  XSHelper *xshelper;
  DSHandler *ds;

  int mArgv;
  char** mArgc;

  //Cannot be copied
  HistsGen(HistsGen &hg);
  //functions in initialize()
  void BookHists();
  //functions in run()
  bool MakeHists();
  //functions in finalize()

public:
  HistsGen (int argv, char** argc);
  virtual ~HistsGen () {};

  bool initialize();
  bool run();
  bool finalize();

};

#endif
