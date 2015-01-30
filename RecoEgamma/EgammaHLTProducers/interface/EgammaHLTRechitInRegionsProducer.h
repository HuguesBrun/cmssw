#ifndef RecoEgamma_EgammayHLTProducers_EgammaHLTRechitInRegionsProducer_h_
#define RecoEgamma_EgammayHLTProducers_EgammaHLTRechitInRegionsProducer_h_

#include <memory>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"


// Geometry
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloTopology/interface/EcalBarrelTopology.h"
#include "Geometry/CaloTopology/interface/EcalEndcapTopology.h"
#include "Geometry/CaloTopology/interface/EcalPreshowerTopology.h"

// Level 1 Trigger
#include "DataFormats/L1Trigger/interface/L1EmParticle.h"
#include "DataFormats/L1Trigger/interface/L1EmParticleFwd.h"
#include "CondFormats/L1TObjects/interface/L1CaloGeometry.h"
#include "CondFormats/DataRecord/interface/L1CaloGeometryRecord.h"

//#include "RecoEcal/EgammaClusterAlgos/interface/HybridClusterAlgo.h"
//#include "RecoEcal/EgammaCoreTools/interface/PositionCalc.h"

template<typename T1>
class HLTRechitInRegionsProducer : public edm::EDProducer {
 typedef std::vector<T1> T1Collection;
 typedef typename T1::const_iterator T1iterator;
  
 public:
  
  HLTRechitInRegionsProducer(const edm::ParameterSet& ps);
  ~HLTRechitInRegionsProducer();

  virtual void produce(edm::Event&, const edm::EventSetup&);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

 private:
    
  void getEtaPhiRegions(std::vector<EcalEtaPhiRegion> *, T1Collection, const L1CaloGeometry&);
    
  bool useUncalib_;
  bool doIsolated_;
  edm::InputTag hitproducer_;
  std::string hitcollection_;
  
  edm::InputTag l1TagIsolated_;
  edm::InputTag l1TagNonIsolated_;
  
  double l1LowerThr_;
  double l1UpperThr_;
  double l1LowerThrIgnoreIsolation_;
  
  double regionEtaMargin_;
  double regionPhiMargin_;
  
  std::vector<edm::InputTag> hitLabels;
  std::vector<std::string> productLabels;
  std::vector<edm::EDGetTokenT<EcalRecHitCollection>> hitTokens;
  std::vector<edm::EDGetTokenT<EcalUncalibratedRecHitCollection>> uncalibHitTokens;
};


#endif


