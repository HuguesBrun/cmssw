import FWCore.ParameterSet.Config as cms

process = cms.Process('GETGBR')

process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:/tmp/hbrun/theDY_miniAOD.root')
)

from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '80X_mcRun2_asymptotic_2016_TrancheIV_v6', '')

process.getGBR25ns = cms.EDAnalyzer("GBRForestGetterFromDB",
    grbForestName = cms.string("gedelectron_p4combination_25ns"),
                                    regressionKey = cms.vstring("gedelectron_EBCorrection_25ns","gedelectron_EECorrection_25ns"),
                                    uncertaintyKey = cms.vstring("gedelectron_EBUncertainty_25ns","gedelectron_EEUncertainty_25ns"),
                                    combinationKey = cms.string("gedelectron_p4combination_25ns"),
    outputFileName = cms.untracked.string("GBRForest_data_25ns.root"),
)
    
process.getGBR50ns = cms.EDAnalyzer("GBRForestGetterFromDB",
    grbForestName = cms.string("gedelectron_p4combination_50ns"),
    outputFileName = cms.untracked.string("GBRForest_data_50ns.root"),
)

process.path = cms.Path(
    process.getGBR25ns)
