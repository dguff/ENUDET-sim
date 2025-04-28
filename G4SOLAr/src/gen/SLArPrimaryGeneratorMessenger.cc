/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPrimaryGeneratorMessenger.cc
 * @created     Friday Dec 30, 2022 18:48:41 CET
 */


#include <cstdlib>
#include "SLArPrimaryGeneratorMessenger.hh"

#include "SLArPrimaryGeneratorAction.hh"
#ifdef SLAR_CRY
#include "SLArCRYGeneratorAction.hh"
#endif
#include "SLArBulkVertexGenerator.hh"
#include "CLHEP/Units/SystemOfUnits.h"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithoutParameter.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

namespace gen {
SLArPrimaryGeneratorMessenger::SLArPrimaryGeneratorMessenger(SLArPrimaryGeneratorAction* SLArGun)
  : G4UImessenger(),
    fSLArAction(SLArGun)
{
  fCmdGenConfig = 
    new G4UIcmdWithAString("/SLAr/gen/configure", this);
  fCmdGenConfig->SetGuidance("Set generator configuration");
  fCmdGenConfig->SetParameterName("ConfigFile", false);

  fCmdTracePhotons = 
    new G4UIcmdWithABool("/SLAr/phys/DoTracePhotons", this); 
  fCmdTracePhotons->SetGuidance("Set/unset tracing of optical photons"); 
  fCmdTracePhotons->SetParameterName("do_trace", false, true); 
  fCmdTracePhotons->SetDefaultValue(true);

  fCmdDriftElectrons = 
    new G4UIcmdWithABool("/SLAr/phys/DoDriftElectrons", this); 
  fCmdDriftElectrons->SetGuidance("Set/unset drift and collection of ionization electrons"); 
  fCmdDriftElectrons->SetParameterName("do_trace", false, true); 
  fCmdDriftElectrons->SetDefaultValue(true);

  fCmdVerbose = 
    new G4UIcmdWithAnInteger("/SLAr/gen/verbose", this); 
  fCmdVerbose->SetGuidance("verbose level"); 
  fCmdVerbose->SetParameterName("verbose", true, false); 
  fCmdVerbose->SetDefaultValue( 1 ); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArPrimaryGeneratorMessenger::~SLArPrimaryGeneratorMessenger()
{
  delete fCmdTracePhotons; 
  delete fCmdDriftElectrons;
  delete fCmdVerbose;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPrimaryGeneratorMessenger::SetNewValue(
              G4UIcommand* command, G4String newValue)
{
  if (command == fCmdTracePhotons) {
    bool do_trace = fCmdTracePhotons->GetNewBoolValue(newValue); 
    fSLArAction->SetTraceOptPhotons(do_trace); 
  }
  else if (command == fCmdDriftElectrons) {
    bool do_drift = fCmdDriftElectrons->GetNewBoolValue(newValue); 
    fSLArAction->SetDriftElectrons(do_drift); 
  }
  else if (command == fCmdGenConfig) {
    G4String config_file = newValue;
    fSLArAction->SourceConfiguration( config_file ); 
  }
  else if (command == fCmdVerbose) {
    G4String verbose_str = newValue; 
    fSLArAction->SetVerboseLevel( std::stoi( verbose_str ) ); 
  }

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
}
