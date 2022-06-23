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
// $Id: SLArSuperCellSD.cc 76474 2013-11-11 10:36:34Z gcosmo $
//
/// \file SLArSuperCellSD.cc
/// \brief Implementation of the SLArSuperCell class

#include "detector/SuperCell/SLArSuperCellSD.hh"
#include "detector/SuperCell/SLArSuperCellHit.hh"
#include "SLArAnalysis.hh"

#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VProcess.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4PhysicalConstants.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArSuperCellSD::SLArSuperCellSD(G4String name)
: G4VSensitiveDetector(name), fHitsCollection(0), fHCID(-1)
{
    collectionName.insert("PMTColl");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArSuperCellSD::~SLArSuperCellSD()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArSuperCellSD::Initialize(G4HCofThisEvent* hce)
{
    fHitsCollection 
      = new SLArSuperCellHitsCollection(SensitiveDetectorName,collectionName[0]);
    if (fHCID<0)
    { fHCID = G4SDManager::GetSDMpointer()
              ->GetCollectionID(fHitsCollection); }

    hce->AddHitsCollection(fHCID,fHitsCollection);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool SLArSuperCellSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  G4StepPoint* preStepPoint  = step->GetPreStepPoint();
  G4StepPoint* postStepPoint = step->GetPostStepPoint();


  G4TouchableHistory* touchable
    = (G4TouchableHistory*)(step->GetPreStepPoint()->GetTouchable());
  if (step->GetTrack()->GetDynamicParticle()
      ->GetDefinition()->GetParticleName() != "opticalphoton") {
    //wavelength = h_Planck / step->GetTrack()->GetTotalEnergy();
    //else wavelength = -1;

    G4ThreeVector worldPos = preStepPoint->GetPosition();
    G4ThreeVector localPos
      = touchable->GetHistory()
        ->GetTopTransform().TransformPoint(worldPos);

    SLArSuperCellHit* hit = new SLArSuperCellHit();
    hit->SetWorldPos(worldPos);
    hit->SetLocalPos(localPos);
    hit->SetPhotonEnergy(preStepPoint->GetTotalEnergy());
    hit->SetTime(preStepPoint->GetGlobalTime());
    hit->SetPMTIdx(
        preStepPoint->GetTouchableHandle()->GetCopyNumber(1));

    fHitsCollection->insert(hit);

  }     

  return true;
}

G4bool SLArSuperCellSD::ProcessHits_constStep(const G4Step* step,
                                       G4TouchableHistory* ){

  G4Track* track = step->GetTrack();
  //G4cout << "SLArSuperCellSD::ProcessHits_constStep" << G4endl;
  //need to know if this is an optical photon
  if(track->GetDefinition()
     != G4OpticalPhoton::OpticalPhotonDefinition()) return false;

  G4double phEne = 0*eV;

  G4StepPoint* preStepPoint  = step->GetPreStepPoint();
  G4StepPoint* postStepPoint = step->GetPostStepPoint();
  
  G4TouchableHistory* touchable
    = (G4TouchableHistory*)(step->GetPostStepPoint()->GetTouchable());

  G4ThreeVector worldPos 
    = postStepPoint->GetPosition();
  G4ThreeVector localPos
    = touchable->GetHistory()
      ->GetTopTransform().TransformPoint(worldPos);
 
  //User replica number 1 since photocathode is a daughter volume
  //to the pmt which was replicated (see LXe example)
  
  SLArSuperCellHit* hit = nullptr;
  //Find the correct hit collection (in case of multple PMTs)

  if (track->GetParticleDefinition()->GetParticleName() 
      == "opticalphoton") 
  {
    // Get the creation process of optical photon
    G4String procName = "";
    if (track->GetTrackID() != 1) // make sure consider only secondaries
    {
      procName = track->GetCreatorProcess()->GetProcessName();
    }
    phEne = track->GetTotalEnergy();

    hit = new SLArSuperCellHit(); //so create new hit
    hit->SetPhotonEnergy( phEne );
    hit->SetWorldPos(worldPos);
    hit->SetLocalPos(localPos);
    hit->SetTime(preStepPoint->GetGlobalTime());
    hit->SetPMTIdx(postStepPoint->
        GetTouchableHandle()->GetCopyNumber(1));
    hit->SetPhotonProcess(procName);
    
    fHitsCollection->insert(hit);
  }
  return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......