/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArRecoHits
 * @created     : Friday Nov 22, 2024 09:49:06 CET
 */

#include "SLArRecoHits.hh"

ClassImp(reco::RecoHit)
ClassImp(reco::hitvarContainer)
ClassImp(reco::hitvarContainerPtr)

namespace reco {

RecoHit& RecoHit::operator=(const RecoHit& other) {
  if (this != &other) {
    x = other.x;
    y = other.y;
    z = other.z;
    charge_true = other.charge_true;
    charge_reco = other.charge_reco;
    time = other.time;
    channel_id = other.channel_id;
    tpc_id = other.tpc_id;

    TObject::operator=(other);
  }
  return *this;
}

bool RecoHit::operator==(const RecoHit& other) const {
  return x == other.x &&
    y == other.y &&
    z == other.z &&
    charge_true == other.charge_true &&
    charge_reco == other.charge_reco &&
    time == other.time &&
    channel_id == other.channel_id &&
    tpc_id == other.tpc_id;
}


hitvarContainer::hitvarContainer() {
  hit_x.reserve(500);
  hit_y.reserve(500);
  hit_z.reserve(500);
  hit_q.reserve(500);
  hit_qtrue.reserve(500);
  hit_tpc.reserve(500);
}

hitvarContainer::hitvarContainer(const hitvarContainer& other) 
  : TObject(other),
  hit_x(other.hit_x),
  hit_y(other.hit_y),
  hit_z(other.hit_z),
  hit_q(other.hit_q),
  hit_qtrue(other.hit_qtrue),
  hit_tpc(other.hit_tpc) {}

hitvarContainer::hitvarContainer(hitvarContainer&& other) noexcept
  : TObject(std::move(other)),
  hit_x(std::move(other.hit_x)),
  hit_y(std::move(other.hit_y)),
  hit_z(std::move(other.hit_z)),
  hit_q(std::move(other.hit_q)),
  hit_qtrue(std::move(other.hit_qtrue)),
  hit_tpc(std::move(other.hit_tpc)) {}

hitvarContainer& hitvarContainer::operator=(const hitvarContainer& other) {
  if (this != &other) {
    hit_x = other.hit_x;
    hit_y = other.hit_y;
    hit_z = other.hit_z;
    hit_q = other.hit_q;
    hit_qtrue = other.hit_qtrue;
    hit_tpc = other.hit_tpc;

    TObject::operator=(other);
  }
  return *this;
}

bool hitvarContainer::operator==(const hitvarContainer& other) const {
  return hit_x == other.hit_x &&
    hit_y == other.hit_y &&
    hit_z == other.hit_z &&
    hit_q == other.hit_q &&
    hit_qtrue == other.hit_qtrue &&
    hit_tpc == other.hit_tpc;
}

bool hitvarContainer::operator<(const hitvarContainer& other) const {
  float qtot = get_total_charge();
  float other_qtot = other.get_total_charge();
  return qtot < other_qtot;
}

hitvarContainerPtr::hitvarContainerPtr() :
  TObject(), 
  hit_x(new std::vector<Float_t>),
  hit_y(new std::vector<Float_t>),
  hit_z(new std::vector<Float_t>),
  hit_q(new std::vector<Float_t>),
  hit_qtrue(new std::vector<Float_t>),
  hit_tpc(new std::vector<Int_t>) {}

hitvarContainerPtr::hitvarContainerPtr(hitvarContainer& container) :
  TObject(container), 
  hit_x(&container.hit_x),
  hit_y(&container.hit_y),
  hit_z(&container.hit_z),
  hit_q(&container.hit_q),
  hit_qtrue(&container.hit_qtrue),
  hit_tpc(&container.hit_tpc) {}
}
