/**
  \class    TSGForOI
  \brief    Create L3MuonTrajectorySeeds from L2 Muons updated at vertex in an outside in manner
  \author   Benjamin Radburn-Smith
 */

#include "RecoMuon/TrackerSeedGenerator/plugins/TSGForOI.h"

TSGForOI::TSGForOI(const edm::ParameterSet & iConfig) :
			src_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("src"))),
			selector_(iConfig.existsAs<std::string>("cut") ? iConfig.getParameter<std::string>("cut") : "", true),
			numOfMaxSeeds_(iConfig.getParameter<uint32_t>("maxSeeds")),
			numOfLayersToTry_(iConfig.getParameter<int32_t>("layersToTry")),
			numOfHitsToTry_(iConfig.getParameter<int32_t>("hitsToTry")),
			fixedErrorRescaling_(iConfig.getParameter<double>("fixedErrorRescaleFactor")),
			adjustErrorsDyanmically_(iConfig.getParameter<bool>("adjustErrorsDyanmically")),
			trackerPropagatorName_(iConfig.getParameter<std::string>("trackerPropagator")),
			muonPropagatorName_(iConfig.getParameter<std::string>("muonPropagator")),
			measurementTrackerTag_(consumes<MeasurementTrackerEvent>(iConfig.getParameter<edm::InputTag>("MeasurementTrackerEvent"))),
			minEtaForTEC_(iConfig.getParameter<double>("minEtaForTEC")),
			maxEtaForTOB_(iConfig.getParameter<double>("maxEtaForTOB")),
			useHitlessSeeds_(iConfig.getParameter<bool>("UseHitlessSeeds")),
			dummyPlane_(Plane::build(Plane::PositionType(), Plane::RotationType())){
	produces<std::vector<TrajectorySeed> >();
	estimator_ = new Chi2MeasurementEstimator(30.0); //in init
	updator_ = new KFUpdator();
	foundCompatibleDet_=false;
	numSeedsMade=0;
	theCategory = "Muon|RecoMuon|TSGForOI";
}


TSGForOI::~TSGForOI(){
	if(estimator_) delete estimator_;
	if(updator_) delete updator_;
}


