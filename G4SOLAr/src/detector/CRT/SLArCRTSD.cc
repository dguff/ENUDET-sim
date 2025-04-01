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
// $Id: SLArCRTSD.cc 76474 2013-11-11 10:36:34Z gcosmo $
//
/// \file SLArCRTSD.cc
/// \brief Implementation of the SLArCRT class

#include "detector/CRT/SLArCRTSD.hh"
#include "detector/CRT/SLArCRTHit.hh"

#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArCRTSD::SLArCRTSD(G4String name, G4int crtID)
: G4VSensitiveDetector(name), fHitsCollection(0), fHCID(-1), fCRTID(crtID)
{
    collectionName.insert("CRT"+std::to_string(crtID)+"Coll");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArCRTSD::~SLArCRTSD()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArCRTSD::Initialize(G4HCofThisEvent* hce)
{
  fHitsCollection 
    = new SLArCRTHitsCollection(SensitiveDetectorName,collectionName[0]);
  if (fHCID<0) { 
    fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection); 
  }

  hce->AddHitsCollection(fHCID,fHitsCollection);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool SLArCRTSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  G4StepPoint* preStepPoint  = step->GetPreStepPoint();
  //G4StepPoint* postStepPoint = step->GetPostStepPoint();

  // check if the particle is entering the sensitive volume
  auto stepStatus = preStepPoint->GetStepStatus();
  if (stepStatus != fGeomBoundary)
    return true;

  G4TouchableHistory* touchable
    = (G4TouchableHistory*)(step->GetPreStepPoint()->GetTouchable());

  G4Track *track = step->GetTrack();

#ifdef SLAR_DEBUG
  printf("SLArCRTSD::ProcessHits(): processing %s [%i] CRT hit\n", 
      step->GetTrack()->GetParticleDefinition()->GetParticleName().data(), 
      step->GetTrack()->GetTrackID());
  //getchar(); 
#endif

  G4ThreeVector worldPos = preStepPoint->GetPosition();
  G4ThreeVector localPos = touchable->GetHistory()->GetTopTransform().TransformPoint(worldPos);
  G4ThreeVector partDir = preStepPoint->GetMomentumDirection();
  G4int pdgCode = track->GetParticleDefinition()->GetPDGEncoding();

  SLArCRTHit* hit = new SLArCRTHit();

  hit->SetPDG(pdgCode);
  hit->SetWorldPos(worldPos);
  hit->SetLocalPos(localPos);
  hit->SetDir(partDir);
  hit->SetTime(preStepPoint->GetGlobalTime());
  hit->SetEkin(preStepPoint->GetKineticEnergy());
  hit->SetCRTNo(preStepPoint->GetTouchableHandle()->GetCopyNumber());

  hit->Print();

  fHitsCollection->insert(hit);
    
  return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArCRTSD::EndOfEvent(G4HCofThisEvent *hce)
{
}