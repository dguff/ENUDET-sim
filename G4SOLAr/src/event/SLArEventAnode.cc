/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventAnode.cc
 * @created     : Wed Aug 10, 2022 14:39:21 CEST
 */

#include <memory>
#include "event/SLArEventAnode.hh"
#include "config/SLArCfgMegaTile.hh"

templateClassImp(SLArEventAnode)

template class SLArEventAnode<std::unique_ptr<SLArEventMegatileUniquePtr>, std::unique_ptr<SLArEventTileUniquePtr>, std::unique_ptr<SLArEventChargePixel>>;
template class SLArEventAnode<SLArEventMegatilePtr*, SLArEventTilePtr*, SLArEventChargePixel*>;

template<class M, class T, class P>
SLArEventAnode<M, T, P>::SLArEventAnode()
  : fID(0), fNhits(0), fIsActive(true), 
    fLightBacktrackerRecordSize(0), fChargeBacktrackerRecordSize(0) 
{}

template<>
SLArEventAnodeUniquePtr::SLArEventAnode(const SLArEventAnode& right) 
  : TNamed(right) 
{
  fID = right.fID; 
  fNhits = right.fNhits;
  fIsActive = right.fIsActive; 
  fLightBacktrackerRecordSize = right.fLightBacktrackerRecordSize;
  fChargeBacktrackerRecordSize = right.fChargeBacktrackerRecordSize;
  for (const auto &mgev : right.fMegaTilesMap) {
    fMegaTilesMap[mgev.first] = std::make_unique<SLArEventMegatileUniquePtr>(*mgev.second);
  }
}

template<>
SLArEventAnodePtr::SLArEventAnode(const SLArEventAnodePtr& right) 
  : TNamed(right) 
{
  fID = right.fID; 
  fNhits = right.fNhits;
  fIsActive = right.fIsActive; 
  fLightBacktrackerRecordSize = right.fLightBacktrackerRecordSize;
  fChargeBacktrackerRecordSize = right.fChargeBacktrackerRecordSize;
  for (const auto &mgev : right.fMegaTilesMap) {
    fMegaTilesMap[mgev.first] = new SLArEventMegatilePtr(*mgev.second);
  }
}

template<class M, class T, class P>
SLArEventAnode<M,T,P>::SLArEventAnode(SLArCfgAnode* cfg) {
  SetName(cfg->GetName()); 
  //ConfigSystem(cfg); 
  return;
}

template<>
SLArEventAnodeUniquePtr::~SLArEventAnode() {
  for (auto &mgtile : fMegaTilesMap) {
    mgtile.second->ResetHits();
  }
  fMegaTilesMap.clear(); 
}

template<>
SLArEventAnodePtr::~SLArEventAnode() {
  for (auto &mgtile : fMegaTilesMap) {
    mgtile.second->ResetHits();
    delete mgtile.second;
  }
  fMegaTilesMap.clear(); 
}

template<>
template<>
void SLArEventAnodeUniquePtr::SoftCopy(SLArEventAnodePtr& record) const 
{
  record.SoftResetHits();
  record.SetName(fName); 
  record.SetID( fID ); 
  record.SetNhits( fNhits ); 
  record.SetActive( fIsActive ); 
  record.SetLightBacktrackerRecordSize( fLightBacktrackerRecordSize ); 
  record.SetChargeBacktrackerRecordSize( fChargeBacktrackerRecordSize ); 

  for (const auto& mt : fMegaTilesMap) {
    record.GetMegaTilesMap()[mt.first] = new SLArEventMegatilePtr(); 

  }
}

template<>
int SLArEventAnodeUniquePtr::ConfigSystem(SLArCfgAnode* cfg) {
  int imegatile = 0; 
  fID = cfg->GetIdx(); 
  for (const auto &mtcfg : cfg->GetMap()) {
    int megatile_idx = mtcfg.second->GetIdx(); 
    if (fMegaTilesMap.count(megatile_idx) == 0) {
      std::unique_ptr<SLArEventMegatileUniquePtr> evMT = std::make_unique<SLArEventMegatileUniquePtr>(mtcfg.second);
      fMegaTilesMap.insert( std::make_pair( megatile_idx, std::move(evMT) ) );
      imegatile++;
    }
  }
  return imegatile;
}

template<>
int SLArEventAnodePtr::ConfigSystem(SLArCfgAnode* cfg) {
  int imegatile = 0; 
  fID = cfg->GetIdx(); 
  for (const auto &mtcfg : cfg->GetMap()) {
    int megatile_idx = mtcfg.second->GetIdx(); 
    if (fMegaTilesMap.count(megatile_idx) == 0) {
      fMegaTilesMap.insert( std::make_pair( megatile_idx, new SLArEventMegatilePtr(mtcfg.second) ) );
      imegatile++;
    }
  }
  return imegatile;
}


