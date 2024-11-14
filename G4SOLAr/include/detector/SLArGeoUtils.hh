/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArGeoUtils.hpp
 * @created     Wed Apr 19, 2023 08:08:00 CEST
 */

#ifndef SLARGEOUTILS_HH

#define SLARGEOUTILS_HH

#include <map>
#include <regex>

#include "G4ThreeVector.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Transform3D.hh"
#include "G4LogicalVolume.hh"

#include "rapidjson/document.h"
#include "rapidjson/allocators.h"

class G4VSolid;

namespace geo {
  enum EBoxFace {kXplus=0, kXminus=1, kYplus=2, kYminus=3, kZplus=4, kZminus=5}; 
  static std::map<EBoxFace, G4ThreeVector> BoxFaceNormal  = {
    {kXplus, G4ThreeVector(-1, 0, 0)},
    {kXminus, G4ThreeVector(+1, 0, 0)},
    {kYplus, G4ThreeVector(0, -1, 0)},
    {kYminus, G4ThreeVector(0, +1, 0)},
    {kZplus, G4ThreeVector(0, 0, -1)},
    {kZminus, G4ThreeVector(0, 0, +1)}
  };

  double get_bounding_volume_surface(const G4VSolid* solid);

  inline std::vector<G4VPhysicalVolume*> GetPhysicalVolumes(const G4LogicalVolume* lv) 
  {
    G4PhysicalVolumeStore* pvs = G4PhysicalVolumeStore::GetInstance();
    std::vector<G4VPhysicalVolume*> pv_list;
    for (auto pv : *pvs) {
      if (pv->GetLogicalVolume() == lv) {
        pv_list.push_back(pv);
      }
    }

    return pv_list;
  }

  inline static const G4Transform3D GetTransformToGlobal(G4VPhysicalVolume* pv) {
    G4Transform3D globalTransform = G4Transform3D::Identity;
    while (pv->GetMotherLogical() != nullptr) {
      printf("pv name: %s\n", pv->GetName().data());
      G4Transform3D localTransform(pv->GetObjectRotationValue(), pv->GetObjectTranslation());
      globalTransform = localTransform * globalTransform;
      auto mother = pv->GetMotherLogical();
      const auto pv_list = GetPhysicalVolumes(mother);
      if (pv_list.size() > 1) {
        printf("GetTransformToGlobal() WARNING: more than one physical volume with the same logical volume\n");
      }
      pv = pv_list[0];
    }
    return globalTransform;
  }
  


}



#endif /* end of include guard SLARGEOUTILS_HH */

