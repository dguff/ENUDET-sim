/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArMCEvent.hh
 * @created     : mercoledì ago 10, 2022 11:52:04 CEST
 */

#ifndef SLArMCEVENT_HH

#define SLArMCEVENT_HH

#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "event/SLArMCPrimaryInfo.hh"
#include "event/SLArEventReadoutTileSystem.hh"
#include "config/SLArCfgBaseSystem.hh"
#include "config/SLArCfgMegaTile.hh"


class SLArMCEvent : public TObject
{
  public: 

    SLArMCEvent();
    ~SLArMCEvent();

    int SetEvNumber(int nEv);
    int GetEvNumber() {return fEvNumber;}

    void SetDirection(double* dir = nullptr); 
    void SetDirection(double px, double py, double pz); 
    std::array<double, 3> GetDirection() {return fDirection;}
    int ConfigReadoutTileSystem (SLArCfgPixSys* pixSysCfg);

    SLArEventReadoutTileSystem* GetReadoutTileSystem() {return fEvSystemTile;}

    std::vector<SLArMCPrimaryInfo*>& GetPrimaries() {return fSLArPrimary ;}
    SLArMCPrimaryInfo* GetPrimary(int ip) {return fSLArPrimary.at(ip);}
    bool  CheckIfPrimary(int trkId);

    inline size_t RegisterPrimary(SLArMCPrimaryInfo* p) 
      {fSLArPrimary.push_back(p); return fSLArPrimary.size();}

    void  Reset();

  private:
    int fEvNumber;
    std::array<double, 3>  fDirection; 
    std::vector<SLArMCPrimaryInfo*> fSLArPrimary; 
    SLArEventReadoutTileSystem* fEvSystemTile;

  public:
    ClassDef(SLArMCEvent, 1);
};


#endif /* end of include guard SLArEVENT_HH */