template<>
std::unique_ptr<SLArEventMegatileUniquePtr>& SLArEventAnodeUniquePtr::GetOrCreateEventMegatile(const int mtIdx) {
  auto it = fMegaTilesMap.find(mtIdx);
  if (it != fMegaTilesMap.end()) {
    //printf("SLArEventAnode::CreateEventMegatile(%i): Megatile nr %i already present in anode %i register\n", mtIdx, mtIdx, fID);
    //getchar();
    return fMegaTilesMap.find(mtIdx)->second;
  }
  else {
    std::unique_ptr<SLArEventMegatileUniquePtr> mt_event = std::make_unique<SLArEventMegatileUniquePtr>();
    mt_event->SetIdx(mtIdx); 
    mt_event->SetLightBacktrackerRecordSize( fLightBacktrackerRecordSize); 
    mt_event->SetChargeBacktrackerRecordSize( fChargeBacktrackerRecordSize); 
    fMegaTilesMap.insert( std::make_pair(mtIdx, std::move(mt_event)) );  
    return fMegaTilesMap[mtIdx];
  }
}

template<>
SLArEventMegatilePtr*& SLArEventAnodePtr::GetOrCreateEventMegatile(const int mtIdx) {
  auto it = fMegaTilesMap.find(mtIdx);
  if (it != fMegaTilesMap.end()) {
    //printf("SLArEventAnode::CreateEventMegatile(%i): Megatile nr %i already present in anode %i register\n", mtIdx, mtIdx, fID);
    //getchar();
    return fMegaTilesMap.find(mtIdx)->second;
  }
  else {
    SLArEventMegatilePtr* mt_event = new SLArEventMegatilePtr();
    mt_event->SetIdx(mtIdx); 
    mt_event->SetLightBacktrackerRecordSize( fLightBacktrackerRecordSize); 
    mt_event->SetChargeBacktrackerRecordSize( fChargeBacktrackerRecordSize); 
    fMegaTilesMap.insert( std::make_pair(mtIdx, std::move(mt_event)) );  
    return fMegaTilesMap[mtIdx];
  }
}

template<class M, class T, class P>
T& SLArEventAnode<M,T,P>::RegisterHit(const SLArEventPhotonHit& hit) {
  int mgtile_idx = hit.GetMegaTileIdx(); 
  M& mt_event = GetOrCreateEventMegatile(mgtile_idx);
  T& t_event = mt_event->RegisterHit(hit);
  return (mt_event->GetTileMap()[t_event->GetIdx()]);
  //} else {
    //printf("SLArEventAnode::RegisterHit WARNING\n"); 
    //printf("Megatile with ID %i is not in store\n", mgtile_idx); 
    //CreateEventMegatile(hit->GetMegaTileIdx());
    //return 0; 
  //}
}

template<class M, class T, class P>
P& SLArEventAnode<M, T, P>::RegisterChargeHit(const SLArCfgAnode::SLArPixIdxCoord& pixID, const SLArEventChargeHit& hit) {
  const int mgtile_idx = pixID.at(0);
  const int tile_idx = pixID.at(1); 
  const int pix_idx = pixID.at(2); 

  M& mt_event = GetOrCreateEventMegatile(mgtile_idx); 
  T& t_event = mt_event->GetOrCreateEventTile(tile_idx);
  P& p_event = t_event->RegisterChargeHit(pix_idx, hit); 

  return p_event;
  //} else {
    //printf("SLArEventAnode::RegisterHit WARNING\n"); 
    //printf("Megatile with ID %i is not in store\n", mgtile_idx); 
    //CreateEventMegatile(hit->GetMegaTileIdx());
    //return 0; 
  //}
}

template<>
int SLArEventAnodeUniquePtr::ResetHits() {
  printf("SLArEventAnode::ResetHits() clear event on anode %i\n", fID);
  int nn = 0; 
  for (auto &mgtile : fMegaTilesMap) {
    nn += mgtile.second->ResetHits(); 
  }

  fMegaTilesMap.clear();
  return nn; 
}

template<>
int SLArEventAnodePtr::ResetHits() {
  printf("SLArEventAnode::ResetHits() clear event on anode %i\n", fID);
  int nn = 0; 
  for (auto &mgtile : fMegaTilesMap) {
    nn += mgtile.second->ResetHits(); 
    delete mgtile.second;
  }

  fMegaTilesMap.clear();
  return nn; 
}


template<class M, class T, class P>
int SLArEventAnode<M,T,P>::SoftResetHits() {
  printf("SLArEventAnode::ResetHits() clear event on anode %i\n", fID);
  int nn = 0; 
  for (auto &mgtile : fMegaTilesMap) {
    nn += mgtile.second->SoftResetHits(); 
  }

  fMegaTilesMap.clear();
  return nn; 
}

template<class M, class T, class P>
void SLArEventAnode<M,T,P>::SetActive(bool is_active) {
  fIsActive = is_active; 
  for (auto &mgtile : fMegaTilesMap) {
    mgtile.second->SetActive(is_active); 
  }
}

//bool SLArEventAnode::SortHits() {
  //int isort = true;
  //for (auto &mgtile : fMegaTilesMap) {
    //isort *= mgtile.second->SortHits(); 
  //}
  //return isort;
//}
