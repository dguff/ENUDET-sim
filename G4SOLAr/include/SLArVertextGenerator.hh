/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArVertextGenerator
 * @created     : Friday Mar 29, 2024 14:58:43 CET
 * @brief       : Vertex generator interface class
 */

#ifndef SLARVERTEXTGENERATOR_HH

#define SLARVERTEXTGENERATOR_HH

#include <cstdio>
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RandomTools.hh"
#include "rapidjson/document.h"
#include "SLArGeoUtils.hh"
#include "SLArUnit.hpp"

namespace gen {
  namespace time {
    enum ETimeGeneratorMode {kFixed = 0, kWindow = 1}; 
    static const std::map<G4String, ETimeGeneratorMode> timeGenMap = {
      {"fixed", ETimeGeneratorMode::kFixed}, 
      {"window", ETimeGeneratorMode::kWindow}
    };

    static inline ETimeGeneratorMode GetTimeGeneratorMode(const G4String& str) {
      ETimeGeneratorMode kGen = ETimeGeneratorMode::kFixed;
      if (timeGenMap.find(str) != timeGenMap.end()) {
        kGen = timeGenMap.find(str)->second;
      }
      return kGen;
    }

    static inline G4String  GetTimeGeneratorModeString(const ETimeGeneratorMode kMode) {
      if (kMode == ETimeGeneratorMode::kFixed) {return G4String("fixed");}
      else if (kMode == ETimeGeneratorMode::kWindow) {return G4String("window"); }
      else {
        fprintf(stderr, "WARNING: Unkown time generator mode %i\n", kMode); 
        return G4String("");
      }
    }

    class SLArTimeGenerator {
      public: 
        struct TimeGenConfig_t {
          ETimeGeneratorMode mode = ETimeGeneratorMode::kFixed;
          G4double time = 0.0; 
          G4double time_min = 0.0; 
          G4double time_max = 0.0; 
        };

        inline SLArTimeGenerator() {};
        inline ~SLArTimeGenerator() {};
        inline TimeGenConfig_t& GetTimeConfig() {return fConfig;}
        inline const TimeGenConfig_t& GetTimeConfig() const {return fConfig;}

        void SourceConfiguration(const rapidjson::Value& jconfig) {
          assert(jconfig.HasMember("mode")); 
          fConfig.mode = GetTimeGeneratorMode( jconfig["mode"].GetString() ); 
          if (jconfig.HasMember("event_time")) {
            fConfig.time = unit::ParseJsonVal( jconfig["event_time"] ); 
          }
          else if (jconfig.HasMember("event_window_limits")) {
            const auto& jwindow = jconfig["event_window_limits"];
            fConfig.time_min = unit::ParseJsonVal( jwindow["t0"] );
            fConfig.time_max = unit::ParseJsonVal( jwindow["t1"] ); 
            //G4cout << "Time min: " << fConfig.time_min << G4endl;
            //G4cout << "Time max: " << fConfig.time_max << G4endl;
            //getchar();
          }
        }

        G4double CalculateTotalTime() const {
          if (fConfig.mode == ETimeGeneratorMode::kFixed) {
            return fConfig.time;
          }
          else {
            return fConfig.time_max - fConfig.time_min;
          }
        }

        inline G4double SampleTime() {
          switch (fConfig.mode) {
            case time::kFixed : {
              return fConfig.time;
              break;
            }
            case time::kWindow : {
              fConfig.time = CLHEP::RandFlat::shoot(fConfig.time_min, fConfig.time_max);
              return fConfig.time;
              break;
            }
            default : {
              fprintf(stderr, "SLArTimeGenerator::SampleTime() WARNING. Undefined vertex time sampling. Setting t = 0 ns\n"); 
              return 0.0; 
            }
          }
          return fConfig.time;
        }

        const rapidjson::Document ExportConfig() const {
          rapidjson::Document d; 
          d.SetObject(); 
          std::string mode_str = GetTimeGeneratorModeString(fConfig.mode).data();
          char buffer[50]; 
          int len = snprintf(buffer, sizeof(buffer), "%s", mode_str.data());
          rapidjson::Value jmode;
          jmode.SetString(buffer, len, d.GetAllocator());
          d.AddMember("mode", jmode, d.GetAllocator());
          if (fConfig.mode == ETimeGeneratorMode::kFixed) {
            d.AddMember("event_time_ns", fConfig.time, d.GetAllocator());
          }
          else if (fConfig.mode == ETimeGeneratorMode::kWindow) {
            rapidjson::Value jwindow; jwindow.SetArray();
            jwindow.PushBack( fConfig.time_min, d.GetAllocator()); 
            jwindow.PushBack( fConfig.time_max, d.GetAllocator()); 
            d.AddMember("event_window_ns", jwindow, d.GetAllocator());
          }
          return d;
        }

      private: 
        TimeGenConfig_t fConfig;
    };
  }

  enum EVertexGenerator {
    kUndefinedVtxGen = -1, 
    kPoint = 0, 
    kBulk = 1, 
    kBoxVolSurface = 2, 
    kGPSPos = 3};

  static const std::map<G4String, EVertexGenerator> vtxGenMap = {
    {"point", EVertexGenerator::kPoint}, 
    {"bulk", EVertexGenerator::kBulk}, 
    {"boxsurface", EVertexGenerator::kBoxVolSurface}, 
    {"gps_pos", EVertexGenerator::kGPSPos}
  };

  static inline EVertexGenerator getVtxGenIndex(G4String str) {
    EVertexGenerator kGen = EVertexGenerator::kUndefinedVtxGen;
    if (vtxGenMap.find(str) != vtxGenMap.end()) {
      kGen = vtxGenMap.find(str)->second;
    }
    return kGen;
  }

