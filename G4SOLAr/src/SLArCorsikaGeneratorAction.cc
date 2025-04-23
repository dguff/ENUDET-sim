/* ================ Corsika Gen ================= *
 * Created: 14-06-2024                            *
 * Author:  Jordan McElwee                        *
 * Email: mcelwee@lp2ib.in2p3.fr                  *
 *                                                *
 * Corsika generator action to apply corsika      *
 * events to each event.                          *
 *                                                *
 * Changelog:                                     *
 * ============================================== */


#include "SLArCorsikaGeneratorAction.hh"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

namespace gen {

  // ***************************************************************************
  // ***** CONSTRUCTORS ********************************************************

  // ----- Constructors -----

  // - - - - - - - - - - - - - - - - -
  SLArCorsikaGeneratorAction::SLArCorsikaGeneratorAction(const G4String label)
    : SLArBaseGenerator(label)
  {}
  // - - - - - - - - - - - - - - - - -


  // - - - - - - - - - - - - - - - - -
  // Destructor
  SLArCorsikaGeneratorAction::~SLArCorsikaGeneratorAction()
  {}
  // - - - - - - - - - - - - - - - - -

  // ***************************************************************************


  // ***************************************************************************
  // ***** ENUDET BASICS *******************************************************

  // - - - - - - - - - - - - - - - - -
  /*
    Reads the Corsika input file and passes 
    them to the Corsika config. 
  */
  void SLArCorsikaGeneratorAction::SourceConfiguration(const rapidjson::Value &config)
  {
    assert( config.HasMember("corsika_db_dir") );
    fConfig.corsika_db_dir = config["corsika_db_dir"].GetString();

    // --- Detector measurements ---
    if ( config.HasMember("det_center") ) {
      auto det_center_array = config["det_center"].GetArray();
      for (int i=0; i < 3; i++)
	fConfig.det_center[i] = det_center_array[i].GetDouble();
    }
    if ( config.HasMember("det_size") ) {
      auto det_size_array = config["det_size"].GetArray();
      for (int i=0; i < 3; i++)
	fConfig.det_size[i] = det_size_array[i].GetDouble();
    }

    // --- Corsika inputs ---
    if ( config.HasMember("gen_buffer") ) {
      auto gen_buffer_array = config["gen_buffer"].GetArray();
      for (int i=0; i < 2; i++)
	fConfig.gen_buffer[i] = gen_buffer_array[i].GetDouble();
    }
    if ( config.HasMember("gen_offset") )
      fConfig.gen_offset = config["gen_offset"].GetDouble();

    if ( config.HasMember("gen_E") ) {
      auto gen_E_array = config["gen_E"].GetArray();
      for (int i=0; i < 2; i++)
	fConfig.gen_E[i] = gen_E_array[i].GetDouble();
    }
    if ( config.HasMember("gen_dT") )
      fConfig.gen_dT = config["gen_dT"].GetDouble();    
    if ( config.HasMember("gen_T_off") )
      fConfig.gen_T_off = config["gen_T_off"].GetDouble();

    if ( config.HasMember("primaries") ) {
      fConfig.primaries.clear();
      const auto &primaries = config["primaries"];
      if ( primaries.IsArray() )
	for ( const auto &prim : primaries.GetArray() ) {
	  const G4String primary_name = prim.GetString();
	  fConfig.primaries.push_back(primary_name);
	}
      else if (primaries.IsString()) {
	const G4String primary_name = primaries.GetString();
	fConfig.primaries.push_back(primary_name);
      }
    }
      
    
  }
  // - - - - - - - - - - - - - - - - -


  // - - - - - - - - - - - - - - - - -

  void SLArCorsikaGeneratorAction::Configure()
  {
    const auto *detector = dynamic_cast<const SLArDetectorConstruction*>
      (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    const auto *cryostat = dynamic_cast<const SLArBaseDetModule*>(detector->GetLArTargetVolume());
    const auto *box = dynamic_cast<const G4Box*>(cryostat->GetModSV());
    if (!box) {
      G4cerr << "Error: G4Box is null or the solid is not a G4Box!" << G4endl;
      return;
    }

    if ( fConfig.det_size[0] < 0)
      for (int i=0; i < 3; i++) {
	fConfig.det_size[0] = 2*box->GetXHalfLength()/1000;
	fConfig.det_size[1] = 2*box->GetZHalfLength()/1000;
	fConfig.det_size[2] = 2*box->GetYHalfLength()/1000;
      }

    
  }
  // - - - - - - - - - - - - - - - - -

  // - - - - - - - - - - - - - - - - -
  /*
    Parses the configuration data from the JSON
    file to the configuration for the generation.
  */
  G4String SLArCorsikaGeneratorAction::WriteConfig() const
  {
    G4String config_str = "";

    rapidjson::Document d;
    d.SetObject();
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    G4String gen_type = GetGeneratorType();

    d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator());
    d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator());
    d.AddMember("corsika_db_dir", rapidjson::StringRef(fConfig.corsika_db_dir.data()), d.GetAllocator());
    
