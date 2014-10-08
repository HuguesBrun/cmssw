#include "PhysicsTools/SelectorUtils/interface/CutApplicatorWithEventContentBase.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"

class GsfEleMVATrigCut : public CutApplicatorWithEventContentBase {
public:
    GsfEleMVATrigCut(const edm::ParameterSet& c);
    
    result_type operator()(const reco::GsfElectronRef&) const override final;
    
    void setConsumes(edm::ConsumesCollector&) override final;
    void getEventContent(const edm::EventBase&) override final;
    
    CandidateType candidateType() const override final {
        return ELECTRON;
    }
    
private:
    float _mvaTrigValueCutValueEB;
    float _mvaTrigValueCutValueEE;
    float _barrelCutOff;
    edm::Handle<edm::ValueMap<float> > _mvaTrigValueMap;
};

DEFINE_EDM_PLUGIN(CutApplicatorFactory,
                  GsfEleMVATrigCut,
                  "GsfEleMVATrigCut");

GsfEleMVATrigCut::GsfEleMVATrigCut(const edm::ParameterSet& c) :
CutApplicatorWithEventContentBase(c),
_mvaTrigValueCutValueEB(c.getParameter<double>("mvaTrigValueCutValueEB")),
_mvaTrigValueCutValueEE(c.getParameter<double>("mvaTrigValueCutValueEE")),
_barrelCutOff(c.getParameter<double>("barrelCutOff")) {
    
    edm::InputTag maptag = c.getParameter<edm::InputTag>("mvaTrigValueMap");
    contentTags_.emplace("mvaTrigValue",maptag);
}

void GsfEleMVATrigCut::setConsumes(edm::ConsumesCollector& cc) {
    auto mvaTrigValue =
    cc.consumes<edm::ValueMap<float> >(contentTags_["mvaTrigValue"]);
    contentTokens_.emplace("mvaTrigValue",mvaTrigValue);
}

void GsfEleMVATrigCut::getEventContent(const edm::EventBase& ev) {
    ev.getByLabel(contentTags_["mvaTrigValue"],_mvaTrigValueMap);
}

CutApplicatorBase::result_type
GsfEleMVATrigCut::
operator()(const reco::GsfElectronRef& cand) const{
    
    // Figure out the cut value
    const float mvaTrigCutValue =
    ( std::abs(cand->superCluster()->position().eta()) < _barrelCutOff ?
     _mvaTrigValueCutValueEB : _mvaTrigValueCutValueEE );
    
    // Retrieve the variable value for this particle
    const float mvaTrigValue = (*_mvaTrigValueMap)[cand];
    
    // Apply the cut and return the result
    return mvaTrigValue > mvaTrigCutValue;
}
