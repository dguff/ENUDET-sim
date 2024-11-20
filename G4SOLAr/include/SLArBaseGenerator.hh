/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArBaseGenerator.hh
 * @created     : Saturday Mar 30, 2024 22:02:27 CET
 */

#ifndef SLARBASEGENERATOR_HH

#define SLARBASEGENERATOR_HH

#include <G4String.hh>
#include <G4IonTable.hh>
#include <G4ParticleTable.hh>
#include <G4SystemOfUnits.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <SLArVertextGenerator.hh>
#include <event/SLArGenRecords.hpp>
#include <rapidjson/document.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TH2F.h>
#include <TGraph.h>

namespace gen{

  enum  EDetectorSite {kSURF = 0};

  enum  EDirectionMode {kFixedDir = 0, kRandomDir = 1, kSunDir = 2};

  enum  EEnergyMode {kFixed = 0, kCustom = 1, kExtSpectrum = 2}; 

  enum  EGenerator {
    kParticleGun=0
      ,kParticleBomb=1 
      ,kDecay0=2 
      ,kMarley=3 
      ,kBackground=4 
      ,kExternalGen=5
      ,kGENIE=6
#ifdef SLAR_CRY
      ,kCRY=7
#endif 
#ifdef SLAR_RADSRC
      ,kRadSrc=8
#endif // DEBUG
      ,kCorsika=9 //--JM
      ,kUndefinedGen = 99

  };

  static const std::map<G4String, EGenerator> genMap = {
    {"particlegun", EGenerator::kParticleGun}
    ,{"particlebomb", EGenerator::kParticleBomb} 
    ,{"decay0", EGenerator::kDecay0}
    ,{"marley", EGenerator::kMarley}
    ,{"external", EGenerator::kExternalGen}
    ,{"genie", EGenerator::kGENIE}
#ifdef SLAR_CRY
    ,{"cry", EGenerator::kCRY}
#endif
#ifdef SLAR_RADSRC
    ,{"radsrc", EGenerator::kRadSrc}
#endif
    ,{"corsika", EGenerator::kCorsika}
  };

  static inline EGenerator GetGeneratorIndex(const G4String gen_type) {
    EGenerator kGen = EGenerator::kUndefinedGen;
    if (genMap.find(gen_type) != genMap.end()) {
      kGen = genMap.find(gen_type)->second;
    }
    else {
      fprintf(stderr, "Unknown generator %s\n", gen_type.data()); 
      exit(EXIT_FAILURE); 
    }
    return kGen;
  }

  static inline void printGeneratorType() {
    printf("Available primary generators:\n");
    for (const auto& gen : genMap) {
      printf("\t- %s\n", gen.first.data());
    }
    return;
  }

  class SLArBaseGenerator : public G4VUserPrimaryGeneratorAction {
    public: 
      struct ExtSourceInfo_t {
        G4String filename; 
        G4String objname;
        G4String type; 

        ExtSourceInfo_t() : filename(), objname(), type() {}; 
        inline const bool is_empty() {
          return ( filename.empty() && objname.empty() ); 
        }

        inline void Configure(const rapidjson::Value& jconfig) {
          if (jconfig.HasMember("filename") == false || 
              jconfig.HasMember("objname") == false) {
            fprintf(stderr, "ExtHistInfo_t::Configure ERROR. fields \"filename\" and \"objname\" required\n"); 
            exit(EXIT_FAILURE); 
          }
          filename = jconfig["filename"].GetString(); 
          objname = jconfig["objname"].GetString(); 
          if (jconfig.HasMember("type")) {
            type = jconfig["type"].GetString(); 
          }
          return; 
        }

        const rapidjson::Document ExportConfig() const;
      };
      
      struct DirectionConfig_t {
        EDirectionMode mode = EDirectionMode::kFixedDir; 
        G4ThreeVector  axis{0, 0, 0};
        ExtSourceInfo_t  nadir_hist = {}; 
        G4ThreeVector  direction_tmp{0, 0, 0}; 
        G4double       cos_nadir_tmp = {}; 
      };

      struct EnergyConfig_t {
        EEnergyMode mode = EEnergyMode::kFixed; 
        G4String energy_distribution_label = {};
        G4double energy_value = 0.0*CLHEP::MeV; 
        G4double energy_tmp = {}; 
        ExtSourceInfo_t spectrum_hist = {}; 
        G4double energy_min = {}; 
        G4double energy_max = {}; 
        G4double temperature = {}; 
        G4double eta = {}; 
        G4double energy_mean = {}; 
        G4double beta = {};
        std::vector<G4double> energy_bin_left = {};
        std::vector<G4double> energies = {};
        std::vector<G4double> weights = {};
        std::vector<G4double> prob_densities = {};
        G4String interpolation_rule = {};
      };

