#ifndef SLARGENIEGENERATORACTION_HH
#define SLARGENIEGENERATORACTION_HH

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "SLArBaseGenerator.hh"
#include "SLArGeneratorConfig.hh"

#include "TFile.h"
#include "TTree.h"
#include "TObjString.h"
#include "TPRegexp.h"

#include "G4Event.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"

namespace gen {

struct GenieEventEncoding_t {
  //! Interaction type enum
  enum class EPType { kWeak = 1, kEM = 2, kHadronic = 3 , kUnknown = 0 };
  //! Interaction current enum
  enum class ECurrent { kCC = 1, kNC = 2, kInterference = 3, kUnknownCurrent = 0 };
  //! Interaction process enum
  enum class EProc { kQES = 1, kRES = 2, kDIS = 3, kNuEEL = 4, kCOH = 5, kMEC = 6, kUnknownProc = 0 };

  //! Interaction type dictionary
  static const std::unordered_map<std::string_view, EPType> interaction_types;
  //! Interaction current dictionary
  static const std::unordered_map<std::string_view, ECurrent> interaction_currents;
  //! Interaction process dictionary
  static const std::unordered_map<std::string_view, EProc> interaction_procs;

  inline static EPType GetInteractionType(const G4String& proc_str)
  {
    if (interaction_types.find(proc_str.data()) != interaction_types.end()) {
      return interaction_types.at(proc_str.data());
    }
    else {
      printf("Warning: Unknown interaction type '%s'\n", proc_str.data());
      return EPType::kUnknown;
    }
  }

  inline static ECurrent GetInteractionCurrent(const G4String& proc_str)
  {
    if (interaction_currents.find(proc_str.data()) != interaction_currents.end()) {
      return interaction_currents.at(proc_str.data());
    }
    else {
      printf("GenieEventEncoding_t WARNING: Unknown interaction current '%s'\n", proc_str.data());
      return ECurrent::kUnknownCurrent;
    }
  }
  inline static EProc GetInteractionProc(const G4String& proc_str) 
  {
    if (interaction_procs.find(proc_str.data()) != interaction_procs.end()) {
      return interaction_procs.at(proc_str.data());
    }
    else {
      printf("GeniEventEnconding_t WARNING: Unknown interaction process '%s'\n", proc_str.data());
      return EProc::kUnknownProc;
    } 
  }

  void parse_process(const TString& proc_str);
  inline void reset() {
    pdgNu = 0;
    target = 0;
    targetN = 0;
    proc = "";
    _type = 0;
    _current = 0;
    _proc = 0;
    _attributes = 0;
  }

  G4int pdgNu = {}; 
  G4int target = {}; 
  G4int targetN = {};
  G4String proc = {};
  G4int _type = {};
  G4int _current = {};
  G4int _proc = {};
  G4int _attributes = {};

  const static std::string key_nu;
  const static std::string key_tgt;
  const static std::string key_N;
  const static std::string key_proc;
  const static std::string key_res;
  const static std::string key_q;
  const static std::string key_charm;
  const static std::string key_strange;

  private: 
    static TPRegexp fType_rgx;
};




struct GenieEvent_t{
  Long64_t EvtNum = {};
  int      nPart = {};
  int      pdg[100] = {};
  int      status[100] = {};
  double   p4[100][4] = {};
  double   x4[100][4] = {};
  double   vtx[4] = {};
  double   weight = {};
  TObjString* evtCode = nullptr;
  GenieEventEncoding_t info;

  inline ~GenieEvent_t() {}
};

class SLArGENIEGeneratorAction : public SLArBaseGenerator
{
  private:

  public:
    struct GENIEConfig_t : public GenConfig_t {
      ExtSourceInfo_t tree_info; 
      G4int           tree_first_entry = 0; 

      struct Filter_t {
        std::vector<G4int> nu_pdgs;
        std::vector<G4int> target_nuclei;
        std::vector<G4int> target_nucleons;
        std::vector<GenieEventEncoding_t::ECurrent> currents; 
        std::vector<GenieEventEncoding_t::EProc> processes;
        std::vector<G4int> products_pdg = {};

        bool HasNeutrinoCuts() const { return !nu_pdgs.empty(); }
        bool HasTargetCuts() const { return !target_nuclei.empty(); }
        bool HasTargetNCuts() const { return !target_nucleons.empty(); }
        bool HasCurrrentCuts() const { return !currents.empty(); }
        bool HasProcessCuts() const { return !processes.empty(); }
        bool HasProductsCuts() const { return !products_pdg.empty(); }
      };

      std::vector<Filter_t> filters = {};
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
    G4long fCurrentEntry = -1; 
    G4Transform3D fTransform;
    GenieEvent_t gVar;

    G4bool CheckSelection(const GenieEventEncoding_t& gEnc) const;
    G4bool CheckSelection(const GenieEvent_t& gEve) const;
    void ParseEventEncoding(const char* code_str, GenieEventEncoding_t& gEnc);
};

}

#endif
