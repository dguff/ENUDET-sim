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
/// \file SLAr/include/SLArPrimaryGeneratorAction.hh
/// \brief Definition of the SLArPrimaryGeneratorAction class
//
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef SLArPrimaryGeneratorAction_h
#define SLArPrimaryGeneratorAction_h 1

#include <G4VUserPrimaryGeneratorAction.hh>
#include <SLArBaseGenerator.hh>
#include <G4VPhysicalVolume.hh>
#include <globals.hh>

#include <rapidjson/document.h>

class G4Event;
namespace gen {
  class SLArBulkVertexGenerator;
  class SLArBoxSurfaceVertexGenerator;
  class SLArPrimaryGeneratorMessenger;
  class SLArPGunGeneratorAction; 
  class SLArPBombGeneratorAction; 
  //class SLArBackgroundGeneratorAction;
  class SLArExternalGeneratorAction;
  class SLArGENIEGeneratorAction;//--JM


  namespace bxdecay0_g4 {
    class SLArDecay0GeneratorAction;
  }
  namespace marley {
    class SLArMarleyGeneratorAction;
  }
#ifdef SLAR_CRY
  namespace cry {
    class SLArCRYGeneratorAction;
  }
#endif 

#ifdef SLAR_RADSRC
  namespace radsrc {
    class SLArRadSrcGeneratorAction;
  }
#endif

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
  class SLArPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
  {

    public:
      SLArPrimaryGeneratorAction();
      SLArPrimaryGeneratorAction(const rapidjson::Value& config); 
      SLArPrimaryGeneratorAction(const G4String config_file_path); 
      ~SLArPrimaryGeneratorAction();

      virtual void GeneratePrimaries(G4Event*);
      void AddGenerator(const rapidjson::Value& generatorConfig);
      void SourceConfiguration(const rapidjson::Value& config); 
      void SourceConfiguration(const G4String config_file_path); 

      //inline void SetGenerator(EGenerator gen) {fGeneratorEnum = gen;}
      inline G4VUserPrimaryGeneratorAction* GetGenerator(const G4String& label) {
        if (fGeneratorActions.find(label) != fGeneratorActions.end()) {
          return fGeneratorActions[label];
        } 
        return nullptr;
      }
      inline std::map<G4String, SLArBaseGenerator*>& GetGenerators() {return fGeneratorActions;}

      void SetVerboseLevel( G4int verbose) { 
        fVerbose = verbose; 
        for (auto& gen : fGeneratorActions) {
          gen.second->SetVerboseLevel( fVerbose ); 
        }
      }
      inline G4int GetVerboseLevel() const {return fVerbose;}

    private:
      std::map<G4String, SLArBaseGenerator*> fGeneratorActions; 

      SLArPrimaryGeneratorMessenger* fGunMessenger;


      G4int fVerbose;

      void Reset(); 

      friend class gen::SLArPrimaryGeneratorMessenger;
  };
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif /*SLArPrimaryGeneratorAction_h*/
