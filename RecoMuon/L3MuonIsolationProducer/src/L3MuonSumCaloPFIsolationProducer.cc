#include "L3MuonSumCaloPFIsolationProducer.h"

// Framework
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "DataFormats/Common/interface/Handle.h"

#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Common/interface/AssociationMap.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidateFwd.h"

#include "DataFormats/RecoCandidate/interface/IsoDeposit.h"
#include "DataFormats/RecoCandidate/interface/IsoDepositFwd.h"

#include "RecoMuon/MuonIsolation/interface/Range.h"
#include "DataFormats/RecoCandidate/interface/IsoDepositDirection.h"

#include "PhysicsTools/IsolationAlgos/interface/IsoDepositExtractor.h"
#include "PhysicsTools/IsolationAlgos/interface/IsoDepositExtractorFactory.h"

#include "L3NominalEfficiencyConfigurator.h"

#include <string>

using namespace edm;
using namespace std;
using namespace reco;

/// constructor with config
L3MuonSumCaloPFIsolationProducer::L3MuonSumCaloPFIsolationProducer(const edm::ParameterSet& config) {
    
    recoChargedCandidateProducer_ = consumes<reco::RecoChargedCandidateCollection>(config.getParameter<edm::InputTag>("recoChargedCandidateProducer"));
    pfEcalClusterProducer_         = consumes<reco::RecoChargedCandidateIsolationMap>(config.getParameter<edm::InputTag>("pfEcalClusterProducer"));
    pfHcalClusterProducer_         = consumes<reco::RecoChargedCandidateIsolationMap>(config.getParameter<edm::InputTag>("pfHcalClusterProducer"));
    
    /*drMax_          = config.getParameter<double>("drMax");
     drVetoBarrel_   = config.getParameter<double>("drVetoBarrel");
     drVetoEndcap_   = config.getParameter<double>("drVetoEndcap");
     etaStripBarrel_ = config.getParameter<double>("etaStripBarrel");
     etaStripEndcap_ = config.getParameter<double>("etaStripEndcap");
     energyBarrel_   = config.getParameter<double>("energyBarrel");
     energyEndcap_   = config.getParameter<double>("energyEndcap");
     
     doRhoCorrection_                = config.getParameter<bool>("doRhoCorrection");
     if (doRhoCorrection_)
     rhoProducer_                    = consumes<double>(config.getParameter<edm::InputTag>("rhoProducer"));
     
     rhoMax_                         = config.getParameter<double>("rhoMax");
     rhoScale_                       = config.getParameter<double>("rhoScale");
     effectiveAreaBarrel_            = config.getParameter<double>("effectiveAreaBarrel");
     effectiveAreaEndcap_            = config.getParameter<double>("effectiveAreaEndcap");*/
    
    produces < edm::ValueMap<float> >();
    
}

L3MuonSumCaloPFIsolationProducer::~L3MuonSumCaloPFIsolationProducer()
{}

void L3MuonSumCaloPFIsolationProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("recoChargedCandidateProducer", edm::InputTag("hltL1SeededRecoChargedCandidatePF"));
    desc.add<edm::InputTag>("pfEcalClusterProducer", edm::InputTag("hltParticleFlowClusterECAL"));
    desc.add<edm::InputTag>("pfHcalClusterProducer", edm::InputTag("hltParticleFlowClusterECAL"));
    descriptions.add(("hltL3MuonSumCaloPFIsolationProducer"), desc);
}

void L3MuonSumCaloPFIsolationProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){
    
    edm::Handle<reco::RecoChargedCandidateCollection> recochargedcandHandle;
    iEvent.getByToken(recoChargedCandidateProducer_,recochargedcandHandle);
    
    edm::Handle<reco::RecoChargedCandidateIsolationMap> ecalIsolation;
    iEvent.getByToken (pfEcalClusterProducer_,ecalIsolation);
    
    edm::Handle<reco::RecoChargedCandidateIsolationMap> hcalIsolation;
    iEvent.getByToken (pfHcalClusterProducer_,hcalIsolation);
    
    std::auto_ptr<edm::ValueMap<float> > caloIsoMap( new edm::ValueMap<float> ());
    std::vector<float> isoFloats(recochargedcandHandle->size(), 0);
    
    for (unsigned int iReco = 0; iReco < recochargedcandHandle->size(); iReco++) {
        reco::RecoChargedCandidateRef candRef(recochargedcandHandle, iReco);
        reco::RecoChargedCandidateIsolationMap::const_iterator mapiECAL = (*ecalIsolation).find( candRef );
        float valisoECAL = mapiECAL->val;
        reco::RecoChargedCandidateIsolationMap::const_iterator mapiHCAL = (*hcalIsolation).find( candRef );
        float valisoHCAL = mapiHCAL->val;
        float caloIso = valisoECAL + valisoHCAL;
        isoFloats[iReco] = caloIso;
    }
    
    edm::ValueMap<float> ::Filler isoFloatFiller(*caloIsoMap);
    isoFloatFiller.insert(recochargedcandHandle, isoFloats.begin(), isoFloats.end());
    isoFloatFiller.fill();
    iEvent.put(caloIsoMap);

}
