#include "HistsGen.hpp"
#include "ttbbNLO_syst.hpp"
#include "TROOT.h"

using namespace std;

HistsGen::HistsGen(int argc, char **argv) : mArgc(argc), mArgv(argv) {}

bool HistsGen::initialize() {
  if (mArgc < 3) {
    printf("HistGen:: initialize:: Given args %i Are Not Enough!", mArgc);
    return false;
    ;
  }
  config = new ConfigParser(mArgv[1]);
  string OutName = config->GetCommonSetting("OutName");
  if (OutName == "") {
    printf("HistsGen:: initialize:: Using Default OutputName!\n");
    hs = new HistStore("output.root");
  } else {
    hs = new HistStore(OutName);
  }
  ds = new DSHandler(mArgv[2]);
  string XSFile = config->GetCommonSetting("XSFile");
  xshelper = new XSHelper(XSFile);
  //  mWorker = new TTreeHandler();
  //  mHelpWorker = new TTreeHandler();
  BookHists();
  InitYields();
  printf("HistsGen:: initialize:: HistGen Initialization Succeeded!\n");
  return true;
}

bool HistsGen::run() {
  std::vector<string> files = ds->Next();
  while (!files.empty()) {
    //    mWorker->SetPara("nominal", files);
    //    mHelpWorker->SetPara("sumWeights", files);
    mWorker = new TTreeHandler("nominal", files);
    mHelpWorker = new TTreeHandler("sumWeights", files);
    MakeHists();
    files = ds->Next();
  }
  return true;
}

bool HistsGen::finalize() {
  FillYields();
  hs->SaveAllHists();
  hs->Close();
  printf("HistsGen:: finalize:: HistsGen Has Finished Running!\n");
  return true;
}

