/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArRecoHits
 * @created     : Tuesday Apr 16, 2024 14:27:18 CEST
 */

#ifndef SLARRECOHITS_HPP

#define SLARRECOHITS_HPP

#include <stdio.h>
#include <assert.h>
#include <TNamed.h>

struct RecoHit_t {
  Float_t x {};
  Float_t y {};
  Float_t z {};
  Float_t charge_true {};
  Float_t charge_reco {};
  Float_t time {};
  UInt_t channel_id {};
  Int_t tpc_id {};

  void reset() {
    x = 0.0; 
    y = 0.0;
    z = 0.0; 
    charge_true = 0.0; 
    charge_reco = 0.0; 
    time = 0.0; 
    channel_id = 0;
    tpc_id = 0;
  }
};

struct hitvarContainer_t {
  std::vector<Float_t> hit_x;
  std::vector<Float_t> hit_y;
  std::vector<Float_t> hit_z; 
  std::vector<Float_t> hit_q;
  std::vector<Float_t> hit_qtrue;
  std::vector<Int_t>   hit_tpc;

  void push_back(const RecoHit_t& hit) {
    hit_x.push_back( hit.x ); 
    hit_y.push_back( hit.y ); 
    hit_z.push_back( hit.z ); 
    hit_q.push_back( hit.charge_reco ); 
    hit_qtrue.push_back( hit.charge_true );
    hit_tpc.push_back( hit.tpc_id ); 
  };

  void reset() {
    hit_x.clear(); hit_x.reserve(500);
    hit_y.clear(); hit_y.reserve(500);
    hit_z.clear(); hit_z.reserve(500); 
    hit_q.clear(); hit_q.reserve(500);
    hit_qtrue.clear(); hit_qtrue.reserve(500);
    hit_tpc.clear(); hit_tpc.reserve(500);
  }
};

struct hitvarContainerPtr_t {
  std::vector<Float_t>* hit_x;
  std::vector<Float_t>* hit_y;
  std::vector<Float_t>* hit_z; 
  std::vector<Float_t>* hit_q;
  std::vector<Float_t>* hit_qtrue;
  std::vector<Int_t>  * hit_tpc;

  inline void push_back(const RecoHit_t& hit) {
    hit_x->push_back( hit.x ); 
    hit_y->push_back( hit.y ); 
    hit_z->push_back( hit.z ); 
    hit_q->push_back( hit.charge_reco );
    hit_qtrue->push_back( hit.charge_true );
    hit_tpc->push_back( hit.tpc_id ); 
  };

  inline void reset() {
    hit_x->clear(); hit_x->reserve(500);
    hit_y->clear(); hit_y->reserve(500);
    hit_z->clear(); hit_z->reserve(500); 
    hit_q->clear(); hit_q->reserve(500);
    hit_qtrue->clear(); hit_qtrue->reserve(500);
    hit_tpc->clear(); hit_tpc->reserve(500);
  }

  inline hitvarContainerPtr_t() {
    hit_x = new std::vector<Float_t>();
    hit_y = new std::vector<Float_t>();    
    hit_z = new std::vector<Float_t>();    
    hit_q = new std::vector<Float_t>();
    hit_qtrue = new std::vector<Float_t>();
    hit_tpc = new std::vector<Int_t>();
  }

  inline ~hitvarContainerPtr_t() {
    if (hit_x) delete hit_x;
    if (hit_y) delete hit_y;
    if (hit_z) delete hit_z;
    if (hit_q) delete hit_q;
    if (hit_qtrue) delete hit_qtrue;
    if (hit_tpc) delete hit_tpc;
  }

};
struct OpHit_t {
  Float_t charge = {}; 
  Float_t time = {};

  OpHit_t() { }

  OpHit_t(const OpHit_t& h) {
    charge = h.charge; 
    time = h.time; 
  }

  void reset() {
    charge = 0.0; 
    time = 0.0; 
  }
};

typedef std::array<UInt_t, 3> OpDetID_t; 

struct OpDetEvent_t {
  OpDetID_t id = {0,0,0}; 
  std::vector<OpHit_t> hits;

  OpDetEvent_t(const UInt_t idx0, const UInt_t idx1, const UInt_t idx2) :
    id{idx0, idx1, idx2} {
      hits.reserve(50);
    }

  OpDetEvent_t(const OpDetEvent_t& opdetev) {
    id = opdetev.id; 
    hits.reserve(opdetev.hits.size()); 
    for (const auto& h : opdetev.hits) {
      hits.emplace_back( h ); 
    }
  }
  
  void register_hit(const OpHit_t& h) {
    hits.emplace_back( h ); 
  }

  void reset() {
    id = {}; 
    hits.clear();
    hits.reserve(50);
  }

};




#endif /* end of include guard SLARRECOHITS_HPP */

