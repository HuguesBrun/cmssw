// makes CaloTowerCandidates from CaloTowers
// original author: M. Sani (UCSD)

#include <cmath>
#include "DataFormats/RecoCandidate/interface/RecoCaloTowerCandidate.h"
#include "DataFormats/CaloTowers/interface/CaloTower.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "RecoEgamma/EgammaHLTProducers/interface/EgammaHLTTimeCleanedCaloTowerProducer.h"

#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

// Math
#include "Math/GenVector/VectorUtil.h"
#include <cmath>

EgammaHLTTimeCleanedCaloTowerProducer::EgammaHLTTimeCleanedCaloTowerProducer( const edm::ParameterSet & p ) {
  towers_  = consumes<CaloTowerCollection>(p.getParameter<edm::InputTag> ("towerCollection"));
  timeCut_ = p.getParameter<double> ("TimeCut");

  produces<CaloTowerCollection>();//p.getParameter<std::string>("outputCollection"));
}

void EgammaHLTTimeCleanedCaloTowerProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {

  edm::ParameterSetDescription desc;
  
  desc.add<edm::InputTag>(("towerCollection"), edm::InputTag("hltTowerMakerForAll"));
  //desc.add<std::string>(("outputCollection"), "hltTimeCleanedTowerMakerForAll");
  desc.add<double>(("TimeCut"), 20.0); 
  descriptions.add(("hltTimeCleanedCaloTower"), desc);  
}


  void EgammaHLTTimeCleanedCaloTowerProducer::produce( edm::Event& evt, const edm::EventSetup& ) {

  edm::Handle<CaloTowerCollection> caloTowers;
  evt.getByToken(towers_, caloTowers);

  std::auto_ptr<CaloTowerCollection> cands;

  for (unsigned idx = 0; idx < caloTowers->size(); idx++) {
    const CaloTower* cal = &((*caloTowers) [idx]);
    if (cal->ecalTime() < timeCut_)
      cands->push_back(*cal);
  }

  evt.put(cands);  
}
