import FWCore.ParameterSet.Config as cms

from DQMOffline.PFTau.PFJetResDQMAnalyzer_cfi import pfJetResDQMAnalyzer

pfJetResValidation1 = pfJetResDQMAnalyzer.clone()
pfJetResValidation1.InputCollection = cms.InputTag('ak5PFJets') # for global Validation
pfJetResValidation1.MatchCollection = cms.InputTag('ak5GenJets') # for global Validatio
pfJetResValidation1.BenchmarkLabel  = cms.string('ElectronValidation/JetPtRes')
pfJetResValidationSequence = cms.Sequence( pfJetResValidation1 )

# NoTracking
pfJetResValidation2 = pfJetResDQMAnalyzer.clone()
pfJetResValidation2.InputCollection = cms.InputTag('ak5PFJets','','PFlowDQMnoTracking')
pfJetResValidation2.MatchCollection = cms.InputTag('ak5GenJets','','PFlowDQMnoTracking')
pfJetResValidation2.BenchmarkLabel  = cms.string('ElectronValidation/JetPtResNoTracking')
pfJetResValidationSequence_NoTracking = cms.Sequence( pfJetResValidation2 )

