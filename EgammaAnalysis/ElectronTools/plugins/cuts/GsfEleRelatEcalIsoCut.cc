#include "PhysicsTools/SelectorUtils/interface/CutApplicatorBase.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"

class GsfEleRelatEcalIsoCut : public CutApplicatorBase {
public:
  GsfEleRelatEcalIsoCut(const edm::ParameterSet& c) :
    CutApplicatorBase(c),
    _relatECALisoCutValueEB(c.getParameter<double>("relatECALisoCutValueEB")),
    _relatECALisoCutValueEE(c.getParameter<double>("relatECALisoCutValueEE")),
    _barrelCutOff(c.getParameter<double>("barrelCutOff")) {
  }
  
  result_type operator()(const reco::GsfElectronRef&) const override final;

  CandidateType candidateType() const override final { 
    return ELECTRON; 
  }

private:
  const double _relatECALisoCutValueEB,_relatECALisoCutValueEE,_barrelCutOff;
};

DEFINE_EDM_PLUGIN(CutApplicatorFactory,
		  GsfEleRelatEcalIsoCut,
		  "GsfEleRelatEcalIsoCut");

CutApplicatorBase::result_type 
GsfEleRelatEcalIsoCut::
operator()(const reco::GsfElectronRef& cand) const{  
  const float relatECALisoCutValue =
    ( std::abs(cand->superCluster()->position().eta()) < _barrelCutOff ? 
      _relatECALisoCutValueEB : _relatECALisoCutValueEE );
  return (cand->dr03EcalRecHitSumEt()/cand->et()) < relatECALisoCutValue;
}
