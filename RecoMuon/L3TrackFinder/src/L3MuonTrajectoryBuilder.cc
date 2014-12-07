/**
 *  Class: L3MuonTrajectoryBuilder
 *
 *  Description:
 *   Reconstruct muons starting
 *   from a muon track reconstructed
 *   in the standalone muon system (with DT, CSC and RPC
 *   information).
 *   It tries to reconstruct the corresponding
 *   track in the tracker and performs
 *   matching between the reconstructed tracks
 *   in the muon system and the tracker.
 *
 *
 *  Authors :
 *  N. Neumeister            Purdue University
 *  C. Liu                   Purdue University
 *  A. Everett               Purdue University
 *  with contributions from: S. Lacaprara, J. Mumford, P. Traczyk
 *
 **/

#include "RecoMuon/L3TrackFinder/interface/L3MuonTrajectoryBuilder.h"

//----------------
// Constructors --
//----------------
L3MuonTrajectoryBuilder::L3MuonTrajectoryBuilder(const edm::ParameterSet& par,
						 const MuonServiceProxy* service,
						 edm::ConsumesCollector& iC) : GlobalTrajectoryBuilderBase(par, service, iC) {
  theTrajectoryCleaner = new TrajectoryCleanerBySharedHits();    
  theTkCollName = par.getParameter<edm::InputTag>("tkTrajLabel");
  theBeamSpotInputTag = par.getParameter<edm::InputTag>("tkTrajBeamSpot");
  theMaxChi2 = par.getParameter<double>("tkTrajMaxChi2");
  theDXYBeamSpot = par.getParameter<double>("tkTrajMaxDXYBeamSpot");
  theUseVertex = par.getParameter<bool>("tkTrajUseVertex");
  theVertexCollInputTag = par.getParameter<edm::InputTag>("tkTrajVertex");
  theTrackToken = iC.consumes<reco::TrackCollection>(theTkCollName);
}

//--------------
// Destructor --
//--------------
L3MuonTrajectoryBuilder::~L3MuonTrajectoryBuilder() {
  if (theTrajectoryCleaner) delete theTrajectoryCleaner;
}

void L3MuonTrajectoryBuilder::fillDescriptions(edm::ParameterSetDescription& desc) {
   edm::ParameterSetDescription descTRB;
   MuonTrackingRegionBuilder::fillDescriptions(descTRB);
   desc.add("MuonTrackingRegionBuilder",descTRB);
}

//
// Get information from event
//
void L3MuonTrajectoryBuilder::setEvent(const edm::Event& event) {
  const std::string category = "Muon|RecoMuon|L3MuonTrajectoryBuilder|setEvent";
  
  GlobalTrajectoryBuilderBase::setEvent(event);
      
  // get tracker TrackCollection from Event
  event.getByToken(theTrackToken,allTrackerTracks);
  LogDebug(category) 
      << "Found " << allTrackerTracks->size() 
      << " tracker Tracks with label "<< theTkCollName;  

  if( theUseVertex ) {
    // PV
    edm::Handle<reco::VertexCollection> pvHandle;
    if ( pvHandle.isValid() ) {
      vtx = pvHandle->front();
    } 
    else {
      edm::LogInfo(category) << "No Primary Vertex available from EventSetup \n";
    }
  }
  else {
    // BS
    event.getByLabel(theBeamSpotInputTag, beamSpotHandle);
    if( beamSpotHandle.isValid() ) {
      beamSpot = *beamSpotHandle;
    }
    else {
      edm::LogInfo(category) << "No beam spot available from EventSetup \n";
    }
  }
}


