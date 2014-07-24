#ifndef EgammaHLTTimeCleanedCaloTowerProducer_h
#define EgammaHLTTimeCleanedCaloTowerProducer_h

/** \class EgammaHLTCaloTowerProducer
 *
 * Framework module that produces a collection
 * of calo towers in the region of interest for Egamma HLT reconnstruction,
 * \author M. Sani (UCSD)
 *
 */

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include <string>

namespace edm {
  class ConfigurationDescriptions;
}


class EgammaHLTTimeCleanedCaloTowerProducer : public edm::EDProducer {
 public:

  EgammaHLTTimeCleanedCaloTowerProducer( const edm::ParameterSet & );
  ~EgammaHLTTimeCleanedCaloTowerProducer() {};
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

 private:
  void produce( edm::Event& e, const edm::EventSetup& ) override;

  edm::EDGetTokenT<CaloTowerCollection> towers_;
  double timeCut_;
};

#endif