bool HistsGen::MakeHists() {
  long nTotalEntries = mWorker->GetEntries();
  long messageSlice = long(nTotalEntries / 15);
  if (messageSlice == 0)
    messageSlice = 1;
  // Calculate normalization (same for all events)
  mWorker->GetEntry(0);
  int runNumber = mWorker->GetValue<int>("runNumber");
  int mcChannel = mWorker->GetValue<int>("mcChannelNumber");
  float norm = 1.0;
  float general_weight = ds->GetWeight(mcChannel);
  // Seems that ttbb rw weight is already in ntuple
  ttbbNLO_syst *ttbbRW;
  if (mcChannel != 0) {
    float totalEvents = mHelpWorker->GetSumUp<float>("totalEventsWeighted");
    string SampleType = ds->GetSampleType(mcChannel);
    //float filter_eff = 1.0;
    //if (mcChannel >= 341541 && mcChannel <= 341558)
    //  filter_eff = 0.5;
    float xs = xshelper->GetXS(to_string(mcChannel));
    float lumi = atof((config->GetCommonSetting("Luminosity")).c_str());
    norm = xs * lumi / totalEvents;
    string NormFile = config->GetCommonSetting("NormFile");
    string ShapeFile = config->GetCommonSetting("ShapeFile");
    if (mcChannel == 410000 || mcChannel == 410009 || mcChannel == 410120 ||
        mcChannel == 410121) {
      ttbbRW = new ttbbNLO_syst("410000", NormFile, ShapeFile);
    } else {
      ttbbRW = new ttbbNLO_syst(to_string(mcChannel), NormFile, ShapeFile);
    }
    ttbbRW->Init();
  }
  long nentries = mWorker->GetEntries();
  for (long ientry = 0; ientry < nentries; ientry++) {
    mWorker->GetEntry(ientry);
    long nProcessed = mWorker->GetCurrentEntry() + 1;

    if (nProcessed % messageSlice == 0) {
      printf(
          "HistsGen:: MakeHists:: ------Now Processed %ld of %ld in %i-----\n",
          nProcessed, nTotalEntries, mcChannel != 0 ? mcChannel : runNumber);
    }
    //Fake!
    bool isFake = false;
    string region, sample;
    sample = ds->GetSampleType(mcChannel);
    if (mcChannel == 0) isFake = false;
    if (sample == "Fakes") isFake = true;
    // Calculate weights (those are applied event by event)
    float weights = 1.0;
    // No weights for DATA
    if (mcChannel != 0) {
      float weight_mc = mWorker->GetValue<float>("weight_mc");
      float weight_pileup = mWorker->GetValue<float>("weight_pileup");
      float weight_jvt = mWorker->GetValue<float>("weight_jvt");
      float weight_leptonSF = mWorker->GetValue<float>("weight_leptonSF");
      float weight_bTagSF_77 = mWorker->GetValue<float>("weight_bTagSF_77");
      // Weight in ntuple is not usable
      //      float weight_ttbb_Nominal =
      //          mWorker->GetValue<float>("weight_ttbb_Nominal");
      cout<<"-------------------"<<endl;
      cout<<"weight_mc: "<<weight_mc<<endl;
      cout<<"weight_pileup: "<<weight_pileup<<endl;
      cout<<"weight_jvt: "<<weight_jvt<<endl;
      cout<<"weight_leptonSF: "<<weight_leptonSF<<endl;
      cout<<"weight_bTagSF_77: "<<weight_bTagSF_77<<endl;
      HFSystDataMembers *event = new HFSystDataMembers();
      event->HF_Classification = mWorker->GetValue<int>("HF_Classification");
      event->q1_eta = mWorker->GetValue<float>("q1_eta");
      event->q1_pt = mWorker->GetValue<float>("q1_pt");
      event->qq_dr = mWorker->GetValue<float>("qq_dr");
      event->qq_pt = mWorker->GetValue<float>("qq_pt");
      event->top_pt = mWorker->GetValue<float>("truth_top_pt") * 1e-3;
      event->ttbar_pt = mWorker->GetValue<float>("truth_ttbar_pt") * 1e-3;

      float weight_ttbb_Nominal =
          ttbbRW->GetttHFWeights(event).at("ttbb_Nominal_weight");
      cout<<"weight_ttbb_Nominal: "<<weight_ttbb_Nominal<<endl;
      cout<<"Normalization: "<<norm<<endl;
      cout<<endl;

      weights = weight_mc * weight_pileup * weight_jvt * weight_bTagSF_77 *
                weight_leptonSF * weight_ttbb_Nominal * general_weight;
    }

    // Some selections
    int ee = mWorker->GetValue<int>("ee");
    int mumu = mWorker->GetValue<int>("mumu");
    int emu = mWorker->GetValue<int>("emu");

    if (ee == 0 && mumu == 0 && emu == 0)
      continue;

    int nEl = mWorker->GetValue<int>("nElectrons");
    int nMu = mWorker->GetValue<int>("nMuons");

    // Fake lepton removal
    if (mcChannel != 0 && sample != "Fakes") {
      std::vector<int> el_true_type =
          mWorker->GetValue<std::vector<int>>("el_true_type");
      std::vector<int> el_true_origin =
          mWorker->GetValue<std::vector<int>>("el_true_origin");
      std::vector<int> mu_true_type =
          mWorker->GetValue<std::vector<int>>("mu_true_type");
      std::vector<int> mu_true_origin =
          mWorker->GetValue<std::vector<int>>("mu_true_origin");
      std::vector<int> el_true_originbkg =
          mWorker->GetValue<std::vector<int>>("el_true_originbkg");

      int et1 = -1;
      int et2 = -1;
      int eo1 = -1;
      int eo2 = -1;
      int eb1 = -1;
      int eb2 = -1;

      if (nEl == 0 && nMu == 2) {
        et1 = mu_true_type.at(0);
        et2 = mu_true_type.at(1);
        eo1 = mu_true_origin.at(0);
        eo2 = mu_true_origin.at(1);
        if (!(et1 == 6 && et2 == 6 &&
              (eo1 == 10 || eo1 == 12 || eo1 == 13 || eo1 == 14) &&
              (eo2 == 10 || eo2 == 12 || eo2 == 13 || eo2 == 14)))
          sample = "Fakes";
      } else if (nEl == 2 && nMu == 0) {
        et1 = el_true_type.at(0);
        et2 = el_true_type.at(1);
        eo1 = el_true_origin.at(0);
        eo2 = el_true_origin.at(1);
        eb1 = el_true_originbkg.at(0);
        eb2 = el_true_originbkg.at(1);

        bool pass =
            ((et1 == 2 && (eo1 == 10 || eo1 == 12 || eo1 == 13 || eo1 == 14)) ||
             (et1 == 4 && eo1 == 5 &&
              (eb1 == 10 || eb1 == 12 || eb1 == 13 || eb1 == 14)));
        pass =
            pass &&
            ((et2 == 2 && (eo2 == 10 || eo2 == 12 || eo2 == 13 || eo2 == 14)) ||
             (et2 == 4 && eo2 == 5 &&
              (eb2 == 10 || eb2 == 12 || eb2 == 13 || eb2 == 14)));
        if (!pass)
          sample = "Fakes";
      } else if (nEl == 1 && nMu == 1) {
        et1 = el_true_type.at(0);
        et2 = mu_true_type.at(0);
        eo1 = el_true_origin.at(0);
        eo2 = mu_true_origin.at(0);
        eb1 = el_true_originbkg.at(0);

        bool pass =
            et2 == 6 && (eo2 == 10 || eo2 == 12 || eo2 == 13 || eo2 == 14);
        pass =
            pass &&
            ((et1 == 2 && (eo1 == 10 || eo1 == 12 || eo1 == 13 || eo1 == 14)) ||
             (et1 == 4 && eo1 == 5 &&
              (eb1 == 10 || eb1 == 12 || eb1 == 13 || eb1 == 14)));
        if (!pass)
          sample = "Fakes";
      } else {
        continue;
      }
      // ttbar merge
      int TopHeavyFlavorFilterFlag =
          mWorker->GetValue<int>("TopHeavyFlavorFilterFlag");
      int truth_top_dilep_filter =
          mWorker->GetValue<int>("truth_top_dilep_filter");

      if (mcChannel == 410000 &&
          (TopHeavyFlavorFilterFlag == 5 || truth_top_dilep_filter == 1))
        continue;
      if (mcChannel == 410120 && truth_top_dilep_filter == 1)
        continue;
      if (mcChannel == 410009 && TopHeavyFlavorFilterFlag == 5)
        continue;
    }
    // Fill Hists!
    int nJets = mWorker->GetValue<int>("nJets");
    int nBTags = mWorker->GetValue<int>("nBTags");
    if (sample == "ttbar") {
      int HF_Classification = mWorker->GetValue<int>("HF_Classification");
      if (HF_Classification == 0)
        sample = "ttlight";
      else if (abs(HF_Classification) > 0 && abs(HF_Classification) < 100)
        sample = "ttcc";
      else
        sample = "ttbb";
    }
    if (nJets == 2 && nBTags == 2 && nEl == 2 && nMu == 0) {
      region = "ee2jex2bex";
    } else if (nJets == 2 && nBTags == 2 && nEl == 1 && nMu == 1) {
      region = "emu2jex2bex";
    } else if (nJets == 2 && nBTags == 2 && nEl == 0 && nMu == 2) {
      region = "mumu2jex2bex";
    } else if (nJets == 3 && nBTags == 2 && nEl == 2 && nMu == 0) {
      region = "ee3jex2bex";
    } else if (nJets == 3 && nBTags == 2 && nEl == 1 && nMu == 1) {
      region = "emu3jex2bex";
    } else if (nJets == 3 && nBTags == 2 && nEl == 0 && nMu == 2) {
      region = "mumu3jex2bex";
    } else if (nJets >= 4 && nBTags == 2 && nEl == 2 && nMu == 0) {
      region = "ee4jin2bex";
    } else if (nJets >= 4 && nBTags == 2 && nEl == 1 && nMu == 1) {
      region = "emu4jin2bex";
    } else if (nJets >= 4 && nBTags == 2 && nEl == 0 && nMu == 2) {
      region = "mumu4jin2bex";
    } else if (nJets >= 3 && nBTags >= 3 && nEl == 2 && nMu == 0) {
      region = "ee3jin3bin";
    } else if (nJets >= 3 && nBTags >= 3 && nEl == 1 && nMu == 1) {
      region = "emu3jin3bin";
    } else if (nJets >= 3 && nBTags >= 3 && nEl == 0 && nMu == 2) {
      region = "mumu3jin3bin";
    } else if (nJets >= 2 && nBTags >= 2 && nEl == 2 && nMu == 0) {
      region = "ee2jin2bin";
    } else if (nJets >= 2 && nBTags >= 2 && nEl == 1 && nMu == 1) {
      region = "emu2jin2bin";
    } else if (nJets >= 2 && nBTags >= 2 && nEl == 0 && nMu == 2) {
      region = "mumu2jin2bin";
    } else {
      continue;
    }

    std::vector<string> exRegion = config->GetRegions();
    if (find(exRegion.begin(), exRegion.end(), region) == exRegion.end())
      continue;
    std::vector<string> vars = config->GetRegionVars(region);

    // First deal with vectors stored in Ntuple
    float pT_jet1, pT_jet2, pT_bJet1, pT_lep1, pT_lep2;
    float eta_jet1, eta_jet2, eta_bJet1, eta_lep1, eta_lep2;
    float phi_jet1, phi_jet2, phi_bJet1, phi_lep1, phi_lep2;

    std::vector<float> jet_pt = mWorker->GetValue<std::vector<float>>("jet_pt");
    std::vector<float> jet_eta =
        mWorker->GetValue<std::vector<float>>("jet_eta");
    std::vector<float> jet_phi =
        mWorker->GetValue<std::vector<float>>("jet_phi");
    std::vector<float> jet_mv2c20 =
        mWorker->GetValue<std::vector<float>>("jet_mv2c20");
    std::vector<float> el_pt = mWorker->GetValue<std::vector<float>>("el_pt");
    std::vector<float> el_eta = mWorker->GetValue<std::vector<float>>("el_eta");
    std::vector<float> el_phi = mWorker->GetValue<std::vector<float>>("el_phi");
    std::vector<float> mu_pt = mWorker->GetValue<std::vector<float>>("mu_pt");
    std::vector<float> mu_eta = mWorker->GetValue<std::vector<float>>("mu_eta");
    std::vector<float> mu_phi = mWorker->GetValue<std::vector<float>>("mu_phi");

    pT_jet1 = jet_pt.at(0);
    pT_jet2 = jet_pt.at(1);
    eta_jet1 = jet_eta.at(0);
    eta_jet2 = jet_eta.at(1);
    phi_jet1 = jet_phi.at(0);
    phi_jet2 = jet_phi.at(1);
    for (int i = 0; i < nJets; i++) {
      float mv2c20 = jet_mv2c20.at(i);
      if (mv2c20 > -0.4434) {
        pT_bJet1 = jet_pt.at(i);
        eta_bJet1 = jet_eta.at(i);
        phi_bJet1 = jet_phi.at(i);
        break;
      }
    }
    // Setup leptons
    if (nEl == 2 && nMu == 0) {
      pT_lep1 = el_pt.at(0);
      pT_lep2 = el_pt.at(1);
      eta_lep1 = el_eta.at(0);
      eta_lep2 = el_eta.at(1);
      phi_lep1 = el_phi.at(0);
      phi_lep2 = el_phi.at(1);
    } else if (nEl == 0 && nMu == 2) {
      pT_lep1 = mu_pt.at(0);
      pT_lep2 = mu_pt.at(1);
      eta_lep1 = mu_eta.at(0);
      eta_lep2 = mu_eta.at(1);
      phi_lep1 = mu_phi.at(0);
      phi_lep2 = mu_phi.at(1);
    } else if (nEl == 1 && nMu == 1) {
      float ept = el_pt.at(0);
      float mpt = mu_pt.at(0);
      if (ept > mpt) {
        pT_lep1 = el_pt.at(0);
        pT_lep2 = mu_pt.at(0);
        eta_lep1 = el_eta.at(0);
        eta_lep2 = mu_eta.at(0);
        phi_lep1 = el_phi.at(0);
        phi_lep2 = mu_eta.at(0);
      } else {
        pT_lep1 = mu_pt.at(0);
        pT_lep2 = el_pt.at(0);
        eta_lep1 = mu_eta.at(0);
        eta_lep2 = el_eta.at(0);
        phi_lep1 = mu_phi.at(0);
        phi_lep2 = el_phi.at(0);
      }
    }
    // loop over vars
    string tmpName = region + "_" + sample;
    mRawYields.at(tmpName) += 1;
    mWeightedYields.at(tmpName) += (weights * norm);
    for (auto var : vars) {
      float value;
      if (var == "pT_jet1")
        value = pT_jet1;
      else if (var == "pT_jet2")
        value = pT_jet2;
      else if (var == "eta_jet1")
        value = eta_jet1;
      else if (var == "eta_jet2")
        value = eta_jet2;
      else if (var == "phi_jet1")
        value = phi_jet1;
      else if (var == "phi_jet2")
        value = phi_jet2;
      else if (var == "pT_bJet1")
        value = pT_bJet1;
      else if (var == "eta_bJet1")
        value = eta_bJet1;
      else if (var == "phi_bJet1")
        value = phi_bJet1;
      else if (var == "pT_lep1")
        value = pT_lep1;
      else if (var == "pT_lep2")
        value = pT_lep2;
      else if (var == "eta_lep1")
        value = eta_lep1;
      else if (var == "eta_lep2")
        value = eta_lep2;
      else if (var == "phi_lep1")
        value = phi_lep1;
      else if (var == "phi_lep2")
        value = phi_lep2;
      else {
        string valueType = mWorker->GetValueType(var);
        if (valueType == "int" || valueType == "Int_t") {
          value = mWorker->GetValue<int>(var);
        } else {
          value = mWorker->GetValue<float>(var);
        }
      }
      string hname = hs->GenName(var, region, sample);
      hs->GetHist(hname)->Fill(value, weights * norm);
    }
  }
  return true;
}