void TSGForOI::produce(edm::Event & iEvent, const edm::EventSetup & iSetup) {
	iSetup.get<IdealMagneticFieldRecord>().get(magfield_);
	iSetup.get<TrackingComponentsRecord>().get(trackerPropagatorName_, trackerPropagator_);
	iSetup.get<TrackingComponentsRecord>().get(muonPropagatorName_, muonPropagator_);
	iSetup.get<TrackingComponentsRecord>().get(muonPropagatorName_, muonPropagatorOpposite_);
	iSetup.get<GlobalTrackingGeometryRecord>().get(geometry_);
	edm::Handle<MeasurementTrackerEvent> measurementTracker;		iEvent.getByToken(measurementTrackerTag_, measurementTracker);
	edm::Handle<reco::TrackCollection> l2TrackCol;					iEvent.getByToken(src_, l2TrackCol);

//	This is what is produced:
	std::auto_ptr<std::vector<TrajectorySeed> > result(new std::vector<TrajectorySeed>());

//	Get vector of Detector layers once:
	std::vector<BarrelDetLayer const*> const& tob = measurementTracker->geometricSearchTracker()->tobLayers();
	std::vector<ForwardDetLayer const*> const& tecPositive = measurementTracker->geometricSearchTracker()->posTecLayers();
	std::vector<ForwardDetLayer const*> const& tecNegative = measurementTracker->geometricSearchTracker()->negTecLayers();

//	Get the suitable progators:
	std::unique_ptr<Propagator> pmuon_cloned = SetPropagationDirection(*muonPropagator_,alongMomentum);
	std::unique_ptr<Propagator> pOpposite 	= SetPropagationDirection(*muonPropagatorOpposite_,oppositeToMomentum);
	std::unique_ptr<Propagator> ptracker_cloned = SetPropagationDirection(*trackerPropagator_, alongMomentum);	//XXX: CHECK THIS!!!

//	Loop over the L2's and making seeds for all of them:
//	std::cout << "TSGForOI - debug: number of L2's: " << l2TrackCol->size() << std::endl;
	for (unsigned int l2TrackColIndex(0);l2TrackColIndex!=l2TrackCol->size();++l2TrackColIndex){
		std::cout << "L2 #" << l2TrackColIndex << std::endl;
		const reco::TrackRef l2(l2TrackCol, l2TrackColIndex);
		std::auto_ptr<std::vector<TrajectorySeed> > out(new std::vector<TrajectorySeed>());

		double errorSF_=1.0;
		if (!adjustErrorsDyanmically_) errorSF_ = fixedErrorRescaling_;

		FreeTrajectoryState fts = trajectoryStateTransform::initialFreeState(*l2, magfield_.product());
		dummyPlane_->move(fts.position() - dummyPlane_->position());
		TrajectoryStateOnSurface tsosAtIP = TrajectoryStateOnSurface(fts, *dummyPlane_);

		FreeTrajectoryState notUpdatedFts = trajectoryStateTransform::innerFreeState(*l2,magfield_.product());
		dummyPlane_->move(notUpdatedFts.position() - dummyPlane_->position());
		TrajectoryStateOnSurface tsosAtMuonSystem = TrajectoryStateOnSurface(notUpdatedFts, *dummyPlane_);

    	numSeedsMade=0;
		foundCompatibleDet_=false;
		bool analysedL2 = false;

//		BARREL
		if (fabs(l2->eta()) < maxEtaForTOB_) {
			int found = 0;
			int temp(0);
			for (auto it=tob.rbegin(); it!=tob.rend(); ++it) {	//This goes from outermost to innermost layer
				++temp;
				if (numSeedsMade<numOfMaxSeeds_){
					std::vector< GeometricSearchDet::DetWithState > dets;
					(**it).compatibleDetsV(tsosAtIP, *(pmuon_cloned.get()), *estimator_, dets);
//					See if we need to adjust the Error Scale Factors:
					if (!analysedL2 && adjustErrorsDyanmically_){
						calculateSFFromL2(**it,tsosAtMuonSystem,dets,*(pmuon_cloned.get()),*l2);
						analysedL2=true;
					}
					if (useHitlessSeeds_){
//						Fill first seed from L2 using only state information:
						if (!foundCompatibleDet_ && dets.size()>0){
							PTrajectoryStateOnDet const& PTSOD = trajectoryStateTransform::persistentState(dets.front().second,dets.front().first->geographicalId().rawId());
							TrajectorySeed::recHitContainer rhContainer;
							out->push_back(TrajectorySeed(PTSOD,rhContainer,oppositeToMomentum));
							foundCompatibleDet_=true;
						}
					}
					if (makeSeedsFromHits(**it, tsosAtIP, *out, *(pmuon_cloned.get()), *(ptracker_cloned.get()), *measurementTracker, errorSF_)) {
						if (++found == numOfLayersToTry_) break;	//XXX numOfLayersToTry_ are those with successful hits!
					}
					numSeedsMade=out->size();
				}
			}
		} //TOB

//		ENDCAP+
		if (l2->eta() > minEtaForTEC_) {
			int found = 0;int temp(0);
			for (auto it=tecPositive.rbegin(); it!=tecPositive.rend(); ++it) {
				++temp;

				if (numSeedsMade<numOfMaxSeeds_){
					std::vector< GeometricSearchDet::DetWithState > dets;
					(**it).compatibleDetsV(tsosAtIP, *(pmuon_cloned.get()), *estimator_, dets);
//					See if we need to adjust the Error Scale Factors:
					if (!analysedL2 && adjustErrorsDyanmically_){
						calculateSFFromL2(**it,tsosAtMuonSystem,dets,*(pmuon_cloned.get()),*l2);
						analysedL2=true;
					}
					if (useHitlessSeeds_){
//						Fill first seed from L2 using only state information:
						if (!foundCompatibleDet_ && dets.size()>0){
							PTrajectoryStateOnDet const& PTSOD = trajectoryStateTransform::persistentState(dets.front().second,dets.front().first->geographicalId().rawId());
							TrajectorySeed::recHitContainer rHC;
							out->push_back(TrajectorySeed(PTSOD,rHC,oppositeToMomentum));
							foundCompatibleDet_=true;
						}
					}
					if (makeSeedsFromHits(**it, tsosAtIP, *out, *(pmuon_cloned.get()), *(ptracker_cloned.get()), *measurementTracker, errorSF_)) {
						if (++found == numOfLayersToTry_) break;
					}
					numSeedsMade=out->size();
				}
			}
		} //TEC+

//		ENDCAP-
		if (l2->eta() < -minEtaForTEC_) {
			int found = 0;int temp(0);
			for (auto it=tecNegative.rbegin(); it!=tecNegative.rend(); ++it) {
				++temp;

				if (numSeedsMade<numOfMaxSeeds_){
					std::vector< GeometricSearchDet::DetWithState > dets;
					(**it).compatibleDetsV(tsosAtIP, *(pmuon_cloned.get()), *estimator_, dets);
//					See if we need to adjust the Error Scale Factors:
					if (!analysedL2 && adjustErrorsDyanmically_){
						calculateSFFromL2(**it,tsosAtMuonSystem,dets,*(pmuon_cloned.get()),*l2);
						analysedL2=true;
					}
					if (useHitlessSeeds_){
//						Fill first seed from L2 using only state information:
						if (!foundCompatibleDet_ && dets.size()>0){
							PTrajectoryStateOnDet const& PTSOD = trajectoryStateTransform::persistentState(dets.front().second,dets.front().first->geographicalId().rawId());
							TrajectorySeed::recHitContainer rHC;
							out->push_back(TrajectorySeed(PTSOD,rHC,oppositeToMomentum));
							foundCompatibleDet_=true;
						}
					}
					if (makeSeedsFromHits(**it, tsosAtIP, *out, *(pmuon_cloned.get()), *(ptracker_cloned.get()), *measurementTracker, errorSF_)) {
						if (++found == numOfLayersToTry_) break;
					}
					numSeedsMade=out->size();
				}
			}
		} //TEC-

//		std::cout << "Number of seeds created for this L2: " << out->size() << std::endl;
		for (std::vector<TrajectorySeed>::iterator it=out->begin(); it!=out->end(); ++it){
			result->push_back(*it);
		}

	} //L2Collection
//	std::cout << "TSGForOI::produce result->size(): " << result->size() << std::endl;
	iEvent.put(result);
}


