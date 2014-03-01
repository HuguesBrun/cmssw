import FWCore.ParameterSet.Config as cms

onlineToOfflineSeed = cms.EDProducer('onlineToOfflineSeed',
                                          L2seedsCollection = cms.InputTag("hltL2MuonSeeds")
                                          )
