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

    inline void SetCRTNo(int idx) { fCRTNo = idx; }
    inline void SetPDG(int pdg) { fPDG = pdg; }
    inline void SetTime(double time) { fTime = time; }
    inline void SetEkin(double ekin) { fEkin = ekin; }
    void SetLocalPos(double x, double y, double z);
    void SetGlobalPos(double x, double y, double z);
    void SetDir(double dx, double dy, double dz);

    inline int GetCRTNo() const { return fCRTNo; }
    inline int GetPDG() const { return fPDG; }
    inline double GetTime() const { return fTime; }
    inline double GetEkin() const { return fEkin; }
    inline ROOT::Math::XYZPoint GetLocalPos() const { return fLocalPos; };
    inline ROOT::Math::XYZPoint GetGlobalPos() const { return fGlobalPos; };
    inline ROOT::Math::XYZVector GetDir() const { return fDirection; };

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
