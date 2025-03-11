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
void SLArDetCRT::BuildCRT()
{ 

  G4cout << "SLArDetCRT::BuildCRT()" << G4endl;
  fGeoInfo->RegisterGeoPar("crt_x", 10.*CLHEP::m);
  fGeoInfo->RegisterGeoPar("crt_y",  1.*CLHEP::m);
  fGeoInfo->RegisterGeoPar("crt_z", 10.*CLHEP::m);
  G4cout << "Done building defaults." << G4endl;

  fGeoInfo->RegisterGeoPar("pos_x",  0.*CLHEP::m);
  fGeoInfo->RegisterGeoPar("pos_y", 10.*CLHEP::m);
  fGeoInfo->RegisterGeoPar("pos_z",  0.*CLHEP::m);

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
/*void SLArDetCRT::Init(const rapidjson::Value& jconf)
{
  assert(jconf.IsObject());
  auto jtpc = jconf.GetObject();
  assert(jtpc.HasMember("dimensions"));
  assert(jtpc.HasMember("position"  ));
  assert(jtpc.HasMember("copyID"    ));

  SetID(jtpc["copyID"].GetInt());
  fGeoInfo->ReadFromJSON(jtpc["dimensions"].GetArray());
  }*/
// - - - - - - - - - - - - - - -

//****************************************************************************