void HistsGen::BookHists() {
  std::vector<string> regions = config->GetRegions();
  std::vector<string> samples = ds->GetAllTypes();
  int nRegions = regions.size();
  int nSamples = samples.size();
  if (find(samples.begin(), samples.end(), "ttbar") == samples.end()) {
    hs->AddHist2D("hist_raw_yields", nSamples, 0, nSamples, nRegions, 0,
                  nRegions);
    hs->AddHist2D("hist_weighted_yields", nSamples, 0, nSamples, nRegions, 0,
                  nRegions);
  } else {
    hs->AddHist2D("hist_raw_yields", nSamples + 2, 0, nSamples + 2, nRegions, 0,
                  nRegions);
    hs->AddHist2D("hist_weighted_yields", nSamples + 2, 0, nSamples + 2,
                  nRegions, 0, nRegions);
  }
  int nx = 1, ny = 1;
  for (auto region : regions) {
    hs->GetHist2D("hist_raw_yields")
        ->GetYaxis()
        ->SetBinLabel(ny, region.c_str());
    hs->GetHist2D("hist_weighted_yields")
        ->GetYaxis()
        ->SetBinLabel(ny++, region.c_str());
  }
  for (auto sample : samples) {
    if (sample != "ttbar") {
      hs->GetHist2D("hist_raw_yields")
          ->GetXaxis()
          ->SetBinLabel(nx, sample.c_str());
      hs->GetHist2D("hist_weighted_yields")
          ->GetXaxis()
          ->SetBinLabel(nx++, sample.c_str());
    } else {
      hs->GetHist2D("hist_raw_yields")->GetXaxis()->SetBinLabel(nx, "ttlight");
      hs->GetHist2D("hist_weighted_yields")
          ->GetXaxis()
          ->SetBinLabel(nx++, "ttlight");
      hs->GetHist2D("hist_raw_yields")->GetXaxis()->SetBinLabel(nx, "ttcc");
      hs->GetHist2D("hist_weighted_yields")
          ->GetXaxis()
          ->SetBinLabel(nx++, "ttcc");
      hs->GetHist2D("hist_raw_yields")->GetXaxis()->SetBinLabel(nx, "ttbb");
      hs->GetHist2D("hist_weighted_yields")
          ->GetXaxis()
          ->SetBinLabel(nx++, "ttbb");
    }
    for (auto region : regions) {
      std::vector<string> vars = config->GetRegionVars(region);
      for (auto var : vars) {
        std::vector<float> bins = config->GetVarBinning(region, var);
        if (bins.empty()) {
          continue;
        }
        if (sample != "ttbar") {
          string hname = hs->GenName(var, region, sample);
          hs->AddHist(hname, bins[0], bins[1], bins[2]);
        } else {
          string hname1 = hs->GenName(var, region, "ttlight");
          string hname2 = hs->GenName(var, region, "ttcc");
          string hname3 = hs->GenName(var, region, "ttbb");
          hs->AddHist(hname1, bins[0], bins[1], bins[2]);
          hs->AddHist(hname2, bins[0], bins[1], bins[2]);
          hs->AddHist(hname3, bins[0], bins[1], bins[2]);
        }
      }
    }
  }
}

