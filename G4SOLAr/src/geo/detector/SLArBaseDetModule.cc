/* * * * * * * * * * * * * * * * * * * * * * * *  
 * @author      : guff (guff@guff-gssi)
 * @file        : SLArBaseModule
 * @created     : mercoledì ago 07, 2019 13:24:21 CEST
 */

#include "detector/SLArBaseDetModule.hh"
#include "SLArGeoInfo.hh"

#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"


SLArBaseDetModule::SLArBaseDetModule() 
  : fMaterial(nullptr), fModLV(nullptr), fModSV(nullptr), 
  fRot(nullptr), fVec(0., 0., 0.), fName(""), fID(999)
{
  fGeoInfo = new SLArGeoInfo();
}

SLArBaseDetModule::SLArBaseDetModule(const SLArBaseDetModule &base)
{
  fMaterial = base.fMaterial;
  fGeoInfo  = base.fGeoInfo ;
  fModLV    = base.fModLV   ;
  fModSV    = base.fModSV   ;
  fModPV    = base.fModPV   ;
  fRot      = base.fRot     ;
  fVec      = base.fVec     ;
  fName     = base.fName    ;
  fID       = base.fID      ; 
}

SLArBaseDetModule::~SLArBaseDetModule() {
  if (fGeoInfo) {delete fGeoInfo; fGeoInfo = nullptr;}
  //if (fModPV )  {delete fModPV  ; fModPV   = nullptr;}
  if (fModLV )  {delete fModLV  ; fModLV   = nullptr;}
  if (fModSV )  {delete fModSV  ; fModSV   = nullptr;}
}

void SLArBaseDetModule::SetSolidVolume(G4VSolid* sol_vol)
{
  fModSV = sol_vol;
}

void SLArBaseDetModule::SetLogicVolume(G4LogicalVolume* log_vol)
{
  fModLV = log_vol;
}
  
G4VPhysicalVolume* SLArBaseDetModule::GetModPV(
        G4String                          name,
        G4RotationMatrix*                 rot,
        const G4ThreeVector               &vec,
        G4LogicalVolume*                  mlv,
        G4bool                            pMany,
        G4int                             pCopyNo)
{
  fRot  = rot;
  fName = name;
  fVec  = vec;
  if (pCopyNo == 0) pCopyNo = fID;
  else fID = pCopyNo;

  fModPV = new G4PVPlacement(fRot,fVec, 
      fModLV, fName, mlv, pMany, pCopyNo, true);
  return fModPV;
}

SLArGeoInfo* SLArBaseDetModule::GetGeoInfo()
{
  return fGeoInfo;
}

G4bool SLArBaseDetModule::ContainsGeoPar(G4String str)
{
  return fGeoInfo->Contains(str);
}

void SLArBaseDetModule::SetGeoPar(G4String str, G4double val)
{
  fGeoInfo->SetGeoPar(str, val);
}

void SLArBaseDetModule::SetGeoPar(std::pair<G4String, G4double> p)
{
  fGeoInfo->SetGeoPar( p );
}

G4double SLArBaseDetModule::GetGeoPar(G4String str)
{
  return fGeoInfo->GetGeoPar(str);
}

void SLArBaseDetModule::ResetGeometry() 
{
  if (fModLV) {
    fModLV->RemoveDaughter(fModPV);
    delete fModPV; fModPV = nullptr; 
  }
}

void SLArBaseDetModule::SetMaterial(G4Material* mat)
{
  fMaterial = mat;
}

G4Material* SLArBaseDetModule::GetMaterial()
{
  return fMaterial;
}
