#include "HLTriggerOffline/Higgs/interface/HLTHiggsHarvesting.h"

HiggsHarvesting::HiggsHarvesting(const edm::ParameterSet& iPSet)
{
    
    analysisName = iPSet.getUntrackedParameter<std::string>("analysisName");
    

}

HiggsHarvesting::~HiggsHarvesting() {}

void HiggsHarvesting::beginJob()
{
    return;
}

void HiggsHarvesting::endJob()
{
    
    dbe = 0;
    dbe = edm::Service<DQMStore>().operator->();
    
    //define type of sources :
    std::vector<std::string> sources(2);
    sources[0] = "gen";
    sources[1] = "rec";
    if (dbe) {
        for(size_t i = 0; i < sources.size(); i++) {
            // monitoring element numerator and denominator histogram
            MonitorElement *meN =
            dbe->get("HLT/Higgs/"+analysisName+"/SummaryPaths_"+analysisName+"_"+sources[i]+"_passingHLT");
            MonitorElement *meD =
            dbe->get("HLT/Higgs/"+analysisName+"/SummaryPaths_"+analysisName+"_"+sources[i]);
        
            if (meN && meD) {
                // get the numerator and denominator histogram
                TH1F *numerator = meN->getTH1F();
                // numerator->Sumw2();
                TH1F *denominator = meD->getTH1F();
                //denominator->Sumw2();
            
                // set the current directory
                dbe->setCurrentFolder("HLT/Higgs/"+analysisName);
            
                // booked the new histogram to contain the results
                TString nameEffHisto = "efficiencySummary_"+sources[i];
                TH1F *efficiencySummary = (TH1F*) numerator->Clone(nameEffHisto);
                MonitorElement *me = dbe->book1D(nameEffHisto, efficiencySummary );
            
                // Calculate the efficiency
                me->getTH1F()->Divide(numerator, denominator, 1., 1., "B");
        
        } else {
            std::cout << "Monitor elements don't exist" << std::endl;
        }
        }
    } else {
        std::cout << "Don't have a valid DQM back end" << std::endl;
    }
    
    return;
}

void HiggsHarvesting::beginRun(const edm::Run& iRun,
                                  const edm::EventSetup& iSetup)
{
    return;
}

void HiggsHarvesting::endRun(const edm::Run& iRun, 
                                const edm::EventSetup& iSetup)
{
    return;
}

void HiggsHarvesting::analyze(const edm::Event& iEvent, 
                                 const edm::EventSetup& iSetup)
{
    return;
}


DEFINE_FWK_MODULE(HiggsHarvesting);