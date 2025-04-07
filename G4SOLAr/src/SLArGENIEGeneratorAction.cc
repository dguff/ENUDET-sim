#include <SLArGENIEGeneratorAction.hh>
#include <SLArAnalysisManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4ParticleTable.hh>
#include <G4IonTable.hh>
#include <G4ParticleDefinition.hh>
#include <G4String.hh>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <cstdio>

namespace gen {

//***********************************************************************
//************************** CONSTRUCTORS *******************************

SLArGENIEGeneratorAction::SLArGENIEGeneratorAction(const G4String label) 
  : SLArBaseGenerator(label), m_gtree(0)
{}

SLArGENIEGeneratorAction::SLArGENIEGeneratorAction(const G4String label, const G4String genie_file)
  : SLArBaseGenerator(label), m_gtree(0)
{}

SLArGENIEGeneratorAction::~SLArGENIEGeneratorAction()
{}

void SLArGENIEGeneratorAction::SourceConfiguration(const rapidjson::Value& config) {
  assert( config.HasMember("genie_tree") ); 
  CopyConfigurationToString(config);

  fConfig.tree_info.Configure(config["genie_tree"]); 

  if (config.HasMember("tree_first_entry")) {
    fConfig.tree_first_entry = config["tree_first_entry"].GetInt(); 
  }
  if (config.HasMember("vertex_gen")) {
    SetupVertexGenerator( config["vertex_gen"] ); 
  }
  else {
    fVtxGen = std::make_unique<SLArPointVertexGenerator>();
  }
  return;
}

void SLArGENIEGeneratorAction::Configure() {
  TFile *GENIEInput = TFile::Open(fConfig.tree_info.filename);

  m_gtree = (TTree*) GENIEInput->Get(fConfig.tree_info.objname);

  m_gtree->SetBranchAddress("EvtNum",&gVar.EvtNum);
  m_gtree->SetBranchAddress("StdHepN",&gVar.nPart);
  m_gtree->SetBranchAddress("StdHepPdg",&gVar.pdg);
  m_gtree->SetBranchAddress("StdHepStatus",&gVar.status);
  m_gtree->SetBranchAddress("StdHepP4",&gVar.p4);
  m_gtree->SetBranchAddress("StdHepX4",&gVar.x4);
  m_gtree->SetBranchAddress("EvtVtx",&gVar.vtx);
  m_gtree->SetBranchAddress("EnuOriginTime",&gVar.t);
}

G4String SLArGENIEGeneratorAction::WriteConfig() const {
  G4String config_str = "";

  rapidjson::Document d; 
  d.SetObject();
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  G4String gen_type = GetGeneratorType();

  d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 

  rapidjson::Document dtree = fConfig.tree_info.ExportConfig(); 
  rapidjson::Value jtree; jtree.CopyFrom(dtree, d.GetAllocator()); 
  d.AddMember("genie_tree_info", jtree, d.GetAllocator());
  d.AddMember("tree_first_entry", fConfig.tree_first_entry, d.GetAllocator()); 

  d.Accept(writer);
  config_str = buffer.GetString();
  return config_str;
}


//***********************************************************************

//***********************************************************************
//************************** EVENT FUNCTIONS ****************************

void SLArGENIEGeneratorAction::GeneratePrimaries(G4Event *ev)
{
  auto& gen_records = SLArAnalysisManager::Instance()->GetGenRecords();
  
  int evtNum = ev->GetEventID() + fConfig.tree_first_entry;
  m_gtree->GetEntry(evtNum);
  std::cout << "   GENIE TTree event selection: " << evtNum << std::endl;

  size_t particle_idx = 0; // Think this can be done in a better way

  G4ThreeVector vtx(0.,0.,0.);
  vtx.set(gVar.vtx[0]*CLHEP::m, gVar.vtx[1]*CLHEP::m, gVar.vtx[2]*CLHEP::m);
  std::vector<G4PrimaryVertex*> primary_vertices;

  for (int i=0; i<gVar.nPart; i++){

    /*G4bool pdg_valid = SLArBaseGenerator::PDGCodeIsValid( gVar.pdg[i] ); 
    if ( pdg_valid == false ) {
      fprintf(stdout, "SLArGENIEGeneratorAction::GeneratePrimaries() WARNING. Event %i contains particle with PDG code %i, which is not valid.\n", 
          evtNum, gVar.pdg[i]); 
      continue;
    }*/

    if (gVar.pdg[i] >= 2000000000) continue;
    
    if (gVar.status[i] == 1){ // 0 - incoming; 1 - outgoing; x - virtual

      G4PrimaryParticle *particle = new G4PrimaryParticle(gVar.pdg[i],
          gVar.p4[i][0]*CLHEP::GeV,
          gVar.p4[i][1]*CLHEP::GeV,
          gVar.p4[i][2]*CLHEP::GeV,
          gVar.p4[i][3]*CLHEP::GeV);
      auto vertex = new G4PrimaryVertex(vtx, gVar.t); // Not sure which is better here
      //    auto vertex = new G4PrimaryVertex(vtx, gVar.vtx[3]); 
      vertex->SetPrimary(particle);
      primary_vertices.push_back(vertex);

      // store primary neutrino info in gen record
      auto record = gen_records.AddRecord( GetGeneratorEnum(), fLabel );
      // TODO: fill record with useful variables
      particle_idx++;
    }
  }
  
  for (const auto& vertex : primary_vertices)
    ev->AddPrimaryVertex(vertex);

}

//***********************************************************************


//***********************************************************************
//***********************************************************************

void SLArGENIEGeneratorAction::SetGENIEEvntExt(G4int evntID)
{
  fConfig.tree_first_entry = evntID;
}


//***********************************************************************
}
