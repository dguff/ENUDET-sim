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

//****************************************************************************