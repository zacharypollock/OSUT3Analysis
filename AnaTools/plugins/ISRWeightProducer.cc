#include <vector>

#include "OSUT3Analysis/AnaTools/plugins/ISRWeightProducer.h"

ISRWeightProducer::ISRWeightProducer (const edm::ParameterSet &cfg) :
  EventVariableProducer(cfg),
  pdgIds_     (cfg.getParameter<vector<int> > ("pdgIds")),
  weightFile_ (cfg.getParameter<string> ("weightFile")),
  weightHist_ (cfg.getParameter<vector<string> > ("weightHist")),
  weights_    ({})
{
  mcparticlesToken_ = consumes<vector<TYPE(hardInteractionMcparticles)> > (collections_.getParameter<edm::InputTag> ("hardInteractionMcparticles"));
}

ISRWeightProducer::~ISRWeightProducer() {
  for (const auto &weight : weights_)
    if (weight)
      delete weight;
}

void
ISRWeightProducer::AddVariables (const edm::Event &event) {

#ifndef STOPPPED_PTLS
  if(event.isRealData() || weightFile_.empty () || weightHist_.empty ()) {
    (*eventvariables)["isrPt"] = -1;
    (*eventvariables)["isrWeight"] = 1;
    (*eventvariables)["isrWeightUp"] = 1;
    (*eventvariables)["isrWeightDown"] = 1;
    return;
  }

  edm::Handle<vector<TYPE(hardInteractionMcparticles)> > mcparticles;
  if (!event.getByToken(mcparticlesToken_, mcparticles)) {
    (*eventvariables)["isrPt"] = -1;
    (*eventvariables)["isrWeight"] = 1;
    (*eventvariables)["isrWeightUp"] = 1;
    (*eventvariables)["isrWeightDown"] = 1;
    return;
  }

  if (weights_.empty ())
    {
      TFile * fin = TFile::Open(weightFile_.c_str());
      if(!fin || fin->IsZombie()) {
        clog << "ERROR [ISRWeightProducer]: Could not find file: " << weightFile_
             << "; would cause a seg fault." << endl;
        exit(1);
      }

      for (const auto &weightHist : weightHist_)
        {
          TH1D *w = (TH1D*)fin->Get(weightHist.c_str());
          if(!w) {
            clog << "ERROR [ISRWeightProducer]: Could not find histogram: " << weightHist
                 << "; would cause a seg fault." << endl;
            exit(1);
          }
          w->SetDirectory(0);
          weights_.push_back (w);
        }
      fin->Close ();
      delete fin;
    }

  double px = 0.0;
  double py = 0.0;

  for(const auto &mcparticle : *mcparticles) {
    for(const auto &pdgId : pdgIds_) {
      if(abs(mcparticle.pdgId()) == abs(pdgId) && isOriginalParticle(mcparticle, mcparticle.pdgId())) {
        px += mcparticle.px();
        py += mcparticle.py();
      }
    }
  }

  double pt = sqrt(px*px + py*py);
  double isrWeight = 1.0, isrWeightUp = 1.0, isrWeightDown = 1.0;

  for (const auto &weight : weights_) {
    double content = weight->GetBinContent (min (weight->FindBin (pt), weight->GetNbinsX ()));
    double error = weight->GetBinError (min (weight->FindBin (pt), weight->GetNbinsX ()));

    isrWeight *= content;
    isrWeightUp *= content + error;
    isrWeightDown *= max(content - error, 0.0); // max just in case error > content
  }

  (*eventvariables)["isrPt"] = pt;
  (*eventvariables)["isrWeight"] = isrWeight;
  (*eventvariables)["isrWeightUp"] = isrWeightUp;
  (*eventvariables)["isrWeightDown"] = isrWeightDown;

#else // if STOPPPED_PTLS
  (*eventvariables)["isrPt"] = -1;
  (*eventvariables)["isrWeight"] = 1;
  (*eventvariables)["isrWeightUp"] = 1;
  (*eventvariables)["isrWeightDown"] = 1;
#endif
}

bool
ISRWeightProducer::isOriginalParticle (const TYPE(hardInteractionMcparticles) &mcparticle, const int pdgId) const
{
#ifndef STOPPPED_PTLS
  if(!mcparticle.numberOfMothers () || mcparticle.motherRef ().isNull ()) return true;
  return(mcparticle.motherRef ()->pdgId () != pdgId) && isOriginalParticle ((TYPE(hardInteractionMcparticles)) *mcparticle.motherRef (), pdgId);
#else
  return false;
#endif
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(ISRWeightProducer);
