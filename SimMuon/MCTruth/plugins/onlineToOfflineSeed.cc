// -*- C++ -*-
//
// Package:    onlineToOfflineSeed
// Class:      onlineToOfflineSeed
// 
/**\class onlineToOfflineSeed onlineToOfflineSeed.cc hugues/onlineToOfflineSeed/plugins/onlineToOfflineSeed.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Hugues Brun
//         Created:  Tue, 05 Nov 2013 13:42:04 GMT
// $Id$
//
//




#include "onlineToOfflineSeed.h"

//
// class declaration
//



//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
onlineToOfflineSeed::onlineToOfflineSeed(const edm::ParameterSet& iConfig)
{
    
    L2seedsTag_ =  iConfig.getParameter<edm::InputTag>("L2seedsCollection");

    produces<TrajectorySeedCollection>(); 

    
  
}


onlineToOfflineSeed::~onlineToOfflineSeed()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
onlineToOfflineSeed::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    using namespace edm;
    using namespace std;

    std::auto_ptr<TrajectorySeedCollection> seedsCollection(new TrajectorySeedCollection);

    // now read the L2 seeds collection :
    edm::Handle<L2MuonTrajectorySeedCollection> L2seedsCollection;
    iEvent.getByLabel(L2seedsTag_, L2seedsCollection);
    const std::vector<L2MuonTrajectorySeed>* L2seeds = 0;
    if (L2seedsCollection.isValid()) L2seeds = L2seedsCollection.product();
    else cout << "L2 seeds collection not found !! " << endl;
        

    if (L2seedsCollection.isValid()){
    
    	for (unsigned int i = 0; i < L2seeds->size() ; i++){
                TrajectorySeed theSeed = (TrajectorySeed) L2seeds->at(i);
		seedsCollection->push_back(theSeed);
    	}
    }
    
    
   iEvent.put(seedsCollection);
}

// ------------ method called once each job just before starting event loop  ------------
void 
onlineToOfflineSeed::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
onlineToOfflineSeed::endJob() {
}

// ------------ method called when starting to processes a run  ------------
/*
void
onlineToOfflineSeed::beginRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a run  ------------
/*
void
onlineToOfflineSeed::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
void
onlineToOfflineSeed::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
onlineToOfflineSeed::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
onlineToOfflineSeed::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(onlineToOfflineSeed);
