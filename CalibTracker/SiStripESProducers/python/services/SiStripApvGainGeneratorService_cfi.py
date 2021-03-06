import FWCore.ParameterSet.Config as cms


SiStripApvGainGenerator = cms.Service("SiStripApvGainGenerator",
                                      file      = cms.FileInPath("CalibTracker/SiStripCommon/data/SiStripDetInfo.dat"),
                                      genMode   = cms.string("default"),
                                      MeanGain  =cms.double(1.0),
                                      SigmaGain =cms.double(0.0),
                                      MinPositiveGain=cms.double(0.1),
                                      printDebug = cms.untracked.uint32(5)
                                      )




