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

/**
 * @struct GenieEventEncoding_t
 * @brief Structure to parse and hold GENIE event encoding information.
 *
 * This structure is used to parse the GENIE event code string and extract
 * relevant information about the neutrino interaction, such as the neutrino
 * PDG code, target nucleus, interaction type, current, and process.
 * */
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

  //! Get interaction type from string (e.g., "Weak", "EM", "Hadronic")
  inline static EPType GetInteractionType(const G4String& proc_str)
  {
    if (interaction_types.find(proc_str.data()) != interaction_types.end()) {
      return interaction_types.at(proc_str.data());
    }
    else {
      TString msg = Form("Unknown interaction type '%s'. Available options are: ", proc_str.data());
      for (const auto& pair : interaction_types) {
        msg += Form("'%s' ", pair.first.data());
      }

      G4Exception("GenieEventEncoding_t::GetInteractionType",
          "InvalidTypeString", FatalException, msg.Data());
    }

    return EPType::kUnknown;
  }

  //! Get interaction current from string (e.g., "CC", "NC", "CC+NC+interference")
  inline static ECurrent GetInteractionCurrent(const G4String& proc_str)
  {
    if (interaction_currents.find(proc_str.data()) != interaction_currents.end()) {
      return interaction_currents.at(proc_str.data());
    }
    else {
      TString msg = Form("Unknown interaction current '%s'. Available options are: ", proc_str.data());
      for (const auto& pair : interaction_currents) {
        msg += Form("'%s' ", pair.first.data());
      }

      G4Exception("GenieEventEncoding_t::GetInteractionCurrent",
          "InvalidCurrentString", FatalException, msg.Data());
    }

    return ECurrent::kUnknownCurrent;
  }

  //! Get interaction process from string (e.g., "QES", "RES", "DIS", "NuEEL", "COH", "MEC")
  inline static EProc GetInteractionProc(const G4String& proc_str) 
  {
    if (interaction_procs.find(proc_str.data()) != interaction_procs.end()) {
      return interaction_procs.at(proc_str.data());
    }
    else {
      TString msg = Form("Unknown interaction process '%s'. Available options are: ", proc_str.data());
      for (const auto& pair : interaction_procs) {
        msg += Form("'%s' ", pair.first.data());
      }
      G4Exception("GenieEventEncoding_t::GetInteractionProc",
          "InvalidProcString", FatalException, msg.Data());
    }

    return EProc::kUnknownProc;
  }

  //! Parse the process string to extract type, current, and process
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

  G4int pdgNu = {};         //! Neutrino PDG code
  G4int target = {};        //! Target nucleus PDG code
  G4int targetN = {};       //! Target nucleon number
  G4String proc = {};       //! Interaction process string
  G4int _type = {};         //! Encoded interaction type
  G4int _current = {};      //! Encoded interaction current
  G4int _proc = {};         //! Encoded interaction process
  G4int _attributes = {};   //! Additional attributes (e.g., resonance ID)

  const static std::string key_nu;          //! Neutrino key
  const static std::string key_tgt;         //! Target key
  const static std::string key_N;           //! Target nucleon key
  const static std::string key_proc;        //! Process key
  const static std::string key_res;         //! Resonance key
  const static std::string key_q;           //! Q key
  const static std::string key_charm;       //! Charm key
  const static std::string key_strange;     //! Strange key

  private: 
    //! Regular expression for parsing process strings
    static TPRegexp fType_rgx;
};




/**
 * @struct GenieEvent_t
 * @brief Structure representing a GENIE event record.
 *
 * This structure holds information about a single event generated by the GENIE neutrino event generator
 * and is used to read events from a GENIE ROOT TTree.
 * It contains particle information, event-level metadata, and kinematic variables.
 */
struct GenieEvent_t{
  Long64_t EvtNum = {}; //! GENIE tree event number
  int      nPart = {}; //! Number of particles in the event
  int      pdg[100] = {}; //! Particle PDG codes
  int      status[100] = {}; //! Particle status codes
  double   p4[100][4] = {}; //! Particle 4-momenta (px, py, pz, E)
  double   x4[100][4] = {}; //! Particle 4-positions (x, y, z, t)
  double   vtx[4] = {}; //! Event vertex position (x, y, z, t)
  double   weight = {}; //! Event weight
  double   nuEnergy = {}; //! Neutrino energy
  TObjString* evtCode = nullptr; //! GENIE event code string
  GenieEventEncoding_t info; //! Encoded event information

  inline ~GenieEvent_t() {}
};

/**
 * @class SLArGENIEGeneratorAction
 * @brief Primary generator action for GENIE neutrino events in SoLAr.
 *
 * This class implements a Geant4 primary generator action that reads neutrino interaction events
 * from a GENIE ROOT TTree and generates primary particles in the Geant4 simulation.
 * It supports event selection based on various criteria.
 *
 * The generator configuration must be passed via a JSON object, 
 * which should include the GENIE tree file path and optional selection filters.
 * Each selection filter works applying an "AND" logic between its criteria,
 * and an "OR" logic between different filters.
 *
 * For example, the following JSON configuration selects events with either
 * an electron neutrino Quasi-Elastic CC interaction  or a muon neutrino resonant CC interaction
 *
 * ```json
 * 
 * {
 *   "generator" : {
 *     "type" : "genie", 
 *     "label" : "genie_test", 
 *     "config" : {
 *       "genie_tree" : {
 *         "filename" : "nuSCOPE_test.root",
 *         "objname" : "gRooTracker"
 *       },
 *       "tree_first_entry" : 0
 *       "selection" : [
 *         {
 *           "nu_pdg" : [12],
 *           "interaction_current" : ["CC"],
 *           "interaction_process" : ["QES"]
 *         },
 *         {
 *           "nu_pdg" : [14],
 *           "interaction_current" : ["CC"],
 *           "interaction_process" : ["RES"]
 *         } 
 *       ],
 *     }
 *   }
 * }
 * ```
 *
 * Additionally, one can require specific target nuclei via the `target_nuclei` field,
 * target nucleons via the `target_nucleon` field, and specific final-state
 * products via the `products_pdg` field.
 *
 * Here is a breakdown of the available selection criteria:
 * | Label                  | Description                                                | Type          |
 * |------------------------|------------------------------------------------------------|---------------|
 * | nu_pdg                 | Neutrino PDG codes to select                               | Array of int  |
 * | target_nucleus         | Target nucleus PDG codes to select                         | Array of int  |
 * | target_nucleon         | Target nucleon numbers to select                           | Array of int  |
 * | interaction_current    | Interaction current types to select (e.g., "CC", "NC")     | Array of str  |
 * | interaction_process    | Interaction process types to select ("QES", "RES", "DIS", "NuEEL", "COH", "MEC") | Array of str  |
 * | products_pdg           | Final-state product PDG codes to require                    | Array of int  |
 * 
 **/

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

    G4String WriteConfig() const override;

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
