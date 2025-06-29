/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArRadSrcGeneratorAction.hh
 * @created     : Thursday May 02, 2024 18:13:18 CEST
 */

#ifndef SLARRADSRCGENERATORACTION_HH

#define SLARRADSRCGENERATORACTION_HH

#include <string>

#include <SLArBaseGenerator.hh>

#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4ParticleGun.hh>
#include <TRandom3.h>

#include <radsource.h>
#include <cpp_api.h>

namespace gen {
namespace radsrc {
class SLArRadSrcGeneratorAction : public SLArBaseGenerator 
{
  public: 
    struct RadSrcConfig_t : public GenConfig_t {
      G4int n_decays = 0; //!< Number of decays to simulate
      G4double flux = 0.0; //!< Flux in particles per unit area per unit time
      std::map<G4String, G4double> isotopes;
      G4double age = 4.9e9; //!< Age in years
      G4bool add_brem = true; //!< Add bremsstrahlung
      G4double min_energy = 0; //!< Min energy in MeV
      G4double max_energy = 0; //!< Max energy in MeV
      G4double  spec_activity = 0; //!< Decaying isotope specific activity (Bq/kg) 
      G4String radsrc_mess_input = {}; 
      void to_input();
    }; 
    SLArRadSrcGeneratorAction(const G4String label); 
    virtual ~SLArRadSrcGeneratorAction() {} 

    virtual void GeneratePrimaries(G4Event* ev) override; 

    void SourceConfiguration(const rapidjson::Value& config) override; 
    void Configure() override;
    G4String WriteConfig() const override;

    G4String GetGeneratorType() const override {return "radsrc";}
    EGenerator GetGeneratorEnum() const override {return EGenerator::kRadSrc;}

    void InputRadSrc();
    void UpdateRadSrc();
    G4String& GetMessInput() {return fConfig.radsrc_mess_input;}

  private:
    RadSrcConfig_t fConfig; 
    std::unique_ptr<G4ParticleGun> particleGun;
    std::unique_ptr<::radsrc::CRadSource> gen;
    G4int fInputState;

    static G4double local_rand(void);
}; 
}
}


#endif /* end of include guard SLARRADSRCGENERATORACTION_HH */

