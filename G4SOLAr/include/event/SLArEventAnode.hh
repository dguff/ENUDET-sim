/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventAnode.hh
 * @created     : Wed Aug 10, 2022 14:29:23 CEST
 */

#ifndef SLAREVENTANODE_HH

#define SLAREVENTANODE_HH

#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgAssembly.hh"
#include "config/SLArCfgMegaTile.hh"
#include "event/SLArEventMegatile.hh"

template<class M, class T, class P>
class SLArEventAnode : public TNamed {
  public:
    SLArEventAnode(); 
    SLArEventAnode(SLArCfgAnode* cfg);
    SLArEventAnode(const SLArEventAnode&);
    ~SLArEventAnode(); 

    int ConfigSystem(SLArCfgAnode* cfg);
    M& GetOrCreateEventMegatile(const int mtIdx); 
    inline std::map<int, M>& GetMegaTilesMap() {return fMegaTilesMap;}
    inline const std::map<int, M>& GetConstMegaTilesMap() const {return fMegaTilesMap;}
    inline int GetNhits() const {return fNhits;}
    inline bool IsActive() const {return fIsActive;}

    T& RegisterHit(const SLArEventPhotonHit& hit); 
    P& RegisterChargeHit(const SLArCfgAnode::SLArPixIdxCoord& pixId, const SLArEventChargeHit& hit); 
    int ResetHits(); 

    void SetActive(bool is_active); 
    inline void SetChargeBacktrackerRecordSize(const UShort_t size) {fChargeBacktrackerRecordSize = size;}
    inline UShort_t GetChargeBacktrackerRecordSize() const {return fChargeBacktrackerRecordSize;}
    inline void SetLightBacktrackerRecordSize(const UShort_t size) {fLightBacktrackerRecordSize = size;}
    inline UShort_t GetLightBacktrackerRecordSize() const {return fLightBacktrackerRecordSize;}
    //bool SortHits(); 

    inline void SetID(const int anode_id) {fID = anode_id;}
    inline int  GetID() const {return fID;}

  private:
    int fID; 
    int fNhits; 
    bool fIsActive;
    UShort_t fLightBacktrackerRecordSize;
    UShort_t fChargeBacktrackerRecordSize;
    std::map<int, M> fMegaTilesMap;

  public:
    ClassDef(SLArEventAnode, 2)
};

typedef SLArEventAnode<std::unique_ptr<SLArEventMegatileUniquePtr>, std::unique_ptr<SLArEventTileUniquePtr>, std::unique_ptr<SLArEventChargePixel>> SLArEventAnodeUniquePtr;
typedef SLArEventAnode<SLArEventMegatilePtr*, SLArEventTilePtr*, SLArEventChargePixel*> SLArEventAnodePtr;

#endif /* end of include guard SLArEventAnode_HH */

