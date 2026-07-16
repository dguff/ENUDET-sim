/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEveCRTRenderer
 * @created     : Thursday Jul 16, 2026 23:07:27 CEST
 */

#include "SLArEveCRTRenderer.hh"

#include "event/SLArEventCRT.hh"

namespace display {
SLArEveCRTRenderer::SLArEveCRTRenderer(const SLArEveGeometry& geometry)
  : fGeometry(geometry)
{}

void SLArEveCRTRenderer::Configure() {
  for (auto& [id, crt_module] : fGeometry.GetCRTModules()) {
    const int crt_id = id;
    const TString name = Form("crtHit_%i", crt_id);
    const TString titl = Form("CRT module %i hits", crt_id);

    printf("SLArEveCRTRenderer: configuring CRT module %i\n", crt_id);

    auto bs_crt = std::make_unique<TEveBoxSet>(name, titl);
    bs_crt->Reset(TEveBoxSet::kBT_AABox, false, 100);
    bs_crt->SetRenderMode(TEveBoxSet::kRM_Fill);

    crt_module.fVolume->AddElement(bs_crt.get());

    fCRTHits.emplace(crt_id, std::move(bs_crt));
  }
}

void SLArEveCRTRenderer::Reset() {
  for (auto& [id, bs] : fCRTHits) {
    bs->Reset(TEveBoxSet::kBT_AABox, false, 10000);
  }
}

void SLArEveCRTRenderer::RenderCRTHits(const SLArListEventCRT* ev_crt_list) {
  if (!ev_crt_list) return;

  for (const auto& crt_ev: ev_crt_list->GetCRTHits())  {

    const int crt_id = crt_ev.GetCRTNo();
    if (fCRTHits.find(crt_id) == fCRTHits.end()) continue;

    auto& bs_crt = fCRTHits.at(crt_id);

    double w = fHitSize, l = fHitSize, h = fHitSize;
    const auto& geo_crt = fGeometry.GetCRTModules().at(crt_id);

    if ( geo_crt.fDimension.x() < geo_crt.fDimension.y() && geo_crt.fDimension.x() < geo_crt.fDimension.z() ) {
      w = geo_crt.fDimension.x();
      if (crt_ev.GetLocalPos().x() > 0) w *= -1;
    }
    else if ( geo_crt.fDimension.y() < geo_crt.fDimension.x() && geo_crt.fDimension.y() < geo_crt.fDimension.z() ) {
      h = geo_crt.fDimension.y();
      if (crt_ev.GetLocalPos().y() > 0) h *= -1;
    }
    else {
      l = geo_crt.fDimension.z();
      if (crt_ev.GetLocalPos().z() > 0) l *= -1;
    }

    bs_crt->AddBox( 
        geo_crt.fPosition.x() + crt_ev.GetLocalPos().x(), 
        geo_crt.fPosition.y() + crt_ev.GetLocalPos().y(),
        geo_crt.fPosition.z() + crt_ev.GetLocalPos().z(),
        w, h, l ); 
    bs_crt->DigitValue( crt_ev.GetEkin() ); 
  }

  for (auto& crt_hitset_itr : fCRTHits) {
    crt_hitset_itr.second->RefitPlex();
  }

  return;
}

}
