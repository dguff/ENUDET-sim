/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArBaseGenerator.cc
 * @created     : Saturday Mar 30, 2024 22:08:11 CET
 */

#include <cstdio>
#include <memory>
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4RunManager.hh"

#include "SLArBaseGenerator.hh"
#include "SLArBulkVertexGenerator.hh"
#include "SLArBoxSurfaceVertexGenerator.hh"
#include "SLArGPSVertexGenerator.hh"
#include "SLArAnalysisManager.hh"
#include "SLArRunAction.hh"
#include "SLArRandomExtra.hh"
#include "SLArUnit.hpp"

#include "rapidjson/prettywriter.h"


namespace gen {

const rapidjson::Document SLArBaseGenerator::ExtSourceInfo_t::ExportConfig() const {
  rapidjson::Document hist_doc; 
  hist_doc.SetObject(); 

  char buffer[200];
  int len = snprintf(buffer, sizeof(buffer), "%s", filename.data());
  rapidjson::Value jfilename;
  jfilename.SetString(buffer, len, hist_doc.GetAllocator());
  hist_doc.AddMember("filename", jfilename, hist_doc.GetAllocator()); 

  rapidjson::Value jobjname(rapidjson::kStringType);
  len = snprintf(buffer, sizeof(buffer), "%s", objname.data()); 
  jobjname.SetString(buffer, len, hist_doc.GetAllocator()); 
  hist_doc.AddMember("objname", jobjname, hist_doc.GetAllocator()); 

  rapidjson::Value jtype(rapidjson::kStringType);
  len = snprintf(buffer, sizeof(buffer), "%s", type.data()); 
  jtype.SetString(buffer, len, hist_doc.GetAllocator()); 
  hist_doc.AddMember("type", jtype, hist_doc.GetAllocator()); 
  return hist_doc; 
}

void SLArBaseGenerator::SetupVertexGenerator(const rapidjson::Value& config) {
  if (!config.HasMember("type")) {
    throw std::invalid_argument("vertex genrator missing mandatory \"type\" field\n");
  }
  
  G4String type = config["type"].GetString(); 
  EVertexGenerator kGen = getVtxGenIndex( type ); 
  printf("[gen] Building %s vertex generator (type %i)\n", type.data(), kGen);

  switch (kGen) {
    case (EVertexGenerator::kPoint) : 
      {
        fVtxGen = std::make_unique<SLArPointVertexGenerator>();
        try { fVtxGen->Config( config["config"] ); }        
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArPointVertexGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }
    
    case (EVertexGenerator::kGPSPos) : 
      {
        printf("Building GPS vertex generator\n");
        fVtxGen = std::make_unique<SLArGPSVertexGenerator>();
        try { fVtxGen->Config( config["config"] ); }
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArGPSPosVertexGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }

    case (EVertexGenerator::kBulk) : 
      {
        fVtxGen = std::make_unique<SLArBulkVertexGenerator>(); 
        try {fVtxGen->Config( config["config"] );}
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArBulkVertexGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }

    case (EVertexGenerator::kBoxVolSurface) : 
      {
        fVtxGen = std::make_unique<SLArBoxSurfaceVertexGenerator>();
        try {fVtxGen->Config( config["config"] );} 
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArBoxSurfaceVertexGenerator" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }

    default:
      {
        char err_msg[100];
        gen::printVtxGeneratorType();
        snprintf(err_msg, sizeof(err_msg), "Unable to find %s vertex generator among the available options\n", 
            type.data()); 
        std::cerr << err_msg << std::endl;
        exit( EXIT_FAILURE ); 
        break;      
      }
  }

  printf("[gen] %s vtx gen for generator %s: %p\n", 
      fVtxGen->GetType().data(), fLabel.data(), static_cast<void*>(fVtxGen.get())); 
  fVtxGen->Print();
  
  return;
}

void SLArBaseGenerator::CopyConfigurationToString(const rapidjson::Value& config) {
  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

  rapidjson::Value config_copy; config_copy.SetObject(); 
  config_copy.CopyFrom(config, d.GetAllocator()); 

  d.AddMember("config", config_copy, d.GetAllocator());

  d.Accept(writer); 
  fJSONConfigDump = buffer.GetString(); 
}

/*
 *void SLArBaseGenerator::SourceConfiguration(const rapidjson::Value& config, GenConfig_t& local) {
 *  CopyConfigurationToString(config);
 *
 *  if (config.HasMember("n_particles")) {
 *    local.n_particles = config["n_particles"].GetInt();
 *  }
 *
 *  if (config.HasMember("direction")) {
 *    SourceDirectionConfig( config["direction"], local.dir_config );
 *  }
 *
 *  if (config.HasMember("energy")) {
 *    SourceEnergyConfig( config["energy"], local.ene_config );
 *  }
 *
 *  if (config.HasMember("vertex_gen")) {
 *    SetupVertexGenerator( config["vertex_gen"] ); 
 *  }
 *  else {
 *    fVtxGen = std::make_unique<SLArPointVertexGenerator>();
 *  }
 *}
 */

/*
 *void SLArBaseGenerator::SourceConfiguration(const rapidjson::Value& config) {
 *
 *  CopyConfigurationToString(config);
 *
 *  if (config.HasMember("n_particles")) {
 *    fConfig.n_particles = config["n_particles"].GetInt();
 *  }
 *
 *  if (config.HasMember("direction")) {
 *    SourceDirectionConfig( config["direction"] );
 *  }
 *
 *  if (config.HasMember("energy")) {
 *    SourceEnergyConfig( config["energy"] );
 *  }
 *
 *  if (config.HasMember("vertex_gen")) {
 *    SetupVertexGenerator( config["vertex_gen"] ); 
 *  }
 *  else {
 *    fVtxGen = std::make_unique<SLArPointVertexGenerator>();
 *  }
 *}
 */

void SLArBaseGenerator::Configure(const GenConfig_t& config) {

  if (fConfig.dir_config.mode == EDirectionMode::kSunDir) {
    TH1D* hist_nadir = this->GetFromRootfile<TH1D>(
        fConfig.dir_config.nadir_hist.filename, 
        fConfig.dir_config.nadir_hist.objname);

    fNadirDistribution = std::unique_ptr<TH1D>( std::move(hist_nadir) ); 
    printf("SLArBaseGenerator::Configure() Sourcing nadir angle distribution\n"); 
    printf("fNadirDistribution ptr: %p\n", fNadirDistribution.get());
  }
  printf("SLArBaseGeneratorAction::Configure(): Energy mode: %i\n", fConfig.ene_config.mode);
  if (fConfig.ene_config.mode == EEnergyMode::kExtSpectrum) {
    TH1D* hist_spectrum = this->GetFromRootfile<TH1D>( 
        fConfig.ene_config.spectrum_hist.filename , 
        fConfig.ene_config.spectrum_hist.objname );

    fEnergySpectrum = std::unique_ptr<TH1D>( std::move(hist_spectrum) ); 
    printf("SLArBaseGenerator::Configure() Sourcing external energy spectrum\n"); 
    printf("fEnergySpectrum ptr: %p\n", fNadirDistribution.get());
  }

  return;
}

G4ThreeVector SLArBaseGenerator::SampleDirection(DirectionConfig_t& dir_config) {
  SLArRunAction* run_action = (SLArRunAction*)G4RunManager::GetRunManager()->GetUserRunAction(); 
  SLArRandom* slar_random = run_action->GetTRandomInterface(); 

  if (dir_config.mode == EDirectionMode::kFixedDir) {
    dir_config.direction_tmp.set( dir_config.axis.x(), dir_config.axis.y(), dir_config.axis.z() ); 
  }
  else if (dir_config.mode == EDirectionMode::kRandomDir) {
    dir_config.direction_tmp = SLArRandom::SampleRandomDirection();
  }
  else if (dir_config.mode == EDirectionMode::kSunDir) {
    // Select nadir angle
    const double cos_nadir = fNadirDistribution->GetRandom( slar_random->GetEngine().get() );
    const double sin_nadir = sqrt(1-cos_nadir*cos_nadir); 
    const double theta = 102.5*TMath::DegToRad(); 
    const double cos_theta = cos( theta ); 
    const double sin_theta = sin( theta ); 
    const double phi = -slar_random->GetEngine()->Uniform(0, M_PI);
    const double cos_phi = cos( phi ); 
    const double sin_phi = sin( phi );

    dir_config.direction_tmp.set(
      cos_theta*sin_nadir*cos_phi + sin_theta*sin_nadir*sin_phi,
      -cos_nadir, 
      -sin_theta*sin_nadir*cos_phi + cos_theta*sin_nadir*sin_phi
      );
    dir_config.direction_tmp = dir_config.direction_tmp.unit(); 
  }

  return dir_config.direction_tmp; 
}

G4ThreeVector SLArBaseGenerator::SampleDirection() {
  return SampleDirection( fConfig.dir_config ); 
}

G4double SLArBaseGenerator::SampleEnergy(EnergyConfig_t& ene_config) {
  G4double ene = 1.0*CLHEP::MeV;

  SLArRunAction* run_action = (SLArRunAction*)G4RunManager::GetRunManager()->GetUserRunAction(); 
  SLArRandom* slar_random = run_action->GetTRandomInterface(); 

  if (ene_config.mode == EEnergyMode::kFixed) {
    ene_config.energy_tmp = ene_config.energy_value; 
  }
  else if (ene_config.mode == EEnergyMode::kExtSpectrum) {
    ene_config.energy_tmp = fEnergySpectrum->GetRandom( slar_random->GetEngine().get() ); 
  }

  return ene_config.energy_tmp;
}

G4double SLArBaseGenerator::SampleEnergy() {
  return SampleEnergy(fConfig.ene_config); 
}

void SLArBaseGenerator::SourceDirectionConfig(const rapidjson::Value& dir_config) {
  SourceDirectionConfig(dir_config, fConfig.dir_config); 
}

void SLArBaseGenerator::SourceDirectionConfig(const rapidjson::Value& dir_config, DirectionConfig_t& local) {
  if (dir_config.IsObject() == false) {
    fprintf(stderr, "ConfigureDirection ERROR: Direction configuration must be an object\n"); 
    exit(EXIT_FAILURE); 
  }

  if (dir_config.HasMember("mode") == false) {
    fprintf(stderr, "ConfigureDirection ERROR: User must specify direction \"mode\" (fixed, isotropic, sun)\n");
    exit(EXIT_FAILURE);
  }

  G4String dir_mode = dir_config["mode"].GetString(); 

  if (dir_mode == "isotropic") {
    local.mode = EDirectionMode::kRandomDir;
    return;
  } 
  else if (dir_mode == "fixed") {
    local.mode = EDirectionMode::kFixedDir;
    local.axis.set(0, 0, 1); 
    if (dir_config.HasMember("axis")) {
      assert( dir_config["axis"].GetArray().Size() == 3 ); 
      G4double dir[3] = {0}; 
      G4int idir = 0; 
      for (const auto& p : dir_config["axis"].GetArray()) {
        dir[idir] = p.GetDouble(); idir++; 
      }
      local.axis.set(dir[0], dir[1], dir[2]); 
    }
  } 
  else if (dir_mode == "sun") {
    local.mode = EDirectionMode::kSunDir;
    //if (dir_config.HasMember("nadir_histogram") == false) {
    //fprintf(stderr, "ConfigureDirection ERROR: Direction mode set to \"sun\" but no histogram for nadir given\n"); 
    //exit(EXIT_FAILURE); 
    //}
    if (dir_config.HasMember("nadir_histogram")) {
      local.nadir_hist.Configure( dir_config["nadir_histogram"] ); 
    }
  }
}

const rapidjson::Document SLArBaseGenerator::ExportDirectionConfig() const {
  rapidjson::Document doc;
  doc.SetObject(); 

  const auto& dconfig = fConfig.dir_config; 
  char buffer[100]; 
  int len = 1; 
  rapidjson::Value jmode; 

  if (dconfig.mode == EDirectionMode::kFixedDir) {
    len = snprintf(buffer, sizeof(buffer), "fixed"); 
    jmode.SetString(buffer, len, doc.GetAllocator()); 
    doc.AddMember("mode", jmode, doc.GetAllocator()); 
    rapidjson::Value jaxis(rapidjson::kArrayType); 
    jaxis.PushBack( dconfig.axis.x(), doc.GetAllocator() ); 
    jaxis.PushBack( dconfig.axis.y(), doc.GetAllocator() ); 
    jaxis.PushBack( dconfig.axis.z(), doc.GetAllocator() ); 
    doc.AddMember("axis", jaxis, doc.GetAllocator()); 
  }
  else if (dconfig.mode == EDirectionMode::kRandomDir) {
    len = snprintf(buffer, sizeof(buffer), "isotropic"); 
    jmode.SetString(buffer, len, doc.GetAllocator()); 
    doc.AddMember("mode", jmode, doc.GetAllocator()); 
  }
  else if (dconfig.mode == EDirectionMode::kSunDir) {
    len = snprintf(buffer, sizeof(buffer), "sun_direction"); 
    jmode.SetString(buffer, len, doc.GetAllocator()); 
    doc.AddMember("mode", jmode, doc.GetAllocator()); 

    if (dconfig.nadir_hist.filename.empty() == false) {
      const rapidjson::Document nadir_hist_doc = dconfig.nadir_hist.ExportConfig(); 
      rapidjson::Value nadir_hist_info; 
      nadir_hist_info.CopyFrom(nadir_hist_doc, doc.GetAllocator()); 
      doc.AddMember("nadir_hist", nadir_hist_info, doc.GetAllocator());
    }
  }

  return doc;
}

void SLArBaseGenerator::SourceEnergyConfig(const rapidjson::Value& ene_config) {
  SourceEnergyConfig( ene_config, fConfig.ene_config ); 
  return;
}

void SLArBaseGenerator::SourceEnergyConfig(const rapidjson::Value& ene_config, EnergyConfig_t& local) {
  if (ene_config.IsObject() == false) {
    fprintf(stderr, "ConfigureEnergy ERROR: Energy configuration must be an object\n"); 
    exit(EXIT_FAILURE); 
  }

  if (ene_config.HasMember("mode") == false) {
    fprintf(stderr, "ConfigureEnergy ERROR: User must specify direction \"mode\" (monoenergetic, custom, spectrum)\n");
    exit(EXIT_FAILURE);
  }

  G4String ene_mode = ene_config["mode"].GetString(); 

  if (ene_mode == "monoenergetic" || ene_mode == "mono") {
    local.mode = EEnergyMode::kFixed;
    local.energy_distribution_label = "monoenergetic"; 

    if ( ene_config.HasMember("value") == false ) {
      fprintf(stderr, "ConfigureEnergy ERROR: monoenergetic particle set but no energy value provided\n"); 
      exit(EXIT_FAILURE); 
    }
    local.energy_value = unit::ParseJsonVal( ene_config["value"] ); 
    printf("set energy to %g MeV\n", local.energy_value); 
  }
  else if (ene_mode == "custom") {
    local.mode = EEnergyMode::kCustom; 
    if ( ene_config.HasMember("label") == false ) {
      fprintf(stderr, "ConfigureEnergy ERROR: custom energy spectrum set but no label given\n");
      exit(EXIT_FAILURE); 
    }
    local.energy_distribution_label = ene_config["label"].GetString(); 

    if (ene_config.HasMember("Emin")) {
      local.energy_min = ene_config["Emin"].GetDouble();
    }
    if (ene_config.HasMember("Emax")) {
      local.energy_max = ene_config["Emax"].GetDouble();
    }
    if (ene_config.HasMember("temperature")) {
      local.temperature = ene_config["temperature"].GetDouble();
    }
    if (ene_config.HasMember("eta")) {
      local.eta = ene_config["eta"].GetDouble();
    }
    if (ene_config.HasMember("Emean")) {
      local.energy_mean = ene_config["Emean"].GetDouble();
    }
    if (ene_config.HasMember("beta")) {
      local.beta = ene_config["beta"].GetDouble();
    }
    if (ene_config.HasMember("E_bin_lefts")) {
      if (ene_config["E_bin_lefts"].IsArray() == false) {
        fprintf(stderr, "SourceEnergyConfiguration ERROR: \"E_bin_lefts\" MUST be an array\n"); 
        exit(EXIT_FAILURE);
      }

      for (const auto& jb : ene_config["E_bin_lefts"].GetArray()) {
        local.energy_bin_left.push_back( jb.GetDouble() ); 
      }
    }
    if (ene_config.HasMember("energies")) {
      if (ene_config["energies"].IsArray() == false) {
        fprintf(stderr, "SourceEnergyConfiguration ERROR: \"energies\" MUST be an array\n"); 
        exit(EXIT_FAILURE);
      }

      for (const auto& jb : ene_config["energies"].GetArray()) {
        local.energies.push_back( jb.GetDouble() ); 
      }
    }
    if (ene_config.HasMember("weights")) {
      if (ene_config["weights"].IsArray() == false) {
        fprintf(stderr, "SourceEnergyConfiguration ERROR: \"weights\" MUST be an array\n"); 
        exit(EXIT_FAILURE);
      }

      for (const auto& jb : ene_config["weights"].GetArray()) {
        local.weights.push_back( jb.GetDouble() ); 
      }
    }
    if (ene_config.HasMember("prob_densities")) {
      if (ene_config["prob_densities"].IsArray() == false) {
        fprintf(stderr, "SourceEnergyConfiguration ERROR: \"prob_densities\" MUST be an array\n");
        exit(EXIT_FAILURE);
      }

      for (const auto& p : ene_config["prob_densities"].GetArray()) {
        local.prob_densities.push_back( p.GetDouble() );
      }
    }
    if (ene_config.HasMember("interpolation_rule")) {
      local.interpolation_rule = ene_config["interpolation_rule"].GetString(); 
    }
  }
  else if (ene_mode == "ext_spectrum") {
    local.mode = EEnergyMode::kExtSpectrum; 
    local.energy_distribution_label = "th1"; 
    if (ene_config.HasMember("energy_distribution") == false) {
      fprintf(stderr, "ConfigureEnergy ERROR: energy spectrum set but no file or object info given\n");
      exit(EXIT_FAILURE); 
    }
    local.spectrum_hist.Configure( ene_config["energy_distribution"] );
  }

  return;
}

const rapidjson::Document SLArBaseGenerator::ExportEnergyConfig() const {
  rapidjson::Document doc; 
  doc.SetObject(); 

  const auto& econfig = fConfig.ene_config; 
  char buffer[100]; 
  int len = 1; 
  rapidjson::Value jmode; 

  if (econfig.mode == EEnergyMode::kFixed) {
    len = snprintf(buffer, sizeof(buffer), "fixed"); 
    jmode.SetString(buffer, len, doc.GetAllocator()); 
    doc.AddMember("mode", jmode, doc.GetAllocator()); 
    doc.AddMember("energy_MeV", econfig.energy_value, doc.GetAllocator()); 
  }
  else if (econfig.mode == EEnergyMode::kCustom) {
    len = snprintf(buffer, sizeof(buffer), "custom"); 
    jmode.SetString(buffer, len, doc.GetAllocator()); 
    doc.AddMember("mode", jmode, doc.GetAllocator()); 
    doc.AddMember("label", rapidjson::StringRef(econfig.energy_distribution_label.data()), doc.GetAllocator()); 
  }
  else if (econfig.mode == EEnergyMode::kExtSpectrum) {
    len = snprintf(buffer, sizeof(buffer), "spectrum"); 
    jmode.SetString(buffer, len, doc.GetAllocator()); 
    doc.AddMember("mode", jmode, doc.GetAllocator()); 
    const rapidjson::Document spectrum_doc = ExportEnergyConfig(); 
    rapidjson::Value spectrum_info; 
    spectrum_info.CopyFrom(spectrum_doc, doc.GetAllocator()); 
    doc.AddMember("spectrum_info", spectrum_info, doc.GetAllocator()); 
  }

  return doc; 
}

void SLArBaseGenerator::RegisterPrimaries(const G4Event* anEvent, const G4int firstVertex) {

  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
  G4IonTable* ionTable = G4IonTable::GetIonTable(); 

  G4int total_vertices = anEvent->GetNumberOfPrimaryVertex(); 

  if (fVerbose) {
    printf("[gen] %s primary generator action produced %i vertex(ices)\n", 
        fLabel.data(), total_vertices - firstVertex); 
  }
  for (int i=firstVertex; i<total_vertices; i++) {
    //std::unique_ptr<SLArMCPrimaryInfoUniquePtr> tc_primary = std::make_unique<SLArMCPrimaryInfoUniquePtr>();
    const G4PrimaryVertex* primary_vertex = anEvent->GetPrimaryVertex(i); 
    const G4int np = primary_vertex->GetNumberOfParticle(); 
    if (fVerbose) {
      printf("vertex %i has %i particles at t = %g\n", i, np, 
          primary_vertex->GetT0()); 
    }

    for (int ip = 0; ip<np; ip++) {
      auto particle = primary_vertex->GetPrimary(ip); 
      G4String name = ""; 
      SLArMCPrimaryInfo tc_primary;

      if (!particle->GetParticleDefinition()) {
        tc_primary.SetID  (particle->GetPDGcode()); 
        name = ionTable->GetIon( particle->GetPDGcode() )->GetParticleName(); 
        tc_primary.SetName(name);
        tc_primary.SetTitle(name + " [" + particle->GetTrackID() +"]"); 
      } else {
        tc_primary.SetID  (particle->GetPDGcode());
        name = particle->GetParticleDefinition()->GetParticleName(); 
        tc_primary.SetName(name);
        tc_primary.SetTitle(name + " [" + particle->GetTrackID() +"]"); 
      }

      tc_primary.SetTrackID(particle->GetTrackID());
      tc_primary.SetPosition(
          primary_vertex->GetX0(),
          primary_vertex->GetY0(), 
          primary_vertex->GetZ0());
      tc_primary.SetMomentum(
          particle->GetPx(), particle->GetPy(), particle->GetPz(), 
          particle->GetKineticEnergy());
      tc_primary.SetTime(primary_vertex->GetT0()); 
      tc_primary.SetGeneratorLabel( fLabel.data() ); 

#ifdef SLAR_DEBUG
      printf("Adding particle to primary output list\n"); 
      tc_primary.PrintParticle(); 
      //getchar();
#endif
      SLArAnaMgr->GetEvent().RegisterPrimary( tc_primary );
    }
  }

}

void SLArBaseGenerator::SetGenRecord(SLArGenRecord& record, const GenConfig_t& config) const {
  record.GetGenCode() = this->GetGeneratorEnum(); 
  record.GetGenLabel() = this->GetLabel(); 

  auto& status = record.GetGenStatus(); 
  status.resize(4, 0.0); 
  status.at(0) = config.ene_config.energy_tmp;
  status.at(1) = config.dir_config.direction_tmp.x(); 
  status.at(2) = config.dir_config.direction_tmp.y(); 
  status.at(3) = config.dir_config.direction_tmp.z(); 

  return; 
}

G4String SLArBaseGenerator::WriteConfig() const {
  G4String config_str = "";

  rapidjson::Document vtx_json = fVtxGen->ExportConfig();
  rapidjson::Document doc_gen; 
  doc_gen.Parse( fJSONConfigDump.data() );

  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  G4String gen_type = GetGeneratorType(); 

  d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  rapidjson::Value gen_config;
  gen_config.CopyFrom(doc_gen, d.GetAllocator()); 
  d.AddMember("config", gen_config.GetObj(), d.GetAllocator()); 
  rapidjson::Value vtx_config;
  vtx_config.CopyFrom(vtx_json, d.GetAllocator());
  d.AddMember("vertex_generator", vtx_config, d.GetAllocator());
  d.Accept(writer);
  config_str = buffer.GetString();

  return config_str;
}

//template TH1D*   SLArBaseGenerator::GetFromRootfile<TH1D>  (const G4String& filename, const G4String& key); 
//template TH1F*   SLArBaseGenerator::GetFromRootfile<TH1F>  (const G4String& filename, const G4String& key); 
//template TH2D*   SLArBaseGenerator::GetFromRootfile<TH2D>  (const G4String& filename, const G4String& key); 
//template TH2F*   SLArBaseGenerator::GetFromRootfile<TH2F>  (const G4String& filename, const G4String& key); 
template<> TGraph* SLArBaseGenerator::GetFromRootfile<TGraph>(const G4String& filename, const G4String& key) {
  TFile* rootFile = TFile::Open(filename, "READ");
  if (!rootFile || rootFile->IsZombie()) {
    std::fprintf(stderr, "GetFromRootfile ERROR: Cannot open file %s", filename.data()); 
    exit(EXIT_FAILURE);
  }
  TGraph* obj = dynamic_cast<TGraph*>(rootFile->Get(key));
  if (!obj) {
    rootFile->Close();
    std::fprintf(stderr, "GetFromRoofile ERROR: Unable to find object with key %s", key.data());
    exit(EXIT_FAILURE);
  }
  rootFile->Close(); 
  return std::move(obj);
}

template TH1D*   SLArBaseGenerator::GetFromRootfile<TH1D>  (const rapidjson::Value& file_and_key); 
template TH1F*   SLArBaseGenerator::GetFromRootfile<TH1F>  (const rapidjson::Value& file_and_key); 
template TH2D*   SLArBaseGenerator::GetFromRootfile<TH2D>  (const rapidjson::Value& file_and_key); 
template TH2F*   SLArBaseGenerator::GetFromRootfile<TH2F>  (const rapidjson::Value& file_and_key); 
template<> TGraph* SLArBaseGenerator::GetFromRootfile<TGraph>(const rapidjson::Value& file_and_key) {
  if (!file_and_key.HasMember("file") || !file_and_key.HasMember("key")) {
    std::fprintf(stderr, "GetFromRootfile Error: mandatory 'file' or 'key' field missing."); 
    exit(EXIT_FAILURE);
  } 
  G4String filename = file_and_key["file"].GetString();
  G4String key = file_and_key["key"].GetString();
  TGraph* obj = GetFromRootfile<TGraph>(filename, key); 
  return std::move(obj);
} 

}