    // Detector specific
    rapidjson::Value det_center_array(rapidjson::kArrayType);
    for (G4double value : fConfig.det_center)
      det_center_array.PushBack(value, d.GetAllocator());
    d.AddMember("det_center", det_center_array, d.GetAllocator());

    rapidjson::Value det_size_array(rapidjson::kArrayType);
    for (G4double value : fConfig.det_size)
      det_size_array.PushBack(value, d.GetAllocator());
    d.AddMember("det_size", det_size_array, d.GetAllocator());
    

    // Generator specific
    rapidjson::Value gen_buffer_array(rapidjson::kArrayType);
    for (G4double value : fConfig.gen_buffer)
      gen_buffer_array.PushBack(value, d.GetAllocator());
    d.AddMember("gen_buffer", gen_buffer_array, d.GetAllocator());
    
    d.AddMember("gen_offset", fConfig.gen_offset, d.GetAllocator());
    
    rapidjson::Value gen_E_array(rapidjson::kArrayType);
    for (G4double value : fConfig.gen_E)
      gen_E_array.PushBack(value, d.GetAllocator());
    d.AddMember("gen_E",  gen_E_array,    d.GetAllocator());
    
    d.AddMember("gen_dT", fConfig.gen_dT, d.GetAllocator());
    d.AddMember("gen_T_off", fConfig.gen_T_off, d.GetAllocator());


    // Particle stuff
    rapidjson::Value jprimaries(rapidjson::kArrayType);
    for (const auto & prim : fConfig.primaries)
      jprimaries.PushBack( rapidjson::StringRef(prim.c_str()), d.GetAllocator());
    d.AddMember("primaries", jprimaries, d.GetAllocator());
    
    
    d.Accept(writer);
    config_str = buffer.GetString();
    return config_str;
  }
  // - - - - - - - - - - - - - - - - -


  // - - - - - - - - - - - - - - - - -
  /*
    Main function for generating the primaries
    within the detector geometry and giving them
    to the rest of the simulation. 
  */
  void SLArCorsikaGeneratorAction::GeneratePrimaries(G4Event *ev)
  {

    // Read the database using the reader class 
    //DBReader *corsDB = new DBReader((fConfig.corsika_db_dir+"/cosmic_db_H_50_10000000.root").c_str());
    
    //Setup a detector level to read from 
    EDetector *pdMuon = new EDetector(fConfig.det_size, fConfig.det_center);
  
    // Set the spill time for all EHandler objects
    EShower::SetSpillT(fConfig.gen_dT);

    //    std::vector<std::string> primaries = {"H"};
    //    for (auto value : fConfig.primaries)
    //      std::cout << value << std::endl;
    
    
    // Create a handler for the shower
    EShower showerHandler(fConfig.corsika_db_dir, pdMuon, fConfig.gen_E, fConfig.primaries);
    showerHandler.SetBuffer(fConfig.gen_buffer);
    showerHandler.SetOffset(fConfig.gen_offset);
    showerHandler.NShowers();
    
    // Create a handler for the particle 
    EParticle particleHandler(fConfig.corsika_db_dir, pdMuon, fConfig.primaries);

    for (int i=0; i < fConfig.primaries.size(); i++) {
    
      for ( int shower : showerHandler.GetShowers(i) ) {
	showerHandler.Process(shower, i);
	particleHandler.Process(shower, &showerHandler, i);
      }

      particleHandler.ResetParticle();

    }
    
    const std::vector<EParticle::Particle> &particles = particleHandler.GetParticles();
  
    G4ThreeVector vtx(0., 0., 0.);
    std::vector<G4PrimaryVertex*> primary_vertices;
    for (const auto &part : particles) {
      
      G4PrimaryParticle *incident_part = new G4PrimaryParticle(part.m_pdg,
                     part.m_mom[0]*1E3,
                     -part.m_mom[2]*1E3,
                     part.m_mom[1]*1E3,
                     part.m_eK*1E3);
      vtx.set(part.m_vtx[0]*1E3, (fConfig.gen_offset)*1E3, part.m_vtx[1]*1E3);
      auto vertex = new G4PrimaryVertex(vtx, fConfig.gen_T_off + part.m_t);
      vertex->SetPrimary(incident_part);
      primary_vertices.push_back(vertex);
    }

    for (const auto& vertex : primary_vertices) 
      ev->AddPrimaryVertex(vertex);
    
  }
  // - - - - - - - - - - - - - - - - -
  
  // ***************************************************************************




}
