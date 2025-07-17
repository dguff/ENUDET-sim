/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGeoUtils
 * @created     : Monday Feb 12, 2024 12:27:48 CET
 */

#include <utility>
#include <unordered_set>
#include <regex>

#include "geo/SLArGeoUtils.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Box.hh"


namespace geo {
  double get_bounding_volume_surface(const G4VSolid* solid) {
    static std::unordered_set<const G4VSolid*> flagged_solids;

    if (dynamic_cast<const G4Box*>(solid)) {
      const auto box = (G4Box*)solid;
      return box->GetSurfaceArea(); 
    }
    else {
      if (flagged_solids.find(solid) == flagged_solids.end()) {
        flagged_solids.insert(const_cast<G4VSolid*>(solid));
        printf("geo::get_bounding_volume_surface: WARNING "); 
        printf("get_bounding_volume_surface is only implemented for G4Box solids. "); 
        printf("Feel free to work on your solid's implementation and let me know!\n");
        printf("Using a box approximation.\n");
      }

      G4ThreeVector lo; 
      G4ThreeVector hi;
      G4ThreeVector dim; 
      solid->BoundingLimits(lo, hi);

      for (int i=0; i<3; i++) dim[i] = fabs(hi[i] - lo[i]); 

      G4double half_area = dim[0]*dim[1] + dim[0]*dim[2] + dim[1]*dim[2];

      return 2*half_area; 
    }
  }

  bool track_crosses_volume(const G4ThreeVector& vtx, const G4ThreeVector& momentum_dir, const G4String& pv_name) {
    G4PhysicalVolumeStore* pvs = G4PhysicalVolumeStore::GetInstance();

    const G4VPhysicalVolume* world = pvs->GetVolume("World");
    const G4VSolid* world_solid = world->GetLogicalVolume()->GetSolid();

    const G4VPhysicalVolume* pv = pvs->GetVolume(pv_name);
    if (pv == nullptr) {
      printf("geo::track_crosses_volume: WARNING: physical volume %s not found\n", pv_name.data());
      exit(EXIT_FAILURE);
    }
    const G4VSolid* solid = pv->GetLogicalVolume()->GetSolid();

    const G4double step_len = 1.0*CLHEP::cm;
    bool inside = false;
    HepGeom::Point3D<G4double> pos(vtx.x(), vtx.y(), vtx.z());
    HepGeom::Vector3D<G4double> dir(momentum_dir.x(), momentum_dir.y(), momentum_dir.z());
    auto pv_transform = GetTransformToGlobal(pv);
    auto pt_transform = pv_transform.inverse();

    while (world_solid->Inside(pos) == kInside && inside == false) {
      const HepGeom::Point3D<G4double> xpos = pt_transform * pos;
      if (solid->Inside(xpos) == kInside) {
        inside = true;
        break;
      }
      pos += step_len*dir;
    }
    return inside;
  }

  G4LogicalVolume* searchInLogicalVolume (G4LogicalVolume* logicalVolume, const G4String& pv_name) {
    if (!logicalVolume) {
      G4cout << "Logical volume is null" << G4endl;
      return nullptr;
    }
    
    for (int i = 0; i < logicalVolume->GetNoDaughters(); ++i) {
      G4VPhysicalVolume* daughter = logicalVolume->GetDaughter(i);
      if (!daughter) {
          G4cout << "Error: Daughter volume is null." << G4endl;
          continue;
      }
      
      if (daughter->GetLogicalVolume()->GetName() == pv_name) {
          G4cout << "Found volume: " << pv_name << " inside logical volume: " << logicalVolume->GetName() << G4endl;
          return daughter->GetLogicalVolume();
      }

      if (daughter->IsParameterised()) {
          G4cout << "Exploring parametrised daughter volume: " << daughter->GetName() << G4endl;
          auto result = searchInLogicalVolume(daughter->GetLogicalVolume(), pv_name);
          if (result != nullptr) {
              return result;
          }
      }
    
      else {
        G4cout << "Exploring unparametrised daughter volume: " << daughter->GetName() << G4endl;
        auto result = searchInLogicalVolume(daughter->GetLogicalVolume(), pv_name);
          if (result != nullptr) {
              return result;
          }
      }
    }

    return nullptr;
  }

  G4LogicalVolume* searchPvVolumeInParametrisedVolume(const G4String& pv_name, const G4String& param_vol_name) {
    auto volumeStore = G4PhysicalVolumeStore::GetInstance();
    if (!volumeStore) {
        G4cout << "Error: G4PhysicalVolumeStore is not initialized." << G4endl;
        return nullptr;
    }


    for (auto vol : *volumeStore) {
      if (vol->GetName() == param_vol_name) {
          auto logicalVolume = vol->GetLogicalVolume();
          if (!logicalVolume) {
              G4cout << "Error: LogicalVolume is null for " << param_vol_name << G4endl;
              return nullptr;
          }

          return searchInLogicalVolume(logicalVolume, pv_name);
      }
    }

    G4cout << "Parametrised volume " << param_vol_name << " not found in G4PhysicalVolumeStore." << G4endl;
    return nullptr;
  }
}

