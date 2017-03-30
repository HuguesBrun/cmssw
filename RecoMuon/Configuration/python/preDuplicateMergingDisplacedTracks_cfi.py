import FWCore.ParameterSet.Config as cms
from RecoTracker.FinalTrackSelectors.TrackCollectionMerger_cfi import *
from RecoTracker.FinalTrackSelectors.trackAlgoPriorityOrder_cfi import trackAlgoPriorityOrder

preDuplicateMergingDisplacedTracks = TrackCollectionMerger.clone()
preDuplicateMergingDisplacedTracks.trackProducers = [
    "muonSeededTracksInOut",
    "muonSeededTracksOutInDisplaced",
    ]
preDuplicateMergingDisplacedTracks.inputClassifiers =[
   "muonSeededTracksInOutClassifier",
   "muonSeededTracksOutInDisplacedClassifier"
   ]

preDuplicateMergingDisplacedTracks.foundHitBonus  = 100.0
preDuplicateMergingDisplacedTracks.lostHitPenalty =   1.0

# For Phase1PU70 tracking, take out muonSeededTracksInOut because the
# cut-selector module is technically incompatible with this one. Since
# that configuration is indended only for tracking comparisons (not
# for production), it is not worth of the effort to try to fix the
# situation.
from Configuration.Eras.Modifier_trackingPhase1PU70_cff import trackingPhase1PU70
trackingPhase1PU70.toModify(preDuplicateMergingDisplacedTracks,
    trackProducers = [x for x in preDuplicateMergingDisplacedTracks.trackProducers if x != "muonSeededTracksInOut"],
    inputClassifiers = [x for x in preDuplicateMergingDisplacedTracks.inputClassifiers if x != "muonSeededTracksInOutClassifier"],
)

# Same for Phase2PU140
from Configuration.Eras.Modifier_trackingPhase2PU140_cff import trackingPhase2PU140
from RecoTracker.FinalTrackSelectors.trackListMerger_cfi import trackListMerger as _trackListMerger

#trackingPhase2PU140.toModify(preDuplicateMergingDisplacedTracks,
#    trackProducers = [x for x in preDuplicateMergingDisplacedTracks.trackProducers],# if x != "muonSeededTracksInOut"],
#    inputClassifiers = [x for x in preDuplicateMergingDisplacedTracks.inputClassifiers],# if x != "muonSeededTracksInOutClassifier"],
#)
trackingPhase2PU140.toReplaceWith(preDuplicateMergingDisplacedTracks, _trackListMerger.clone(
    TrackProducers = cms.VInputTag(
        cms.InputTag("muonSeededTracksInOut"),
        cms.InputTag("muonSeededTracksOutInDisplaced"),
    ),
    hasSelector = cms.vint32(1,1),
    selectedTrackQuals = cms.VInputTag(
        cms.InputTag("muonSeededTracksInOutSelector","muonSeededTracksInOutHighPurity"),
        cms.InputTag("muonSeededTracksOutInDisplacedSelector","muonSeededTracksOutInDisplacedLoose"),
    ),
    mvaValueTags = cms.VInputTag(
        cms.InputTag("muonSeededTracksInOutSelector","MVAVals"),
        cms.InputTag("muonSeededTracksOutInDisplacedSelector","MVAVals"),
    ),
    setsToMerge = cms.VPSet(cms.PSet(pQual = cms.bool(False), tLists = cms.vint32(0, 1))),
    FoundHitBonus  = 100.0,
    LostHitPenalty =   1.0,
    indivShareFrac = cms.vdouble(1.0, 0.16, 0.095, 0.09, 0.095,0.095, 0.095, 0.08),
    copyExtras = True,
    makeReKeyedSeeds = cms.untracked.bool(False)
    )
)
