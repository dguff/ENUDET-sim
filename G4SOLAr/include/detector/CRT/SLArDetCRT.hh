/*************************************************
 * Filename:   SLArDetCRT.hh     		 *
 * Author:     Jordan McElwee 			 *
 * Created:    2025-03-04 15:18 		 * 
 * Description:					 *
 *************************************************/

#ifndef SLARDETCRT_HH
#define SLARDETCRT_HH

#include "detector/SLArBaseDetModule.hh"

class SLArDetCRT : public SLArBaseDetModule {

public:
  SLArDetCRT();
  virtual ~SLArDetCRT();

  void BuildCRT();
  void BuildMaterial(G4String);


  virtual void Init(const rapidjson::Value& jconf) override;
  

private:
  SLArMaterial *fMatCRT;
  
};

#endif // Header guard
