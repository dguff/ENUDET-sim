/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPBombGeneratorAction
 * @created     Friday Sep 22, 2023 09:18:51 CEST
 */

#include "SLArPBombGeneratorAction.hh"
#include <SLArAnalysisManager.hh>
#include "SLArRandomExtra.hh"
#include "G4ParticlePropertyTable.hh"
#include "G4OpticalPhoton.hh"
#include "G4PrimaryVertex.hh"
#include "G4Event.hh"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace gen {
SLArPBombGeneratorAction::SLArPBombGeneratorAction(const G4String label) : 
  SLArBaseGenerator(label)
{
  fParticleTable = G4ParticleTable::GetParticleTable(); 
}

SLArPBombGeneratorAction::SLArPBombGeneratorAction(const G4String label, const int n_particle) 
  : SLArPBombGeneratorAction(label)
{
  fConfig.n_particles = n_particle;
  fParticleTable = G4ParticleTable::GetParticleTable(); 
}

void SLArPBombGeneratorAction::SetParticle(const char* particle_name) {
  G4ParticleDefinition* particle = fParticleTable->FindParticle(particle_name); 
  SetParticle(particle); 
}

void SLArPBombGeneratorAction::SetParticle(G4ParticleDefinition* particle_def) 
{
  if (particle_def) {
    fConfig.particle_name = particle_def->GetParticleName();
    fParticleDefinition = particle_def;
    return;
  } else {
    fprintf(stderr, "SLArPBombGeneratorAction::SetParticle "); 
    fprintf(stderr, "ERROR: cannot find %s in particle table.\n", 
        particle_def->GetParticleName().c_str()); 
    exit(EXIT_FAILURE);
    return;
  }
}

void SLArPBombGeneratorAction::Configure() {
  if (fConfig.dir_config.mode == EDirectionMode::kSunDir) {
    TH1D* hist_nadir = this->GetFromRootfile<TH1D>(
        fConfig.dir_config.nadir_hist.filename, 
        fConfig.dir_config.nadir_hist.objname);

    fNadirDistribution = std::unique_ptr<TH1D>( std::move(hist_nadir) ); 
    printf("SLArPBombGenerator::Configure() Sourcing nadir angle distribution\n"); 
    printf("fNadirDistribution ptr: %p\n", fNadirDistribution.get());
  }
  if (fConfig.ene_config.mode == EEnergyMode::kExtSpectrum) {
    TH1D* hist_spectrum = this->GetFromRootfile<TH1D>( 
        fConfig.ene_config.spectrum_hist.filename , 
        fConfig.ene_config.spectrum_hist.objname );

    fEnergySpectrum = std::unique_ptr<TH1D>( std::move(hist_spectrum) ); 
    printf("SLArPBombGenerator::Configure() Sourcing external energy spectrum\n"); 
    printf("fEnergySpectrum ptr: %p\n", fNadirDistribution.get());
  }

  SetParticle( fConfig.particle_name.data() );
}

void SLArPBombGeneratorAction::GeneratePrimaries(G4Event* anEvent) 
{
  G4ThreeVector vtx; fVtxGen->ShootVertex( vtx ); 
  G4double vtx_time = fVtxGen->GetTimeGenerator().SampleTime();

  G4PrimaryVertex* vertex = new G4PrimaryVertex( vtx, vtx_time ); 
  auto& gen_records = SLArAnalysisManager::Instance()->GetGenRecords();

#ifdef SLAR_DEBUG
  printf("SLArPBombGeneratorAction::GeneratePrimaries(): Generating %i %ss\n", 
      fConfig.n_particles, fConfig.particle_name.data()); 
#endif // DEBUG

  auto& record = gen_records.AddRecord( GetGeneratorEnum(), GetLabel() ); 

  for (size_t n=0; n<fConfig.n_particles; n++) {
    G4PrimaryParticle* particle = new G4PrimaryParticle(fParticleDefinition);

    fConfig.dir_config.direction_tmp = SampleDirection(fConfig.dir_config);
    fConfig.ene_config.energy_tmp = SampleEnergy();

    particle->SetMomentumDirection( fConfig.dir_config.direction_tmp ); 
    particle->SetKineticEnergy( fConfig.ene_config.energy_tmp ); 

    if (fParticleDefinition == G4OpticalPhoton::OpticalPhotonDefinition()) {
      G4ThreeVector polarization = SLArRandom::SampleLinearPolarization( fConfig.dir_config.direction_tmp ); 
      //printf("Random linear polarization: %.2f, %.2f, %.2f\n", polarization.x(), polarization.y(), polarization.z()); 
      particle->SetPolarization(polarization.x(), polarization.y(), polarization.z()); 
    }


    vertex->SetPrimary( particle ) ; 
  }

  anEvent->AddPrimaryVertex(vertex); 
  return;
}

SLArPBombGeneratorAction::~SLArPBombGeneratorAction()
{
  printf("deleting PBomb generator action... DONE\n");
}

void SLArPBombGeneratorAction::SourceConfiguration(const rapidjson::Value& config) {

  CopyConfigurationToString(config);

  if (config.HasMember("n_particles")) {
    fConfig.n_particles = config["n_particles"].GetInt();
  }

  if (config.HasMember("direction")) {
    SourceDirectionConfig( config["direction"], fConfig.dir_config );
  }

  if (config.HasMember("energy")) {
    SourceEnergyConfig( config["energy"], fConfig.ene_config );
  }

  if (fConfig.dir_config.mode == EDirectionMode::kFixedDir) {
    fprintf(stderr, "SLArPBombGeneratorAction::SourceConfiguration() WARNING. ");
    fprintf(stderr, "Fixed direction with more than 1 particle is not allowed. Reverting to isotropic sampling\n");
    fConfig.dir_config.mode = EDirectionMode::kRandomDir;
  }

  if (config.HasMember("particle")) {
    fConfig.particle_name = config["particle"].GetString();
    SetParticle( fConfig.particle_name ); 
  }

  if (config.HasMember("vertex_gen")) {
    SetupVertexGenerator( config["vertex_gen"] ); 
  }
  else {
    fVtxGen = std::make_unique<SLArPointVertexGenerator>();
  }

  return;
}

//G4String SLArPBombGeneratorAction::WriteConfig() const {
  //G4String config_str = "";

  //rapidjson::Document d; 
  //d.SetObject(); 
  //rapidjson::StringBuffer buffer;
  //rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  //G4String particle_name = fParticleDefinition->GetParticleName();
  //G4String gen_type = GetGeneratorType(); 

  //d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  //d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  //d.AddMember("particle", rapidjson::StringRef(particle_name.data()), d.GetAllocator()); 

  //const rapidjson::Document ene_doc = ExportEnergyConfig(); 
  //rapidjson::Value ene_val; 
  //ene_val.CopyFrom(ene_doc, d.GetAllocator()); 
  //d.AddMember("energy", ene_val, d.GetAllocator()); 

  //const rapidjson::Document dir_doc = ExportDirectionConfig(); 
  //rapidjson::Value dir_val; 
  //dir_val.CopyFrom(dir_doc, d.GetAllocator()); 
  //d.AddMember("direction", dir_val, d.GetAllocator()); 

  //const rapidjson::Document vtx_json = fVtxGen->ExportConfig(); 
  //rapidjson::Value vtx_config;
  //vtx_config.CopyFrom(vtx_json, d.GetAllocator()); 
  //d.AddMember("vertex_generator", vtx_config, d.GetAllocator()); 

  //d.Accept(writer);
  //config_str = buffer.GetString();
  //return config_str;
//}


}