double TSGForOI::calculateSFFromL2(const GeometricSearchDet& layer,
		const TrajectoryStateOnSurface &tsosAtMuonSystem,
		const std::vector< GeometricSearchDet::DetWithState >& dets,
		const Propagator& muon_propagator,
		const reco::Track& track){

//    std::cout << "calculateSFFromL2" << std::endl;
	double theSF=1.0;

//	Overlap blowup:
	if (fabs(track.eta())>minEtaForTEC_ && fabs(track.eta())<maxEtaForTOB_){
		theSF=theSF*2.0;
	}

//	L2 TSOS Comparison blowup:
//	For this particular layer find the DetWithState using a propagation directly from the Muon system of a L2 without updating at vetex:
	std::vector< GeometricSearchDet::DetWithState > detsFromNotUpdated;
	layer.compatibleDetsV(tsosAtMuonSystem, muon_propagator, *estimator_, detsFromNotUpdated);

	const TrajectoryStateOnSurface TSOSUpdated = dets.front().second;

	if (dets.size()>0 && detsFromNotUpdated.size()>0){ //XXX Warning: check to see if on same det
		const TrajectoryStateOnSurface TSOSNU = detsFromNotUpdated.front().second;

		double diffDeltaR = deltaR(TSOSUpdated.globalMomentum(),TSOSNU.globalMomentum());
//		double diffEta = fabs(TSOSUpdated.globalMomentum().eta()-TSOSNU.globalMomentum().eta());
//		double diffPhi = fabs(TSOSUpdated.globalMomentum().phi()-TSOSNU.globalMomentum().phi());
//		double diffPositionX = fabs(TSOSUpdated.globalPosition().x()-TSOSNU.globalPosition().x());
//		double diffPositionY = fabs(TSOSUpdated.globalPosition().y()-TSOSNU.globalPosition().y());
//		double diffPositionZ = fabs(TSOSUpdated.globalPosition().z()-TSOSNU.globalPosition().z());
//		std::cout << "Difference in dR: " << diffDeltaR
//				<< " diffEta: " << diffEta
//				<< " diffPhi: " << diffPhi
//				<< " x diff: " << diffPositionX
//				<< " y diff: " << diffPositionY
//				<< " z diff: " << diffPositionZ
//				<< std::endl;
		if (diffDeltaR>0.03){
			theSF=theSF*5.0;
		}
	}

//    std::cout << "calculateSFFromL2 theSF = " << theSF << std::endl;
	return theSF;
}


