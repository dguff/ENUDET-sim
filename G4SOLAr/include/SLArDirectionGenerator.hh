/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDirectionGenerator
 * @created     : Wednesday Feb 12, 2025 11:52:11 CET
 */

#ifndef SLARDIRECTIONGENERATOR_HH

#define SLARDIRECTIONGENERATOR_HH

#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RandomTools.hh"
#include "rapidjson/document.h"
#include "SLArGeoUtils.hh"
#include "SLArGeneratorConfig.hh"
#include "SLArUnit.hpp"

namespace gen {
  namespace direction {
  enum  EDirectionGenerator {
    kFixedDir = 0, 
    kIsotropicDir = 1, 
    kSunDir = 2, 
    kGPSDir = 3,
    kUndefinedDirGen = 99
  };

  static const std::map<G4String, EDirectionGenerator> dirGenMap = {
    {"fixed", EDirectionGenerator::kFixedDir}, 
    {"isotropic", EDirectionGenerator::kIsotropicDir}, 
    {"sun", EDirectionGenerator::kSunDir}, 
    {"gps_dir", EDirectionGenerator::kGPSDir}
  };

  static inline EDirectionGenerator getDirGenIndex(G4String str) {
    EDirectionGenerator kGen = EDirectionGenerator::kUndefinedDirGen;
    if (dirGenMap.find(str) != dirGenMap.end()) {
      kGen = dirGenMap.find(str)->second;
    }
    return kGen;
  }

  static inline void printDirGeneratorType() {
    printf("Available direction generators:\n");
    for (const auto& vgen : dirGenMap) {
      printf("\t-[%i] %s\n", getDirGenIndex(vgen.first), vgen.first.data());
    }
    return;
  }

  class SLArDirectionGenerator {
    public:
      SLArDirectionGenerator() = default;

      virtual ~SLArDirectionGenerator() = default;

      /// Check if the generator can provide at least one more vertex (default: return true)
      inline virtual bool HasNextVertex() const {return true;}
      virtual G4String GetType() const = 0; 
      virtual EDirectionGenerator GetDirectionGeneratorEnum() const = 0;
      virtual void ShootDirection(G4ThreeVector &  direction_) = 0;
      G4ThreeVector& GetTmpDirection() {return fTmpDirection;}
      const G4ThreeVector& GetTmpDirection() const {return fTmpDirection;}
      G4double GetTmpCosNadir() const {return -fTmpDirection.y();}
      void SetTmpDirection(const G4ThreeVector& vec) {fTmpDirection = vec;}
      virtual void Config(const rapidjson::Value&) = 0;
      virtual const rapidjson::Document ExportConfig() const = 0;  
      virtual void Print() const = 0;

      protected:
      G4ThreeVector fTmpDirection = G4ThreeVector(0.0, 0.0, 0.0);
      inline void FillGenConfigVector(const G4ThreeVector& vec, 
          rapidjson::Value& jsonArray, 
          rapidjson::Document::AllocatorType& allocator) const
      {
        jsonArray.SetArray();
        jsonArray.PushBack(vec.x(), allocator);
        jsonArray.PushBack(vec.y(), allocator);
        jsonArray.PushBack(vec.z(), allocator);
        return;
      }
  };

}
}

#endif /* end of include guard SLARDIRECTIONGENERATOR_HH */

