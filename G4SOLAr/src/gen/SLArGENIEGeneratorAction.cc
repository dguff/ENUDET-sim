#include "SLArGENIEGeneratorAction.hh"
#include "SLArAnalysisManager.hh"
#include "SLArRunAction.hh"
#include "SLArPointVertexGenerator.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4String.hh"
#include "G4RunManager.hh"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include <CLHEP/Units/SystemOfUnits.h>
#include <cstdio>

namespace gen {

const std::unordered_map<std::string_view, GenieEventEncoding_t::EPType>
GenieEventEncoding_t::interaction_types = {
    {"Weak", EPType::kWeak},
    {"EM", EPType::kEM},
    {"Hadronic", EPType::kHadronic}
};

const std::unordered_map<std::string_view, GenieEventEncoding_t::ECurrent>
GenieEventEncoding_t::interaction_currents = {
    {"CC", ECurrent::kCC},
    {"NC", ECurrent::kNC},
    {"CC+NC+interference", ECurrent::kInterference}
};

const std::unordered_map<std::string_view, GenieEventEncoding_t::EProc>
GenieEventEncoding_t::interaction_procs = {
    {"QES", EProc::kQES},
    {"RES", EProc::kRES},
    {"DIS", EProc::kDIS},
    {"NuEEL", EProc::kNuEEL},
    {"COH", EProc::kCOH},
    {"MEC", EProc::kMEC}
};

const std::string GenieEventEncoding_t::key_nu = "nu";
const std::string GenieEventEncoding_t::key_tgt = "tgt";
const std::string GenieEventEncoding_t::key_N = "N";
const std::string GenieEventEncoding_t::key_proc = "proc";
const std::string GenieEventEncoding_t::key_res = "res";
const std::string GenieEventEncoding_t::key_q = "q";
const std::string GenieEventEncoding_t::key_charm = "charm";
const std::string GenieEventEncoding_t::key_strange = "strange";

TPRegexp GenieEventEncoding_t::fType_rgx("^([[:alpha:]]+)\\[(.*)\\],([[:alpha:]]+)(;[[:alpha:]]:([[:alnum]]+))?$");

void GenieEventEncoding_t::parse_process(const TString& proc_str) 
{
  TObjArray* matches = fType_rgx.MatchS(proc_str);
  if (matches->GetEntries() < 4) {
    G4Exception("GenieEventEncoding_t::parse_process",
        "InvalidProcessString", FatalException, "Process string format is invalid.");
    return;
  }
  else {
    TString interaction_type_str = ((TObjString*)matches->At(1))->GetString();
    TString interaction_current_str = ((TObjString*)matches->At(2))->GetString();
    TString interaction_proc_str = ((TObjString*)matches->At(3))->GetString();

    _type = static_cast<int>( GetInteractionType(interaction_type_str.Data()) );
    _current = static_cast<int>( GetInteractionCurrent(interaction_current_str.Data()) );
    _proc = static_cast<int>( GetInteractionProc(interaction_proc_str.Data()) );
  }

  delete matches;
}

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

/**
 * @brief Configures the SLArGENIEGeneratorAction source from a JSON configuration.
 *
 * This function parses the provided RapidJSON configuration object to set up the generator action.
 * It expects the configuration to contain at least a "genie_tree" field, and optionally a "selection" array
 * of filter criteria, "tree_first_entry" and "tree_last_entry" integers, and a "vertex_gen" object for vertex generation.
 *
 * The "selection" array, if present, should contain objects specifying filtering criteria such as neutrino PDG codes,
 * target nuclei/nucleons, interaction current/process, and product PDG codes. Each filter is added to the generator's
 * configuration.
 *
 * @param config The RapidJSON value containing the configuration for the generator action.
 *
 * @throws G4Exception If required fields are missing or if the configuration is malformed.
 */
void SLArGENIEGeneratorAction::SourceConfiguration(const rapidjson::Value& config) {
  assert( config.HasMember("genie_tree") ); 
  CopyConfigurationToString(config);

  fConfig.tree_info.Configure(config["genie_tree"]); 

  if ( config.HasMember("selection") ) {
    const auto& selection = config["selection"];

    if ( selection.IsArray() == false ) {
      G4Exception("SLArGENIEGeneratorAction::SourceConfiguration",
          "InvalidConfiguration",
          FatalException,
          "The 'selection' field must be an array of selection criteria.");
    }

    for ( const auto& sel : selection.GetArray() ) {
      if ( sel.IsObject() == false ) {
        G4Exception("SLArGENIEGeneratorAction::SourceConfiguration",
            "InvalidConfiguration",
            FatalException,
            "Each selection criterion must be a JSON object.");
      }
      GENIEConfig_t::Filter_t filter; 

      if ( sel.HasMember("nu_pdg") ) {
        const auto& nu_pdgs = sel["nu_pdg"];
        for ( rapidjson::SizeType i = 0; i < nu_pdgs.Size(); i++ ) {
          filter.nu_pdgs.push_back( nu_pdgs[i].GetInt() ); 
        }
      }

      if ( sel.HasMember("target_nucleus") ) {
        const auto& target_nuclei = sel["target_nucleus"];
        for ( rapidjson::SizeType i = 0; i < target_nuclei.Size(); i++ ) {
          filter.target_nuclei.push_back( target_nuclei[i].GetInt() ); 
        }
      }

      if ( sel.HasMember("target_nucleon") ) {
        const auto& target_nucleons = sel["target_nucleon"];
        for ( rapidjson::SizeType i = 0; i < target_nucleons.Size(); i++ ) {
          filter.target_nucleons.push_back( target_nucleons[i].GetInt() ); 
        }
      }

      if ( sel.HasMember("interaction_current") ) {
        const auto& currents = sel["interaction_current"];
        for ( rapidjson::SizeType i = 0; i < currents.Size(); i++ ) {
          std::string cur_str = currents[i].GetString(); 
          filter.currents.push_back( GenieEventEncoding_t::GetInteractionCurrent(cur_str) ); 
        }
      }
      
      if ( sel.HasMember("interaction_process") ) {
        const auto& processes = sel["interaction_process"];
        for ( rapidjson::SizeType i = 0; i < processes.Size(); i++ ) {
          std::string proc_str = processes[i].GetString(); 
          filter.processes.push_back( GenieEventEncoding_t::GetInteractionProc(proc_str) ); 
        }
      }

      if ( sel.HasMember("product_pdg") ) {
        const auto& products_pdg = sel["product_pdg"];
        for ( rapidjson::SizeType i = 0; i < products_pdg.Size(); i++ ) {
          filter.products_pdg.push_back( products_pdg[i].GetInt() ); 
        }
      }
      
      fConfig.filters.push_back( filter );
    }
  }

  if (config.HasMember("tree_first_entry")) {
    fConfig.tree_first_entry = config["tree_first_entry"].GetInt(); 
  }
  if (config.HasMember("tree_last_entry")) {
    fConfig.tree_last_entry = config["tree_last_entry"].GetInt(); 
  }
  if (config.HasMember("vertex_gen")) {
    SetupVertexGenerator( config["vertex_gen"] ); 
  }
  else {
    fVtxGen = std::make_unique<vertex::SLArPointVertexGenerator>();
  }
  return;
}

void SLArGENIEGeneratorAction::Configure() {
  TFile *GENIEInput = TFile::Open(fConfig.tree_info.filename.data());
  m_gtree = (TTree*) GENIEInput->Get(fConfig.tree_info.objname.data());

  m_gtree->SetBranchAddress("EvtNum",&gVar.EvtNum);
  m_gtree->SetBranchAddress("StdHepN",&gVar.nPart);
  m_gtree->SetBranchAddress("StdHepPdg",&gVar.pdg);
  m_gtree->SetBranchAddress("StdHepStatus",&gVar.status);
  m_gtree->SetBranchAddress("StdHepP4",&gVar.p4);
  m_gtree->SetBranchAddress("StdHepX4",&gVar.x4);
  m_gtree->SetBranchAddress("EvtVtx",&gVar.vtx);
  m_gtree->SetBranchAddress("EvtWght",&gVar.weight);
  m_gtree->SetBranchAddress("EvtCode",&gVar.evtCode);

  // Set the current entry to the first entry
  fCurrentEntry = fConfig.tree_first_entry - 1;

  // Set the transformation to map the vertex coordinates to the global coordinate system
  G4PhysicalVolumeStore* pvstore = G4PhysicalVolumeStore::GetInstance();
  auto ref = pvstore->GetVolume("target_lar_pv");
  const auto to_global = geo::GetTransformToGlobal(ref);
  fTransform = to_global;
}

G4String SLArGENIEGeneratorAction::WriteConfig() const {
  G4String config_str = SLArBaseGenerator::WriteConfig();

  // Parse the base config and add GENIE-specific fields
  rapidjson::Document doc_base;
  doc_base.Parse( config_str.data() );

  rapidjson::Document d; 
  d.SetObject();
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  
  auto& dallocator = d.GetAllocator();
  d.CopyFrom(doc_base, dallocator);

  rapidjson::Value gen_tree_dict(rapidjson::kArrayType);
  gen_tree_dict.PushBack( rapidjson::StringRef("genietree_entry"), dallocator );
  gen_tree_dict.PushBack( rapidjson::StringRef("weight"), dallocator );
  gen_tree_dict.PushBack( rapidjson::StringRef("nu_energy"), dallocator );
  gen_tree_dict.PushBack( rapidjson::StringRef("nu_pdg"), dallocator );
  gen_tree_dict.PushBack( rapidjson::StringRef("target_nucleus"), dallocator );
  gen_tree_dict.PushBack( rapidjson::StringRef("target_nucleon"), dallocator );
  gen_tree_dict.PushBack( rapidjson::StringRef("interaction_type"), dallocator );
  gen_tree_dict.PushBack( rapidjson::StringRef("interaction_current"), dallocator );
  gen_tree_dict.PushBack( rapidjson::StringRef("interaction_process"), dallocator );
  gen_tree_dict.PushBack( rapidjson::StringRef("interaction_attributes"), dallocator );

  d.AddMember("gen_tree_dictionary", gen_tree_dict, dallocator);

  d.Accept(writer);
  config_str = buffer.GetString();
  return config_str;
}


//***********************************************************************

//***********************************************************************
//************************** EVENT FUNCTIONS ****************************

void SLArGENIEGeneratorAction::ParseEventEncoding(const char* code_dump, GenieEventEncoding_t& gEnc)
{
  TString code_str(code_dump);
  std::unordered_map<std::string, std::string> out;

  std::unique_ptr<TObjArray> pairs(code_str.Tokenize(";"));
  for (int i=0; i<pairs->GetEntries(); i++) {
    auto* obj = (TObjString*)pairs->At(i);
    if (!obj) continue;

    TString token = obj->GetString();
    Ssiz_t pos = token.First(':');
    if (pos < 0) continue;

    TString key = token(0, pos);
    TString val = token(pos + 1, token.Length() - pos - 1);

    key.Strip(TString::kBoth);
    val.Strip(TString::kBoth);

    out[key.Data()] = val.Data();
  }

  gEnc.pdgNu = std::stoi( out[GenieEventEncoding_t::key_nu] );
  gEnc.target = std::stoi( out[GenieEventEncoding_t::key_tgt] );
  gEnc.targetN = (out.find(GenieEventEncoding_t::key_N) != out.end()) 
    ? std::stoi( out[GenieEventEncoding_t::key_N] ) : 0;
  gEnc.proc = out[GenieEventEncoding_t::key_proc];

  gEnc.parse_process( gVar.info.proc );

  if (gEnc._proc == (int)GenieEventEncoding_t::EProc::kRES) {
    auto res_itr = out.find(GenieEventEncoding_t::key_res);
    if (res_itr != out.end()) {
      int res_val = std::stoi( res_itr->second );
      gEnc._attributes = res_val;
    }
  }
  else if (gEnc._proc == (int)GenieEventEncoding_t::EProc::kQES) {
    const auto charm_itr = out.find(GenieEventEncoding_t::key_charm);
    const auto strange_itr = out.find(GenieEventEncoding_t::key_strange);
    if (charm_itr != out.end()) {
      int charm_val = std::stoi( charm_itr->second );
      gEnc._attributes = charm_val;
    }
    else if (strange_itr != out.end()) {
      int strange_val = std::stoi( strange_itr->second );
      gEnc._attributes = strange_val;
    }
  }

  printf("Parsed GENIE event encoding: nu PDG=%d, target=%d, targetN=%d, interaction=%s\n",
      gEnc.pdgNu,
      gEnc.target,
      gEnc.targetN,
      gEnc.proc.c_str());
}

G4bool SLArGENIEGeneratorAction::CheckSelection(const GenieEventEncoding_t& info) const {

  if (fConfig.filters.empty()) { return true; }

  for (const auto& filter : fConfig.filters) {

    bool match_nu = true;
    bool match_tgt = true;
    bool match_tgtN = true;
    bool match_cur = true;
    bool match_prc = true;

    if (filter.HasNeutrinoCuts()) {
      match_nu = false;
      for (auto pdg : filter.nu_pdgs) if (pdg == info.pdgNu) { match_nu = true; break; }
    }

    if (filter.HasTargetCuts()) {
      match_tgt = false;
      for (auto tgt : filter.target_nuclei) if (tgt == info.target) { match_tgt = true; break; }
    }

    if (filter.HasTargetNCuts()) {
      match_tgtN = false;
      for (auto N : filter.target_nucleons) if (N == info.targetN) { match_tgtN = true; break; }
    }

    if (filter.HasCurrrentCuts()) {
      match_cur = false;
      auto evt_curr = static_cast<GenieEventEncoding_t::ECurrent>(info._current);
      for (auto cur : filter.currents) if (cur == evt_curr) { match_cur = true; break; }
    }

    if (filter.HasProcessCuts()) {
      match_prc = false;
      auto evt_proc = static_cast<GenieEventEncoding_t::EProc>(info._proc);
      for (auto prc : filter.processes) if (prc == evt_proc) { match_prc = true; break; }
    }

    // Apply global AND logic
    if (match_nu && match_tgt && match_cur && match_prc) {
      return true;
    }
  }

  return false;
};

G4bool SLArGENIEGeneratorAction::CheckSelection(const GenieEvent_t& gEvent) const {
  // return true if no filters are defined
  if (fConfig.filters.empty()) return true;

  for (const auto& filter : fConfig.filters) {
    const G4bool pass_event_enc_cuts = CheckSelection( gEvent.info );
    
    bool match_products = true;

    if (filter.products_pdg.empty() == false) {
      match_products = false;
      for (int i=0; i<gEvent.nPart; i++) {
        if (gVar.status[i] == 1) {
          for (auto req_pdg : filter.products_pdg) {
            if (gVar.pdg[i] == req_pdg) {
              match_products = true;
              break; 
            }
          }
        }
        if (match_products) break;
      }
    }

    if (pass_event_enc_cuts && match_products) {
      return true;
    }
  }

  return false;
}


void SLArGENIEGeneratorAction::GeneratePrimaries(G4Event *ev)
{
  auto& gen_records = SLArAnalysisManager::Instance()->GetGenRecords();
  G4bool select_event = false; 
  while (select_event == false && fCurrentEntry < m_gtree->GetEntries() && fCurrentEntry < fConfig.tree_last_entry) {
    fCurrentEntry++;
    gVar.info.reset();
    gVar.nuEnergy = {};
    m_gtree->GetEntry(fCurrentEntry);
    gVar.EvtNum = fCurrentEntry;

    ParseEventEncoding( gVar.evtCode->GetString().Data(), gVar.info );

    if ( CheckSelection(gVar) ) select_event = true;
  }

  if (fCurrentEntry == m_gtree->GetEntries()) {
    G4Exception("SLArGENIEGeneratorAction::GeneratePrimaries",
        "OutOfRange",
        JustWarning,
        "No more GENIE events available in the input tree.");

    // terminate the run
    G4RunManager::GetRunManager()->AbortRun( true );
  }

  if (fCurrentEntry == fConfig.tree_last_entry) {
    G4Exception("SLArGENIEGeneratorAction::GeneratePrimaries",
        "RunCompleted",
        JustWarning,
        "All the requested GENIE events have been processed.");

    // terminate the run
    G4RunManager::GetRunManager()->AbortRun( true );
  }

  size_t particle_idx = 0; // Think this can be done in a better way

  HepGeom::Point3D<G4double> vtx(0.,0.,0.);
  vtx.set(gVar.vtx[2]*CLHEP::m, gVar.vtx[1]*CLHEP::m, gVar.vtx[0]*CLHEP::m);
  vtx = fTransform * vtx;

  std::vector<G4PrimaryVertex*> primary_vertices;

  const double gen_time = fVtxGen->GetTimeGenerator().SampleTime();

  for (int i=0; i<gVar.nPart; i++){

    /*G4bool pdg_valid = SLArBaseGenerator::PDGCodeIsValid( gVar.pdg[i] ); 
    if ( pdg_valid == false ) {
      fprintf(stdout, "SLArGENIEGeneratorAction::GeneratePrimaries() WARNING. Event %i contains particle with PDG code %i, which is not valid.\n", 
          evtNum, gVar.pdg[i]); 
      continue;
    }*/

    if (gVar.pdg[i] >= 2000000000) continue;
    
    if (gVar.status[i] == 0) { // 0 - incoming; 1 - outgoing; x - virtual
      if ( abs(gVar.pdg[i]) == 12 || abs(gVar.pdg[i]) == 14 || abs(gVar.pdg[i]) == 16 ) {
        gVar.nuEnergy = gVar.p4[i][3];
      }
    }
    else if (gVar.status[i] == 1) {
      G4PrimaryParticle *particle = new G4PrimaryParticle(gVar.pdg[i],
          gVar.p4[i][2]*CLHEP::GeV,
          gVar.p4[i][1]*CLHEP::GeV,
          gVar.p4[i][0]*CLHEP::GeV,
          gVar.p4[i][3]*CLHEP::GeV);
      // Set the time of the primary particle
      double time = gen_time;

      auto vertex = new G4PrimaryVertex(vtx, time); // Not sure which is better here
      //    auto vertex = new G4PrimaryVertex(vtx, gVar.vtx[3]); 
      vertex->SetPrimary(particle);
      primary_vertices.push_back(vertex);

      particle_idx++;
    }
  }

  // store primary neutrino info in gen record
  auto& record = gen_records.AddRecord( GetGeneratorEnum(), fLabel );
  auto& status = record.GetGenStatus();
  status.push_back( static_cast<float>(gVar.EvtNum) );
  status.push_back( static_cast<float>(gVar.weight) ); 
  status.push_back( static_cast<float>(gVar.nuEnergy) );
  status.push_back( static_cast<float>(gVar.info.pdgNu) ); 
  status.push_back( static_cast<float>(gVar.info.target) ); 
  status.push_back( static_cast<float>(gVar.info.targetN) );
  status.push_back( static_cast<float>(gVar.info._type) );
  status.push_back( static_cast<float>(gVar.info._current) );
  status.push_back( static_cast<float>(gVar.info._proc) );
  status.push_back( static_cast<float>(gVar.info._attributes) );
  
  for (const auto& vertex : primary_vertices) {
    ev->AddPrimaryVertex(vertex);
  }

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
