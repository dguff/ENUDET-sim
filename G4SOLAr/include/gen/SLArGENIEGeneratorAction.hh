#ifndef SLARGENIEGENERATORACTION_HH
#define SLARGENIEGENERATORACTION_HH

#include <iostream>
#include <string>
#include <vector>

#include "SLArBaseGenerator.hh"
#include "SLArGeneratorConfig.hh"

#include "TFile.h"
#include "TTree.h"

#include "G4Event.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"

namespace gen {
struct GenieEvent_t{
  Long64_t EvtNum;
  int nPart;
  int pdg[100];
  int status[100];
  double p4[100][4];
  double x4[100][4];
  double vtx[4];
};

class SLArGENIEGeneratorAction : public SLArBaseGenerator
{
  private:

  public:
    struct GENIEConfig_t : public GenConfig_t {
      ExtSourceInfo_t tree_info; 
      G4int           tree_first_entry = 0; 
    };
    SLArGENIEGeneratorAction(const G4String label = "");
    SLArGENIEGeneratorAction(const G4String label, const G4String genie_file);
    ~SLArGENIEGeneratorAction();

    G4String GetGeneratorType() const override {return "genie";}
    EGenerator GetGeneratorEnum() const override {return kGENIE;}
    
    void SourceConfiguration(const rapidjson::Value& config) override;
    void Configure() override;

    void SetGENIEEvntExt(G4int evntID);  
    void Initialize();

    //G4String WriteConfig() const override;

    virtual void GeneratePrimaries(G4Event* evnt) override;

    inline virtual void SetGenRecord( SLArGenRecord& record) const override {
      SLArBaseGenerator::SetGenRecord(record, fConfig);
    }

  protected:
    GENIEConfig_t fConfig; 
    TTree *m_gtree {};
    TFile *m_gfile {}; 
    G4Transform3D fTransform;
    GenieEvent_t gVar;
};

}

#endif
