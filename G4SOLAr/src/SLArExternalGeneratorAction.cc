/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArExternalGeneratorAction.cc
 * @created     Tua Apr 11, 2023 09:44:12 CEST
 */

#include <SLArAnalysisManager.hh>
#include <SLArExternalGeneratorAction.hh>
#include <SLArBoxSurfaceVertexGenerator.hh>
#include <SLArBulkVertexGenerator.hh>
#include <SLArRunAction.hh>
#include <SLArRandomExtra.hh>

#include <stdio.h>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/allocators.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <TFile.h>

#include "G4RandomTools.hh"
#include "G4Poisson.hh"
#include <G4RunManager.hh>
#include <G4Event.hh>
#include <G4RunManager.hh>

namespace gen {

SLArExternalGeneratorAction::SLArExternalGeneratorAction(const G4String label)
  : SLArBaseGenerator(label)
{
  fParticleGun = std::make_unique<G4ParticleGun>(); 
}

SLArExternalGeneratorAction::~SLArExternalGeneratorAction()
{}

//G4double SLArExternalGeneratorAction::SourceExternalConfig(const G4String ext_cfg_path) {
  //FILE* ext_cfg_file = std::fopen(, "r");
  //char readBuffer[65536];
  //rapidjson::FileReadStream is(ext_cfg_file, readBuffer, sizeof(readBuffer));

  //rapidjson::Document d;
  //d.ParseStream<rapidjson::kParseCommentsFlag>(is);

  //assert(d.HasMember("externals")); 
  //assert(d["externals"].IsObject());

  //auto external_cfg = d["externals"].GetObject(); 
  

  //const auto& vtx_gen_settings = external_cfg["vertex_gen"];
  //assert( vtx_gen_settings.HasMember("type")); 
  //assert( vtx_gen_settings.HasMember("config")); 
  //G4String vtx_gen_label = vtx_gen_settings["type"].GetString(); 
  //const auto& vtx_gen_config = vtx_gen_settings["config"];
  //if (vtx_gen_label == "bulk" ) {
    //fVtxGen = std::unique_ptr<SLArBoxSurfaceVertexGenerator>(); 
  //}
  //else if (vtx_gen_label == "box_surf") {
    //fVtxGen = std::unique_ptr<SLArBoxSurfaceVertexGenerator>();
  //}
  //else {
    //fVtxGen = std::unique_ptr<SLArBulkVertexGenerator>();
  //}

  //fVtxGen->Config(vtx_gen_config);

  //TFile input_file(external_cfg["file"].GetString()); 
  //if (input_file.IsOpen() == false) {
    //printf("SLArExternalGeneratorAction::SourceExternalConfig ERROR\n");
    //printf("Cannot open external background file %s.\n", external_cfg["file"].GetString()); 
    //exit(2); 
  //}
  //TH1D* h = input_file.Get<TH1D>(external_cfg["key"].GetString()); 
  //h->SetDirectory( nullptr ); 
  //input_file.Close(); 

  //fEnergySpectrum = std::make_unique<TH1D>( *h ); 
  //if (!fEnergySpectrum) {
    //printf("SLArExternalGeneratorAction::SourceExternalConfig ERROR\n");
    //printf("Cannot read key %s from external background file %s.\n", 
        //external_cfg["key"].GetString(), external_cfg["file"].GetString()); 
    //exit(2); 
  //}

  //fclose(ext_cfg_file); 
  //delete h; 

