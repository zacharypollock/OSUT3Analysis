#include "OSUT3Analysis/AnaTools/interface/CommonUtils.h"

#include "OSUT3Analysis/Collections/plugins/McparticleProducer.h"

McparticleProducer::McparticleProducer (const edm::ParameterSet &cfg) :
  collections_ (cfg.getParameter<edm::ParameterSet> ("collections"))
{
  collection_ = collections_.getParameter<edm::InputTag> ("mcparticles");

  produces<vector<osu::Mcparticle> > (collection_.instance ());
}

McparticleProducer::~McparticleProducer ()
{
}

void
McparticleProducer::produce (edm::Event &event, const edm::EventSetup &setup)
{
  edm::Handle<vector<TYPE(mcparticles)> > collection;
  anatools::getCollection (collection_, collection, event);

  pl_ = auto_ptr<vector<osu::Mcparticle> > (new vector<osu::Mcparticle> ());
  for (const auto &object : *collection)
    {
      const osu::Mcparticle * const mcparticle = new osu::Mcparticle (object);
      pl_->push_back (*mcparticle);
    }

  event.put (pl_, collection_.instance ());
  pl_.reset ();
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(McparticleProducer);