/**
  \class    TSGForOI
  \brief    Create L3MuonTrajectorySeeds from L2 Muons updated at vertex in an outside in manner
  \author   Benjamin Radburn-Smith
 */

#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"
#include "TrackingTools/MeasurementDet/interface/MeasurementDet.h"
#include "TrackingTools/PatternTools/interface/TrajectoryStateUpdator.h"
#include "TrackingTools/PatternTools/interface/TrajectoryMeasurement.h"
#include "TrackingTools/PatternTools/interface/TrajMeasLessEstim.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "TrackingTools/TrackRefitter/interface/TrackTransformer.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"

#include "RecoTracker/MeasurementDet/interface/MeasurementTracker.h"
#include "RecoTracker/MeasurementDet/interface/MeasurementTrackerEvent.h"
#include "RecoTracker/TkDetLayers/interface/GeometricSearchTracker.h"

#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"
#include "TrackingTools/KalmanUpdators/interface/Chi2MeasurementEstimator.h"
#include "DataFormats/MuonSeed/interface/L3MuonTrajectorySeedCollection.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/Math/interface/deltaR.h"


class TSGForOI : public edm::stream::EDProducer<> {
public:
	explicit TSGForOI(const edm::ParameterSet & iConfig);
	virtual ~TSGForOI();
	virtual void produce(edm::Event & iEvent, const edm::EventSetup & iSetup) override;
private:
	/// Labels for input collections
	edm::EDGetTokenT<reco::TrackCollection> src_;

	/// L2 selection
	StringCutObjectSelector<reco::Track> selector_;

	/// Maximum number of seeds for each L2
	unsigned int numOfMaxSeeds_;

	/// How many layers to try
	int numOfLayersToTry_;

	/// How many hits to try on same layer
	int numOfHitsToTry_;

	/// How much to rescale errors from STA for fixed option
	double fixedErrorRescaling_;

	/// Whether or not to use an automatically calculated SF value
	bool adjustErrorsDyanmically_;

	std::string trackerPropagatorName_;
	std::string muonPropagatorName_;
	edm::EDGetTokenT<MeasurementTrackerEvent> measurementTrackerTag_;
	std::string measurementTrackerName_;
	std::string estimatorName_;

	double minEtaForTEC_, maxEtaForTOB_;

	/// Switch to use hitless seeds or not
	bool useHitlessSeeds_;

	bool foundCompatibleDet_;

	unsigned int numSeedsMade;

	edm::ESHandle<MagneticField>          magfield_;
	edm::ESHandle<Propagator>             muonPropagator_;
	edm::ESHandle<Propagator>             muonPropagatorOpposite_;
	edm::ESHandle<Propagator>             trackerPropagator_;
	edm::ESHandle<GlobalTrackingGeometry> geometry_;

	/// Surface used to make a TSOS at the PCA to the beamline
	Plane::PlanePointer dummyPlane_;

	/// Function used to calculate the dynamic error SF by analysing the L2
	double calculateSFFromL2(const GeometricSearchDet& layer,
			const TrajectoryStateOnSurface &tsosAtMuonSystem,
			const std::vector<GeometricSearchDet::DetWithState>& dets,
			const Propagator& muon_propagator,
			const reco::Track& track);

	/// Function to find hits on layers and create seeds from updated TSOS
	int makeSeedsFromHits(const GeometricSearchDet &layer,
			const TrajectoryStateOnSurface &state,
			std::vector<TrajectorySeed> &out,
			const Propagator &muon_propagator,
			const Propagator &tracker_propagator,
			const MeasurementTrackerEvent &mte,
			double errorSF);

	const Chi2MeasurementEstimator* estimator_;
	const TrajectoryStateUpdator* updator_;
	std::string theCategory;
};
