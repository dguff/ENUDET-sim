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
        flagged_solids.insert(solid);
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
}

