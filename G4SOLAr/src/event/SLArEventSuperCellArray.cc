/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArEventSuperCellArray.cc
 * @created     Thur Oct 20, 2022 16:16:03 CEST
 */

#include "event/SLArEventSuperCellArray.hh"

ClassImp(SLArEventSuperCellArray)

SLArEventSuperCellArray::SLArEventSuperCellArray()
  : TNamed(), fNhits(0), fIsActive(true) {}

SLArEventSuperCellArray::SLArEventSuperCellArray(const SLArEventSuperCellArray& ev)
  : TNamed(ev) 
{
  fNhits = ev.fNhits; 
  fIsActive = ev.fIsActive; 
  for (const auto &sc : ev.fSuperCellMap) {
    fSuperCellMap.insert(
        std::make_pair(sc.first, (SLArEventSuperCell*)sc.second->Clone()));
  }
  return;
}

SLArEventSuperCellArray::SLArEventSuperCellArray(SLArCfgSuperCellArray* cfg) 
  : SLArEventSuperCellArray()
{
  SetName(cfg->GetName());
  ConfigSystem(cfg); 
  return;
}

SLArEventSuperCellArray::~SLArEventSuperCellArray() {
  for (auto &scevent : fSuperCellMap) {
    delete scevent.second; scevent.second = nullptr;
  }
  fSuperCellMap.clear(); 
}

int SLArEventSuperCellArray::ConfigSystem(SLArCfgSuperCellArray* cfg) {
  int nsc = 0; 
  for (const auto &sc : cfg->GetMap()) {
      if (fSuperCellMap.count(sc.first) == 0) {
        fSuperCellMap.insert(
              std::make_pair(sc.first, new SLArEventSuperCell(sc.first))
            ); 
        nsc++;
    }
  }

  return nsc; 
}


int SLArEventSuperCellArray::RegisterHit(SLArEventPhotonHit* hit) {
  int sc_idx = hit->GetTileIdx(); 
  if (fSuperCellMap.count(sc_idx)) {
    fSuperCellMap.find(sc_idx)->second->RegisterHit(hit);
    fNhits++;
    return 1; 
  } else {
    printf("SLArEventSuperCellArray::RegisterHit WARNING\n"); 
    printf("SuperCell with ID %i is not in store [%i,%i,%i]\n", 
        sc_idx, hit->GetMegaTileIdx(), hit->GetRowTileNr(), hit->GetTileNr()); 
    return 0; 
  }
}

int SLArEventSuperCellArray::ResetHits() {
  int nn = 0; 
  for (auto &sc : fSuperCellMap) {
    nn += sc.second->ResetHits(); 
  }
  fNhits = 0; 
  return nn; 
}

void SLArEventSuperCellArray::SetActive(bool is_active) {
  fIsActive = is_active; 
  for (auto &sc : fSuperCellMap) {
    sc.second->SetActive(is_active); 
  }
}

bool SLArEventSuperCellArray::SortHits() {
  int isort = true;
  for (auto &sc : fSuperCellMap) {
    isort *= sc.second->SortHits(); 
  }
  return isort;
}












