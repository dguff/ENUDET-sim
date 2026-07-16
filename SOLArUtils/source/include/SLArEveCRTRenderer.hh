/**
 * @author      : Daniele Guffanti (University of Milano-Bicocca, INFN Milano-Bicocca)
 * @file        : SLArEveCRTRenderer.hh
 * @created     : Thursday Jul 16, 2026 12:19:03 CEST
 */

#ifndef SLAREVECRTRENDERER_HH

#define SLAREVECRTRENDERER_HH

#include "TEveBoxSet.h"
#include "event/SLArEventCRT.hh"
#include "SLArEveEventReader.hh"
#include "SLArEveGeometry.hh"

namespace display {

  class SLArEveCRTRenderer {
    public:
      explicit SLArEveCRTRenderer(const SLArEveGeometry& geometry);
      ~SLArEveCRTRenderer() = default;

      SLArEveCRTRenderer(const SLArEveCRTRenderer&) = delete;
      SLArEveCRTRenderer& operator=(const SLArEveCRTRenderer&) = delete;

      void Configure(); 

      void RenderCRTHits(const SLArListEventCRT* ev_crt_list);

      void Reset(); 

    private: 
      const SLArEveGeometry& fGeometry;
      std::map<int, std::unique_ptr<TEveBoxSet>> fCRTHits;

      double fHitSize = 100.0; //!< Transverse size of the CRT hit box in mm
  };

}

#endif /* end of include guard SLAREVECRTRENDERER_HH */
