/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGPSDirectionGenerator.hh
 * @created     : Wednesday Feb 12, 2025 11:40:20 CET
 */

#ifndef SLARGPSDIRECTIONGENERATOR_HH

#define SLARGPSDIRECTIONGENERATOR_HH

#include "SLArDirectionGenerator.hh"
#include "SLArRandomExtra.hh"
#include "G4SPSAngDistribution.hh"
#include "G4SPSRandomGenerator.hh"

namespace gen{
namespace direction{

  class SLArGPSDirectionGenerator : public SLArDirectionGenerator {
    public: 
      struct GPSAngConfig_t {
        G4ThreeVector rot1 = G4ThreeVector();
        G4ThreeVector rot2 = G4ThreeVector();
        G4ThreeVector focus_point = G4ThreeVector();
        G4double mintheta = 0.0;
        G4double maxtheta = M_PI;
        G4double minphi = 0.0;
        G4double maxphi = 2.0*M_PI;
        G4double sigma_x = 0.0;
        G4double sigma_y = 0.0;
        G4double sigma_r = 0.0;
        ExtSourceInfo_t theta_hist = {};
        ExtSourceInfo_t phi_hist = {};
      };

      inline SLArGPSDirectionGenerator() {
        fAngDist = std::make_unique<G4SPSAngDistribution>();
        fRandGen = std::make_unique<G4SPSRandomGenerator>();
      }

      inline ~SLArGPSDirectionGenerator() {}

      inline G4String GetType() const override {return "gps_dir_generator";}

      inline EDirectionGenerator GetDirectionGeneratorEnum() const override {return kGPSDir;}

      inline void ShootDirection(G4ThreeVector& direction) override {
        if (fAngDist == nullptr) {
          fprintf(stderr, "SLArGPSDirectionGenerator ERROR: Missing angular distribution\n");
          exit(EXIT_FAILURE);
        }
        G4ParticleMomentum p = fAngDist->GenerateOne();
        fTmpDirection.set( p.x(), p.y(), p.z() );
        direction.set( fTmpDirection.x(), fTmpDirection.y(), fTmpDirection.z() );
        return;
      }

      inline void Config(const rapidjson::Value& config) override;

      void Print() const override;

      const rapidjson::Document ExportConfig() const override;
      

    protected: 
      std::unique_ptr<G4SPSAngDistribution> fAngDist = nullptr;
      std::unique_ptr<G4SPSRandomGenerator> fRandGen = nullptr;
      GPSAngConfig_t fGPSConfig = {};

      inline bool DoesJSONFieldExist(const rapidjson::Value& jval, const G4String& field) const {
        if (jval.HasMember(field.data()) == false) {
          G4String msg = "SLArGPSDirectionGenerator::Config ERROR: ";
          msg += "Missing mandatory field \"" + field + "\"\n";
          fprintf(stderr, "%s", msg.data());
          exit(EXIT_FAILURE);
        }
        return true;
      }

      inline bool CheckJSONFieldIsArray(const rapidjson::Value& jval, const G4String& field) const {
        if (jval[field.data()].IsArray() == false) {
          G4String msg = "SLArGPSDirectionGenerator::Config ERROR: ";
          msg += "Field \"" + field + "\" must be a rapidjson::Array\n";
          fprintf(stderr, "%s", msg.data());
          exit(EXIT_FAILURE);
        }
        return true;
      }

      inline bool CheckJSONFieldIsNumber(const rapidjson::Value& jval, const G4String& field) const {
        if (jval[field.data()].IsNumber() == false) {
          G4String msg = "SLArGPSDirectionGenerator::Config ERROR: ";
          msg += "Field \"" + field + "\" must be a number\n";
          fprintf(stderr, "%s", msg.data());
          exit(EXIT_FAILURE);
        }
        return true;
      }

      inline bool CheckJSONFieldIsObject(const rapidjson::Value& jval, const G4String& field) const {
        if (jval[field.data()].IsObject() == false) {
          G4String msg = "SLArGPSDirectionGenerator::Config ERROR: ";
          msg += "Field \"" + field + "\" must be a rapidjson::Object\n";
          fprintf(stderr, "%s", msg.data());
          exit(EXIT_FAILURE);
        }
        return true;
      }

      inline bool CheckJSONFieldIsString(const rapidjson::Value& jval, const G4String& field) const {
        if (jval[field.data()].IsString() == false) {
          G4String msg = "SLArGPSDirectionGenerator::Config ERROR: ";
          msg += "Field \"" + field + "\" must be a string\n";
          fprintf(stderr, "%s", msg.data());
          exit(EXIT_FAILURE);
        }
        return true;
      }

      inline bool CheckJSONFieldIsSValue(const rapidjson::Value& j, const G4String& field) const {
        if (j.HasMember("val") == false) {
          G4String msg = "SLArGPSDirectionGenerator::Config ERROR: ";
          msg += "Field \"" + field + "\" must have a \"val\" field\n";
          fprintf(stderr, "%s", msg.data());
          exit(EXIT_FAILURE);
        }

        const auto& jval = j["val"];
        if (jval.IsNumber() == false && jval.IsArray() == false) {
          G4String msg = "SLArGPSDirectionGenerator::CheckJSONFieldIsSValue ERROR: ";
          msg += "Field \"val\" must be a number or an array\n";
          fprintf(stderr, "%s", msg.data());
          exit(EXIT_FAILURE);
        }
        
        if (j.HasMember("unit")) {
          CheckJSONFieldIsString(j, "unit");
        }
        return true;
      }
  };

}
}


#endif /* end of include guard SLARGPSDIRECTIONGENERATOR_HH */

