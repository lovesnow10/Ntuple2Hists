/*
  A template class that need to be inherited by real Analysis class
*/

#include <iostream>
#include <memory>
#include "TTreeHandler.hpp"

using namespace std;

#ifndef ANALYSETEMPLATE_HPP_
#define ANALYSETEMPLATE_HPP_

class AnalyseTemplate {
protected:
  TTreeHandler *mWorker;
public:
  AnalyseTemplate (shared_ptr<TTreeHandler> handler) : mWorker(handler.get()) {};
  virtual ~AnalyseTemplate () {};

  virtual bool initialize() = 0;
  virtual bool run() = 0;
  virtual bool finalize() = 0;

};

#endif