  static inline void printVtxGeneratorType() {
    printf("Available vertex generators:\n");
    for (const auto& vgen : vtxGenMap) {
      printf("\t-[%i] %s\n", getVtxGenIndex(vgen.first), vgen.first.data());
    }
    return;
  }

  class SLArVertexGenerator {
    public:
      SLArVertexGenerator() = default;

      virtual ~SLArVertexGenerator() = default;

      /// Check if the generator can provide at least one more vertex (default: return true)
      inline virtual bool HasNextVertex() const {return true;}
      virtual G4String GetType() const = 0; 
      virtual void ShootVertex(G4ThreeVector &  vertex_) = 0;
      virtual void Config(const rapidjson::Value&) = 0;
      virtual const rapidjson::Document ExportConfig() const = 0;  
      virtual void Print() const = 0;
      inline time::SLArTimeGenerator& GetTimeGenerator() {return fTimeGen;}
      inline const time::SLArTimeGenerator& GetTimeGenerator() const {return fTimeGen;}

      time::SLArTimeGenerator fTimeGen = {}; 

      protected:
      inline void FillConfigVector(const G4ThreeVector& vec, 
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

  class SLArPointVertexGenerator : public SLArVertexGenerator {
    public: 
      inline SLArPointVertexGenerator() : fVertex(0.0, 0.0, 0.0) {}

      inline SLArPointVertexGenerator(const G4ThreeVector& v) {
        fVertex.set( v.x(), v.y(), v.z() ); 
      }

      inline ~SLArPointVertexGenerator() {}

      inline G4String GetType() const override {return "point_vertex_generator";}

      inline G4ThreeVector GetVertex() const {return fVertex;}

      inline void ShootVertex(G4ThreeVector& vertex) override {
        HepGeom::Point3D<G4double> vtx(fVertex.x(), fVertex.y(), fVertex.z());
        vtx = fTransform * vtx;
        vertex.set( vtx.x(), vtx.y(), vtx.z() );
        return;
      }

      inline void Config(const rapidjson::Value& config) override {
        if ( config.HasMember("time") ) {
          fTimeGen.SourceConfiguration( config["time"] );
        }
        if ( config.HasMember("volume") ) {
          fReferenceVolumeName = config["volume"].GetString(); 

          G4PhysicalVolumeStore* pvstore = G4PhysicalVolumeStore::GetInstance();
          auto ref = pvstore->GetVolume(fReferenceVolumeName);
          const auto to_global = geo::GetTransformToGlobal(ref);
          fTransform = to_global;
        }
        if ( !config.HasMember("xyz") ) {
          throw std::invalid_argument("point vtx gen missing mandatory \"xyz\" field\n");
        }

        const auto& jxyz = config["xyz"];
        if (jxyz.HasMember("val") == false) {
          throw std::invalid_argument("field \"val\" not found in \"xyz\" field\n");
        }
        const auto& jxyz_val = jxyz["val"];
        if (jxyz_val.IsArray() == false) {
          throw std::invalid_argument("field \"value\" must be a rapidjson::Array\n");
        }
        G4double vunit = unit::GetJSONunit(jxyz);
        assert(jxyz_val.Size() == 3);
        fVertex.setX( jxyz_val.GetArray()[0].GetDouble() * vunit ); 
        fVertex.setY( jxyz_val.GetArray()[1].GetDouble() * vunit ); 
        fVertex.setZ( jxyz_val.GetArray()[2].GetDouble() * vunit ); 
        return;
      }

      void Print() const override {
        printf("SLArPointVertexGenerator configuration dump:\n"); 
        printf("vertex set to %g, %g, %g mm\n\n", 
            fVertex.x(), fVertex.y(), fVertex.z()); 
        if (fReferenceVolumeName.size() > 0) {
          printf("Reference volume: %s\n", fReferenceVolumeName.data());
        }
        return;
      }

      const rapidjson::Document ExportConfig() const override {
        rapidjson::Document vtx_info; 
        vtx_info.SetObject(); 

        G4String gen_type = GetType();
        char buffer[50];
        int len = snprintf(buffer, sizeof(buffer), "%s", gen_type.data());
        rapidjson::Value str_gen_type;
        str_gen_type.SetString(buffer, len, vtx_info.GetAllocator());

        vtx_info.AddMember("type", str_gen_type, vtx_info.GetAllocator()); 
        rapidjson::Value vtx_coord(rapidjson::kArrayType); 
        vtx_coord.PushBack( fVertex.x(), vtx_info.GetAllocator() ); 
        vtx_coord.PushBack( fVertex.y(), vtx_info.GetAllocator() ); 
        vtx_coord.PushBack( fVertex.z(), vtx_info.GetAllocator() ); 
        vtx_info.AddMember("vertex", vtx_coord, vtx_info.GetAllocator()); 

        if (fReferenceVolumeName.empty() == false) {
          rapidjson::Value jvol; 
          jvol.SetString(fReferenceVolumeName.data(), vtx_info.GetAllocator());
          vtx_info.AddMember("volume", jvol, vtx_info.GetAllocator());
        }

        auto dtime = fTimeGen.ExportConfig();
        rapidjson::Value jtime; jtime.SetObject();
        jtime.CopyFrom( dtime, vtx_info.GetAllocator() );
        vtx_info.AddMember("time", jtime, vtx_info.GetAllocator());

        return vtx_info; 
      }

    private: 
      G4ThreeVector fVertex;
      G4String fReferenceVolumeName = {};
      G4Transform3D fTransform = {};

  };

}


#endif /* end of include guard SLARVERTEXTGENERATOR_HH */