int TSGForOI::makeSeedsFromHits(const GeometricSearchDet &layer,
		const TrajectoryStateOnSurface &tsosAtIP,
		std::vector<TrajectorySeed> &out,
		const Propagator & muon_propagator,
		const Propagator & tracker_propagator,
		const MeasurementTrackerEvent &measurementTracker,
		const double errorSF) {

	std::vector< GeometricSearchDet::DetWithState > dets;
	layer.compatibleDetsV(tsosAtIP, muon_propagator, *estimator_, dets);
//	std::cout << "TSGForOI::makeSeedsFromHits: num of compatibleDetsV: " << dets.size() << std::endl;
//	Find Measurements on each DetWithState:
	std::vector<TrajectoryMeasurement> meas;
	for (std::vector<GeometricSearchDet::DetWithState>::iterator it=dets.begin(); it!=dets.end(); ++it) {
		MeasurementDetWithData det = measurementTracker.idToDet(it->first->geographicalId());
		if (det.isNull()) {
			edm::LogError(theCategory) << "In makeSeedsFromHits: det.isNull() " << it->first->geographicalId().rawId();
			continue;
		}
		if (!it->second.isValid()) continue;	//Skip if TSOS is not valid

//		Error Rescaling:
		it->second.rescaleError(errorSF);

		std::vector < TrajectoryMeasurement > mymeas = det.fastMeasurements(it->second, tsosAtIP, tracker_propagator, *estimator_);	//Second TSOS is not used
		for (std::vector<TrajectoryMeasurement>::const_iterator it2 = mymeas.begin(), ed2 = mymeas.end(); it2 != ed2; ++it2) {
			if (it2->recHit()->isValid()) meas.push_back(*it2);	//Only save those which are valid
		}
	}

//	Update TSOS using TMs after sorting, then create Trajectory Seed and put into vector:
	int found = 0;
	std::sort(meas.begin(), meas.end(), TrajMeasLessEstim());
	for (std::vector<TrajectoryMeasurement>::const_iterator it=meas.begin(); it!=meas.end(); ++it) {
		TrajectoryStateOnSurface updatedTSOS = updator_->update(it->forwardPredictedState(), *it->recHit());
		if (updatedTSOS.isValid()) {
			edm::OwnVector<TrackingRecHit> seedHits;
			seedHits.push_back(*it->recHit()->hit());
			PTrajectoryStateOnDet const& pstate = trajectoryStateTransform::persistentState(updatedTSOS, it->recHit()->geographicalId().rawId());
			TrajectorySeed seed(pstate, std::move(seedHits), oppositeToMomentum);
			out.push_back(seed);
			found++;
			if (found == numOfHitsToTry_) break;
		}
	}
	return found;
}


DEFINE_FWK_MODULE(TSGForOI);
