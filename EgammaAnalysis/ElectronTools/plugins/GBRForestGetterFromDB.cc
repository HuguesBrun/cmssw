#ifndef CalibratedElectronProducer_h
#define CalibratedElectronProducer_h

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CondFormats/DataRecord/interface/GBRDWrapperRcd.h"
#include "CondFormats/EgammaObjects/interface/GBRForestD.h"

#include "CondFormats/DataRecord/interface/GBRWrapperRcd.h"
#include "CondFormats/EgammaObjects/interface/GBRForest.h"
#include <TFile.h>


class GBRForestGetterFromDB: public edm::one::EDAnalyzer<>
{
    public:
        explicit GBRForestGetterFromDB( const edm::ParameterSet & ) ;
        virtual ~GBRForestGetterFromDB() ;
        virtual void analyze( const edm::Event &, const edm::EventSetup & ) override ;

    private:
        std::string theGBRForestName;
    std::vector<std::string> theRegressionKey;
    std::vector<std::string> theUncertaintyKey;
    std::string theCombinationKey;
        std::string theOutputFileName;
        std::string theOutputObjectName;
    edm::ESHandle<GBRForestD> theGBRForestDHandle;
    edm::ESHandle<GBRForest> theGBRForestHandle;
};

GBRForestGetterFromDB::GBRForestGetterFromDB( const edm::ParameterSet & conf ) :
theGBRForestName(conf.getParameter<std::string>("grbForestName")),
theRegressionKey(conf.getParameter<std::vector<std::string>>("regressionKey")),
theUncertaintyKey(conf.getParameter<std::vector<std::string>>("uncertaintyKey")),
theCombinationKey(conf.getParameter<std::string>("combinationKey")),
    theOutputFileName(conf.getUntrackedParameter<std::string>("outputFileName")),
    theOutputObjectName(conf.getUntrackedParameter<std::string>("outputObjectName", theGBRForestName.empty() ? "GBRForest" : theGBRForestName))
{
}

GBRForestGetterFromDB::~GBRForestGetterFromDB()
{
}

void
GBRForestGetterFromDB::analyze( const edm::Event & iEvent, const edm::EventSetup & iSetup ) 
{
    TString theName = theOutputObjectName.c_str();
    TFile *fOut = TFile::Open(theOutputFileName.c_str(), "RECREATE");
    
    iSetup.get<GBRWrapperRcd>().get(theCombinationKey, theGBRForestHandle);
    fOut->WriteObject(theGBRForestHandle.product(), "combination_"+theName);

    iSetup.get<GBRDWrapperRcd>().get(theRegressionKey.at(0), theGBRForestDHandle);
    fOut->WriteObject(theGBRForestHandle.product(), "correction_EB_"+theName);
    iSetup.get<GBRDWrapperRcd>().get(theUncertaintyKey.at(0), theGBRForestDHandle);
    fOut->WriteObject(theGBRForestHandle.product(), "uncertainty_EB_"+theName);
    
    iSetup.get<GBRDWrapperRcd>().get(theRegressionKey.at(1), theGBRForestDHandle);
    fOut->WriteObject(theGBRForestHandle.product(), "correction_EE_"+theName);
    iSetup.get<GBRDWrapperRcd>().get(theUncertaintyKey.at(1), theGBRForestDHandle);
    fOut->WriteObject(theGBRForestHandle.product(), "uncertainty_EE_"+theName);
    
    fOut->Close();
    std::cout << "Wrote output to " << theOutputFileName << std::endl;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GBRForestGetterFromDB);

#endif
