/**
 * @author      : Antonio Branca (antonio.branca@mib.infn.it)
 * @file        : SLArEventCRT
 * @created     : 2025-03-26 15:00
 */

#include "TObject.h"

#include "G4Types.hh"
#include "Math/Point3D.h"
#include "Math/Vector3D.h"

#ifndef SOLAREVENTCRT_HH

#define SOLAREVENTCRT_HH

class SLArEventCRT : public TObject {
public:
    SLArEventCRT();
    virtual ~SLArEventCRT();

    void SetCRTNo(int idx) { fCRTNo = idx; }
    void SetPDG(int pdg) { fPDG = pdg; }
    void SetTime(double time) { fTime = time; }
    void SetEkin(double ekin) { fEkin = ekin; }
    void SetLocalPos(double x, double y, double z);
    void SetGlobalPos(double x, double y, double z);
    void SetDir(double dx, double dy, double dz);

    int GetCRTNo() { return fCRTNo; }
    int GetPDG() { return fPDG; }
    double GetTime() { return fTime; }
    double GetEkin() { return fEkin; }
    ROOT::Math::XYZPoint GetLocalPos() { return fLocalPos; };
    ROOT::Math::XYZPoint GetGlobalPos() { return fGlobalPos; };
    ROOT::Math::XYZVector GetDir() { return fDirection; };

private:
    int fCRTNo;
    int fPDG;
    double fTime;
    double fEkin;
    ROOT::Math::XYZPoint fLocalPos;
    ROOT::Math::XYZPoint fGlobalPos;
    ROOT::Math::XYZVector fDirection;

    ClassDef(SLArEventCRT, 1);
};

class SLArListEventCRT : public TObject {
  public: 
    inline SLArListEventCRT() : fEvNumber(-1), TObject() {}

    inline SLArListEventCRT(const SLArListEventCRT& ev) 
      : fEvNumber(ev.fEvNumber), TObject(ev) 
    {
      for (const auto& p : ev.fCrtHits) {
        fCrtHits.push_back( SLArEventCRT(p) );
      }
    }

    inline ~SLArListEventCRT() { fCrtHits.clear(); }

    inline void SetEventNumber(int ev) {fEvNumber = ev;}
    inline int  GetEventNumber() const {return fEvNumber;}

    inline std::vector<SLArEventCRT>& GetCRTHits() {return fCrtHits;}

    inline void RegisterCRTHit(const SLArEventCRT& hit) {
      fCrtHits.push_back( hit );
    }

    inline void Reset() {
      fCrtHits.clear();
      fEvNumber = -1;
    }

  private: 
    int fEvNumber; 
    std::vector<SLArEventCRT> fCrtHits; 

  public:
    ClassDef(SLArListEventCRT, 1); 
};

#endif