void HistsGen::FillYields() {
  int nx = hs->GetHist2D("hist_raw_yields")->GetNbinsX();
  int ny = hs->GetHist2D("hist_raw_yields")->GetNbinsY();
  for (int ix = 1; ix < nx + 1; ix++) {
    string xname =
        hs->GetHist2D("hist_raw_yields")->GetXaxis()->GetBinLabel(ix);
    for (int iy = 1; iy < ny + 1; iy++) {
      string yname =
          hs->GetHist2D("hist_raw_yields")->GetYaxis()->GetBinLabel(iy);
      string tmpName = yname + "_" + xname;
      printf("HistsGen:: FillYields:: Filling Yields %s\n", tmpName.c_str());
      float raw, weighted;
      if (mRawYields.find(tmpName) != mRawYields.end()) {
        raw = mRawYields.at(tmpName);
        weighted = mWeightedYields.at(tmpName);
      } else {
        printf("HistsGend:: FillYields:: Cannot Find %s\n", tmpName.c_str());
        raw = 0;
        weighted = 0;
      }
      hs->GetHist2D("hist_raw_yields")->SetBinContent(ix, iy, raw);
      hs->GetHist2D("hist_weighted_yields")->SetBinContent(ix, iy, weighted);
    }
  }
}

void HistsGen::InitYields() {
  std::vector<string> regions = config->GetRegions();
  std::vector<string> samples = ds->GetAllTypes();
  for (auto region : regions) {
    for (auto sample : samples) {
      if (sample != "ttbar") {
        string tmpName = region + "_" + sample;
        mRawYields[tmpName] = 0;
        mWeightedYields[tmpName] = 0;
      } else {
        string tmpName1 = region + "_ttlight";
        string tmpName2 = region + "_ttcc";
        string tmpName3 = region + "_ttbb";
        mRawYields[tmpName1] = 0;
        mWeightedYields[tmpName1] = 0;
        mRawYields[tmpName2] = 0;
        mWeightedYields[tmpName2] = 0;
        mRawYields[tmpName3] = 0;
        mWeightedYields[tmpName3] = 0;
      }
    }
  }
}