//
// reconstruct trajectories
//
MuonCandidate::CandidateContainer L3MuonTrajectoryBuilder::trajectories(const TrackCand& staCandIn) {

  const std::string category = "Muon|RecoMuon|L3MuonTrajectoryBuilder|trajectories";
  
  // cut on muons with low momenta
  if ( (staCandIn).second->pt() < thePtCut || (staCandIn).second->innerMomentum().Rho() < thePtCut || (staCandIn).second->innerMomentum().R() < 2.5 ) return CandidateContainer();
  
  // convert the STA track into a Trajectory if Trajectory not already present
  TrackCand staCand(staCandIn);
  
  std::vector<TrackCand> trackerTracks;
  
  std::vector<TrackCand> regionalTkTracks = makeTkCandCollection(staCand);
  LogDebug(category) << "Found " << regionalTkTracks.size() << " tracks within region of interest";  
  
  // match tracker tracks to muon track
  trackerTracks = trackMatcher()->match(staCand, regionalTkTracks);
  
  LogDebug(category) << "Found " << trackerTracks.size() << " matching tracker tracks within region of interest";
  if ( trackerTracks.empty() ) return CandidateContainer();
  
  // build a combined tracker-muon MuonCandidate
  // turn tkMatchedTracks into MuonCandidates
  LogDebug(category) << "turn tkMatchedTracks into MuonCandidates";
  CandidateContainer tkTrajs;
  for (std::vector<TrackCand>::const_iterator tkt = trackerTracks.begin(); tkt != trackerTracks.end(); tkt++) {
    if ((*tkt).first != 0 && (*tkt).first->isValid()) {
      MuonCandidate* muonCand = new MuonCandidate( 0 ,staCand.second,(*tkt).second, new Trajectory(*(*tkt).first));
      tkTrajs.push_back(muonCand);
    } else {
      MuonCandidate* muonCand = new MuonCandidate( 0 ,staCand.second,(*tkt).second, 0);
      tkTrajs.push_back(muonCand);
    }
  }
    
  if ( tkTrajs.empty() )  {
    LogDebug(category) << "tkTrajs empty";
    return CandidateContainer();
  }
  
  CandidateContainer result = build(staCand, tkTrajs);  
  LogDebug(category) << "Found "<< result.size() << " L3Muons from one L2Cand";

  // free memory
  if ( staCandIn.first == 0) delete staCand.first;

  for( CandidateContainer::const_iterator it = tkTrajs.begin(); it != tkTrajs.end(); ++it) {
    if ( (*it)->trajectory() ) delete (*it)->trajectory();
    if ( (*it)->trackerTrajectory() ) delete (*it)->trackerTrajectory();
    if ( *it ) delete (*it);
  }
  tkTrajs.clear();  

  for ( std::vector<TrackCand>::const_iterator is = regionalTkTracks.begin(); is != regionalTkTracks.end(); ++is) {
    delete (*is).first;   
  }
  
  return result;
}


//
// make a TrackCand collection using tracker Track, Trajectory information
//
std::vector<L3MuonTrajectoryBuilder::TrackCand> L3MuonTrajectoryBuilder::makeTkCandCollection(const TrackCand& staCand) {
  const std::string category = "Muon|RecoMuon|L3MuonTrajectoryBuilder|makeTkCandCollection";
  std::vector<TrackCand> tkCandColl;
  std::vector<TrackCand> tkTrackCands;
  
  for ( unsigned int position = 0; position != allTrackerTracks->size(); ++position ) {
    reco::TrackRef tkTrackRef(allTrackerTracks,position);
    TrackCand tkCand = TrackCand((Trajectory*)(0),tkTrackRef);
    tkCandColl.push_back(tkCand);
  }

  //Loop over TrackCand collection made from allTrackerTracks in previous step
  for(std::vector<TrackCand>::const_iterator tk = tkCandColl.begin(); tk != tkCandColl.end() ; ++tk) {
	  bool canUseL3MTS = false;
	  try{
		  edm::Ref<L3MuonTrajectorySeedCollection> test = (*tk).second->seedRef().castTo<edm::Ref<L3MuonTrajectorySeedCollection> >() ;
		  canUseL3MTS=true;
		  LogDebug(category) << "Converterted TS into L3MTS";

	  }
	  catch(...){
		  LogDebug(category) << "Failed to convert TS into L3MTS: using all tracker tracks for L3 matching";
	  }
    if (canUseL3MTS){
    	edm::Ref<L3MuonTrajectorySeedCollection> l3seedRef = (*tk).second->seedRef().castTo<edm::Ref<L3MuonTrajectorySeedCollection> >() ;
    	reco::TrackRef staTrack = l3seedRef->l2Track();
    	if( staTrack == (staCand.second) ) {
    		// apply a filter (dxy, chi2 cut)
    		double tk_vtx;
    		if( theUseVertex ) tk_vtx = (*tk).second->dxy(vtx.position());
    		else tk_vtx = (*tk).second->dxy(beamSpot.position());
    		if( fabs(tk_vtx) > theDXYBeamSpot || (*tk).second->normalizedChi2() > theMaxChi2 ) continue;
    		tkTrackCands.push_back(*tk);
    	}
    }
    else{
//    	We will try to match all tracker tracks with the muon:
        double tk_vtx;
        if( theUseVertex ) tk_vtx = (*tk).second->dxy(vtx.position());
        else tk_vtx = (*tk).second->dxy(beamSpot.position());
        if( fabs(tk_vtx) > theDXYBeamSpot || (*tk).second->normalizedChi2() > theMaxChi2 ) continue;
        tkTrackCands.push_back(*tk);
    }

  }

  return tkTrackCands;
}

