//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: SLArCRTHit.cc 76474 2013-11-11 10:36:34Z gcosmo $
//
/// \file SLArCRTHit.cc
/// \brief Implementation of the SLArCRTHit class

#include "detector/CRT/SLArCRTHit.hh"

#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4AttDefStore.hh"
#include "G4AttDef.hh"
#include "G4AttValue.hh"
#include "G4UIcommand.hh"
#include "G4UnitsTable.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ThreadLocal G4Allocator<SLArCRTHit>* SLArCRTHitAllocator;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArCRTHit::SLArCRTHit()
  : G4VHit(), fCRTNo(0), fPDG(0), fTime(0.), fEkin(0.), fLocalPos(0), fWorldPos(0), fDirection(0)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArCRTHit::~SLArCRTHit()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArCRTHit::Draw()
{
    G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
    if(pVVisManager)
    {
        G4Circle circle(fWorldPos);
        circle.SetScreenSize(2);
        circle.SetFillStyle(G4Circle::filled);
        G4Colour colour(1.,1.,0.);
        G4VisAttributes attribs(colour);
        circle.SetVisAttributes(attribs);
        pVVisManager->Draw(circle);
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const std::map<G4String,G4AttDef>* SLArCRTHit::GetAttDefs() const
{
    G4bool isNew;
    std::map<G4String,G4AttDef>* store
      = G4AttDefStore::GetInstance("SLArCRTHit",isNew);

    if (isNew) {
        (*store)["HitType"] 
          = G4AttDef("HitType","Hit Type","Physics","","G4String");
        
        (*store)["Time"] 
          = G4AttDef("Time","Time","Physics","ns","G4double");
        
        (*store)["Pos"] 
          = G4AttDef("Pos", "Position", "Physics","G4BestUnit","G4ThreeVector");
    
        (*store)["CRTNo"] 
          = G4AttDef("CRTNo","Position","Physics","","G4int");
    }
    return store;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

std::vector<G4AttValue>* SLArCRTHit::CreateAttValues() const
{
    std::vector<G4AttValue>* values = new std::vector<G4AttValue>;
    
    values
      ->push_back(G4AttValue("HitType","CRTHit",""));
    values
      ->push_back(G4AttValue("Time", G4BestUnit(fTime,"Time"), ""));
    values
      ->push_back(G4AttValue("Pos", G4BestUnit(fWorldPos,"Length"),""));
    values
      ->push_back(G4AttValue("CellNo", G4UIcommand::ConvertToString(fCRTNo), ""));

    return values;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArCRTHit::Print()
{

    G4cout << " CRT#: " << fCRTNo << "\n"
           << " : pdg "         << fPDG << "\n"
           << " : time "         << fTime/CLHEP::ns << " (ns)" << "\n"
           << " : Ekin "         << fEkin << " (MeV)" << "\n"
           << " --- global (x,y,z) "<< G4BestUnit(fWorldPos.x(), "Length")
           << ", " << G4BestUnit(fWorldPos.y(), "Length")
           << ", " << G4BestUnit(fWorldPos.z(), "Length") << "\n"
           << " --- dir (dx,dy,dz) " << fDirection.x()
           << ", " << fDirection.y()
           << ", " << fDirection.z() << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......