  //return 0.0; 
//}

void SLArExternalGeneratorAction::GeneratePrimaries(G4Event* ev) 
{
#ifdef SLAR_DEBUG
  printf("SLArExternalGeneratorAction::GeneratePrimaries\n");
#endif

  auto& gen_records = SLArAnalysisManager::Instance()->GetGenRecords();
  SLArRunAction* run_action = (SLArRunAction*)G4RunManager::GetRunManager()->GetUserRunAction();
  const auto slar_random = run_action->GetTRandomInterface();
  
  for (size_t iev = 0; iev < fConfig.n_particles; iev++) {
    G4ThreeVector vtx_pos(0, 0, 0); 
    fVtxGen->ShootVertex(vtx_pos);

    //printf("Energy spectrum pointer: %p\n", fEnergySpectrum.get());
    //printf("Energy spectrum from %s\n", fEnergySpectrum->GetName());
    fConfig.ene_config.energy_tmp = SampleEnergy(fConfig.ene_config); 

    G4ThreeVector dir = SampleDirection(fConfig.dir_config); 

    if (dynamic_cast<SLArBoxSurfaceVertexGenerator*>(fVtxGen.get())) {
      auto face = static_cast<SLArBoxSurfaceVertexGenerator*>(fVtxGen.get())->GetVertexFace(); 
      const auto& face_normal = geo::BoxFaceNormal[face]; 
      //printf("SLArExternalGeneratorAction: vtx face is %i\n", face);
      //printf("SLArExternalGeneratorAction: face normal is [%.1f, %.1f, %.1f]\n", 
      //face_normal.x(), face_normal.y(), face_normal.z()); 

      while ( dir.dot(face_normal) < 0 ) {
        dir = SLArRandom::SampleRandomDirection(); 
      }
    }

    fConfig.dir_config.direction_tmp = dir;
    //G4cout << "Momentum direction is: " << dir << G4endl; 
    fParticleGun->SetParticleDefinition(fParticleDef); 
    fParticleGun->SetParticleMomentumDirection(fConfig.dir_config.direction_tmp); 
    fParticleGun->SetParticlePosition(vtx_pos); 
    fParticleGun->SetParticleEnergy(fConfig.ene_config.energy_tmp); 
    fParticleGun->SetParticleTime(0); 

    fParticleGun->GeneratePrimaryVertex(ev); 

    auto& record = gen_records.AddRecord( GetGeneratorEnum(), fLabel ); 
    //getchar(); 
  }

  return;
}

void SLArExternalGeneratorAction::Configure() {
  SLArBaseGenerator::Configure( fConfig ); 
  //if (fConfig.dir_config.mode == EDirectionMode::kSunDir) {
    //TH1D* hist_nadir = this->GetFromRootfile<TH1D>(
        //fConfig.dir_config.nadir_hist.filename, 
        //fConfig.dir_config.nadir_hist.objname);

    //fNadirDistribution = std::unique_ptr<TH1D>( std::move(hist_nadir) ); 
  //}

  //if (fConfig.ene_config.mode == EEnergyMode::kExtSpectrum) {
    //TH1D* hist_spectrum = this->GetFromRootfile<TH1D>( 
        //fConfig.ene_config.spectrum_hist.filename , 
        //fConfig.ene_config.spectrum_hist.objname );

    //fEnergySpectrum = std::unique_ptr<TH1D>( std::move(hist_spectrum) ); 
  //}

  fParticleDef = G4ParticleTable::GetParticleTable()->FindParticle( fConfig.ext_primary_particle ); 
}

void SLArExternalGeneratorAction::SourceConfiguration(const rapidjson::Value& config) {

  SLArBaseGenerator::SourceConfiguration(config, fConfig);

  if (config.HasMember("particle")) {
    fConfig.ext_primary_particle = config["particle"].GetString(); 
  } else {
    throw std::invalid_argument("ext gen missing mandatory \"particle\" field.\n"); 
  }

  return;
}

//G4String SLArExternalGeneratorAction::WriteConfig() const {
  //G4String config_str = "";

  //rapidjson::Document d; 
  //d.SetObject(); 
  //rapidjson::StringBuffer buffer;
  //rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  //G4String gen_type = GetGeneratorType(); 

  //d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  //d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  //d.AddMember("particle", rapidjson::StringRef(fConfig.ext_primary_particle.data()), d.GetAllocator()); 
  //d.AddMember("n_particles", fConfig.n_particles, d.GetAllocator()); 

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