      struct GenConfig_t {
        G4int n_particles = 1; 
        DirectionConfig_t  dir_config = {};
        EnergyConfig_t ene_config = {}; 
      };

      inline SLArBaseGenerator(const G4String label="") 
        : G4VUserPrimaryGeneratorAction(), fVerbose(0), fLabel(label) {}
      inline virtual ~SLArBaseGenerator() {};

      inline void SetVerboseLevel(const G4int i) {fVerbose = i;}
      inline G4int GetVerboseLevel() const {return fVerbose;}

      inline void SetLabel(const G4String& label) {fLabel = label;}
      inline G4String GetLabel() const {return fLabel;}
      virtual void SourceConfiguration(const rapidjson::Value& config) = 0; 
      //virtual void SourceConfiguration(const rapidjson::Value& config, GenConfig_t& local) = 0; 
      virtual void Configure() { Configure(fConfig); } 
      virtual void Configure(const GenConfig_t& config); 

      void SetupVertexGenerator(const rapidjson::Value& config);

      virtual G4String GetGeneratorType() const = 0; 
      virtual EGenerator GetGeneratorEnum() const = 0; 

      template <typename T>
        T* GetFromRootfile(const G4String& filename, const G4String& key)
        {
          TFile* rootFile = TFile::Open(filename, "READ");
          if (!rootFile || rootFile->IsZombie()) {
            std::fprintf(stderr, "GetFromRootfile ERROR: Cannot open file %s", filename.data()); 
            exit(EXIT_FAILURE);
          }
          T* obj = dynamic_cast<T*>(rootFile->Get(key));
          obj->SetDirectory(0);
          if (!obj) {
            rootFile->Close();
            std::fprintf(stderr, "GetFromRoofile ERROR: Unable to find object with key %s", key.data());
            exit(EXIT_FAILURE);
          }
          rootFile->Close(); 
          return std::move(obj);
        }

      template <typename T>
        T* GetFromRootfile(const rapidjson::Value& file_and_key)
        {
          if (!file_and_key.HasMember("file") || !file_and_key.HasMember("key")) {
            std::fprintf(stderr, "GetFromRootfile Error: mandatory 'file' or 'key' field missing."); 
            exit(EXIT_FAILURE);
          } 
          G4String filename = file_and_key["file"].GetString();
          G4String key = file_and_key["key"].GetString();
          T* obj = GetFromRootfile<T>(filename, key); 
          return std::move(obj);
        }

      virtual void SetGenRecord( SLArGenRecord& record, const GenConfig_t& config ) const; 
      inline virtual void SetGenRecord( SLArGenRecord& record) const {
        SetGenRecord( record, fConfig ); 
      } 
      virtual G4String WriteConfig() const;

      virtual void GeneratePrimaries(G4Event*) = 0; 
      void RegisterPrimaries(const G4Event*, const G4int); 

      inline static G4bool PDGCodeIsValid(const G4int pdgcode) {
        G4ParticleDefinition* def = nullptr; 
        def = G4ParticleTable::GetParticleTable()->FindParticle(pdgcode);
        if ( def != nullptr ) {return true;}
        def = G4IonTable::GetIonTable()->GetIon(pdgcode);
        if ( def != nullptr ) {return true;}
        return false;
      }

    protected: 
      G4int fVerbose;
      G4String fLabel;
      GenConfig_t fConfig; 
      std::unique_ptr<SLArVertexGenerator> fVtxGen;
      std::unique_ptr<TH1D> fNadirDistribution; 
      std::unique_ptr<TH1D> fEnergySpectrum; 
      G4String fJSONConfigDump;

      void CopyConfigurationToString(const rapidjson::Value& config); 
      virtual void SourceDirectionConfig(const rapidjson::Value& dir_config); 
      virtual void SourceDirectionConfig(const rapidjson::Value& dir_config, DirectionConfig_t& local); 
      virtual void SourceEnergyConfig(const rapidjson::Value& ene_config); 
      virtual void SourceEnergyConfig(const rapidjson::Value& ene_config, EnergyConfig_t& local); 
      virtual const rapidjson::Document ExportDirectionConfig() const; 
      virtual const rapidjson::Document ExportEnergyConfig() const; 
      virtual G4double SampleEnergy(); 
      virtual G4double SampleEnergy(EnergyConfig_t& ene_config); 
      virtual G4ThreeVector SampleDirection(); 
      virtual G4ThreeVector SampleDirection(DirectionConfig_t& dir_config); 
  };

}

#endif /* end of include guard SLARVERTEXGENERATORINTERFACE_HH */

