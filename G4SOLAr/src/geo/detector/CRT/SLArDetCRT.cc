/*************************************************
 * Filename:   SLArDetCRT.cc    		 *
 * Author:     Jordan McElwee 			 *
 * Created:    2025-03-05 14:07 		 * 
 * Description:					 *
 *************************************************/

#include "detector/CRT/SLArDetCRT.hh"

//****************************************************************************
//***** CONSTRUCTORS *********************************************************

// - - - - - - - - - - - - - - -
SLArDetCRT::SLArDetCRT() : SLArBaseDetModule(), fMatCRT(nullptr)
{
    fGeoInfo = new SLArGeoInfo();
}
// - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - -
SLArDetCRT::~SLArDetCRT()
{
    std::cout << "Deleting SLArDetCRT..." << std::endl;
    std::cout << "SLArDetCRT DONE" << std::endl;
}
// - - - - - - - - - - - - - - -

//****************************************************************************


//****************************************************************************

// - - - - - - - - - - - - - - -
void SLArDetCRT::BuildMaterial(G4String db_file)
{
    /*
        The material here needs to change, but it's not in the database.
        --JM
    */
    fMatCRT = new SLArMaterial();
    fMatCRT->SetMaterialID("Steel");
    fMatCRT->BuildMaterialFromDB(db_file);
}
// - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - -
void SLArDetCRT::BuildDefaultGeoParMap()
{
    G4cout << "SLArDetCRT::BuildDefaultGeoParMap() Using default values" << G4endl;
    fGeoInfo->RegisterGeoPar("crt_x", 10.*CLHEP::m);
    fGeoInfo->RegisterGeoPar("crt_y",  1.*CLHEP::m);
    fGeoInfo->RegisterGeoPar("crt_z", 10.*CLHEP::m);
}

// - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - -
void SLArDetCRT::BuildCRT()
{ 

  G4cout << "SLArDetCRT::BuildCRT()" << G4endl;

  G4double x_ = fGeoInfo->GetGeoPar("crt_x")*0.5;
  G4double y_ = fGeoInfo->GetGeoPar("crt_y")*0.5;
  G4double z_ = fGeoInfo->GetGeoPar("crt_z")*0.5;

  // Should come from BaseDetMod
  fModSV = new G4Box("CRT", x_, y_, z_);
  SetLogicVolume(new G4LogicalVolume(fModSV,
				     fMatCRT->GetMaterial(),
				     "CRT"+std::to_string(fID)+"_lv", 0, 0, 0) );
 
}
// - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - -
void SLArDetCRT::Init(const rapidjson::Value& jconf)
{

  // Get basic information
  assert(jconf.IsObject());
  auto jcrt = jconf.GetObject();
  assert(jcrt.HasMember("dimensions"));
  assert(jcrt.HasMember("position"  ));
  assert(jcrt.HasMember("copyID"    ));

  SetID(jcrt["copyID"].GetInt());
  fGeoInfo->ReadFromJSON(jcrt["dimensions"].GetArray());

  const auto jposition = jcrt["position"].GetObj(); 
  G4double vunit = 1.0;
  if ( jposition.HasMember("unit") )
    vunit = G4UIcommand::ValueOf( jposition["unit"].GetString() );
  G4String svar[3] = {"x", "y", "z"};
  int ii = 0;
  for (const auto &v : jposition["xyz"].GetArray()) {
    G4double tmp = v.GetDouble() * vunit; 
    fGeoInfo->RegisterGeoPar("crt_pos_"+svar[ii], tmp); 
    ++ii; 
  }

}
// - - - - - - - - - - - - - - -

//****************************************************************************


//****************************************************************************
//***** OTHER STUFF **********************************************************

// - - - - - - - - - - - - - - -
void SLArDetCRT::SetVisAttributes()
{
    G4cout << "SLArDetCRT::SetVisAttributes Setting visual parameters." << G4endl;

    G4VisAttributes *visOut = new G4VisAttributes();

    visOut->SetVisibility(true);
    visOut->SetColour(1., 1., 0., 1.);
    fModLV->SetVisAttributes( visOut );
}
// - - - - - - - - - - - - - - -

//****************************************************************************








