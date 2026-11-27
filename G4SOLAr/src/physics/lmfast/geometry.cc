/**
 * @author      : Luca_pi_mistral-large-4
 * @file        : geometry.cc
 * @brief       : Simplified 2D-surface geometry model for the LMFast fast light core
 */

#include "geometry.hh"

#include <algorithm>
#include <cmath>
#include <iostream>

// --- translator from the ENUDET-sim geometry (compiled into solar_sim, NOT into
// --- the pure SLArLMFast lib, because of these ENUDET-sim / Geant4 / ROOT headers)
#include "geo/detector/SLArDetectorConstruction.hh"
#include "geo/detector/TPC/SLArDetTPC.hh"
#include "geo/SLArGeoUtils.hh"
#include "analysis/SLArAnalysisManager.hh"
#include "config/SLArCfgSuperCellArray.hh"
#include "config/SLArCfgAnode.hh"

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Exception.hh"
#include "G4ios.hh"

void FaceInAxes(int f, int& uAxis, int& vAxis)
{
  const int axis = FaceAxis(f);
  uAxis = (axis == 0) ? 1 : 0;
  vAxis = (axis == 2) ? 1 : 2;
}

double FaceConstant(const double (&boundaries)[3][2], int f)
{
  return boundaries[FaceAxis(f)][FaceSide(f)];
}

void ExpandSurfacePoint(const double (&boundaries)[3][2], int face, double u, double v, double out[3])
{
  int uAxis, vAxis;
  FaceInAxes(face, uAxis, vAxis);
  out[FaceAxis(face)] = FaceConstant(boundaries, face);
  out[uAxis] = u;
  out[vAxis] = v;
}

void PrintSurface(int face, const surface& surf)
{
  std::cout << "\tface " << face << " (axis " << FaceAxis(face)
            << (FaceSide(face) ? "+" : "-") << ")"
            << ", u [" << surf.uMin << ", " << surf.uMax << "]"
            << ", v [" << surf.vMin << ", " << surf.vMax << "]"
            << ", id " << surf.id << "\n";
}

void PrintGeometry(const geometry& geo, const double (&boundaries)[3][2])
{
  std::cout << "\n";
  for( int f=0; f<(int)geo.surfColl.size(); f++)
  {
    std::cout << "Surface collection " << f << " (face axis " << FaceAxis(f)
              << ", " << (FaceSide(f) ? "max" : "min") << ", const coord "
              << FaceConstant(boundaries, f) << ")\n";

    for( int j=0; j<(int)geo.surfColl.at(f).surf.size(); j++ )
    {
      std::cout << "\tSurface number " << j << "\n";
      PrintSurface( f, geo.surfColl.at(f).surf.at(j) );
    }
  }
}

void SegmentGeometry(geometry& baseGeo, geometry& workGeo,
                     const double (&boundaries)[3][2],
                     const double (&origin)[3],
                     const double (&point)[3]) // need 2?: shifted and reflected light
{
  //  > exclude detector face on which the active point lies

  //  > for each of the other 5 faces

  //    > divide surface in 6 sectors finding the local maxima of visibility (origin-surface*surface-active_point)
  //      (looking for it close to the projection of the light origin and active point)

  //    > get a rough gradient in each sector and segment the base geometry surfaces
  //      > or based on available parameters (light origin and others)? Known in advance -> no need of sampling

  //  > 30 sectors  ...~n of surface segments? better many segments and fewer GL nodes or vice versa? depends on ^


  // same for shifter (isotropic or Lambertian?) and reflection?

  // avoid computing sufrace segments giving a negligible contribution? => fill geometry?


  constexpr double eps = 1e-9;

  // Face ordering: fixed convention, face f -> axis FaceAxis(f), side
  // FaceSide(f) (0: -X, 1: +X, 2: -Y, 3: +Y, 4: -Z, 5: +Z).

  workGeo.surfColl.clear();

  if (baseGeo.surfColl.size() != kNFaces)
    return;

  workGeo.surfColl.resize(kNFaces);

  auto near = [](double a, double b) {
    return std::abs(a - b) <= eps;
  };

  // Surfaces are 2D: in-face coordinates are (u, v); the out-of-face
  // coordinate is implied by the face index (FaceConstant) and not stored.
  // In-face axes of face f are the two axes != FaceAxis(f), ascending.

  // Locate the face containing point.
  int activeFace = -1;

  for (int f = 0; f < kNFaces; ++f) {
    const int axis = FaceAxis(f);
    int uAxis, vAxis;
    FaceInAxes(f, uAxis, vAxis);

    if (!near(point[axis], FaceConstant(boundaries, f)))
      continue;

    if (point[uAxis] < boundaries[uAxis][0] - eps ||
        point[uAxis] > boundaries[uAxis][1] + eps ||
        point[vAxis] < boundaries[vAxis][0] - eps ||
        point[vAxis] > boundaries[vAxis][1] + eps)
        continue;

    activeFace = f;
    break;
  }

  if (activeFace < 0)
    return;

  // Add a coordinate if it is inside the given face bounds.
  auto addCut = [&](std::vector<double>& cuts,
                  double x, double lo, double hi) {
    if (x > lo + eps && x < hi - eps)
      cuts.push_back(x);
  };

  auto normalizeCuts = [&](std::vector<double>& cuts) {
    std::sort(cuts.begin(), cuts.end());

    std::vector<double> uniqueCuts;
    for (double x : cuts) {
      if (uniqueCuts.empty() ||
          std::abs(x - uniqueCuts.back()) > eps)
          uniqueCuts.push_back(x);
    }

    cuts.swap(uniqueCuts);
  };

  // Process every face except the active one.
  for (int f = 0; f < kNFaces; ++f) {
    if (f == activeFace)
      continue;

    int uAxis, vAxis;
    FaceInAxes(f, uAxis, vAxis);

    const double uMin = boundaries[uAxis][0];
    const double uMax = boundaries[uAxis][1];
    const double vMin = boundaries[vAxis][0];
    const double vMax = boundaries[vAxis][1];

    if (uMax - uMin <= eps || vMax - vMin <= eps)
      return;

    // Begin with the full face boundaries.
    std::vector<double> uCuts{uMin, uMax};
    std::vector<double> vCuts{vMin, vMax};

    // Add the projected coordinates of origin and active point.
    // Points outside the face do not create cuts.
    addCut(uCuts, origin[uAxis], uMin, uMax);
    addCut(uCuts, point[uAxis], uMin, uMax);
    addCut(vCuts, origin[vAxis], vMin, vMax);
    addCut(vCuts, point[vAxis], vMin, vMax);

    // Add all original surface boundaries, clipped to the box.
    // (every surface of collection f lies on face f by construction)
    for (const auto& src : baseGeo.surfColl[f].surf) {
      addCut(uCuts, src.uMin, uMin, uMax);
      addCut(uCuts, src.uMax, uMin, uMax);
      addCut(vCuts, src.vMin, vMin, vMax);
      addCut(vCuts, src.vMax, vMin, vMax);
    }

    normalizeCuts(uCuts);
    normalizeCuts(vCuts);

    auto& out = workGeo.surfColl[f].surf;

    // Build cells across the entire face. This fills holes because
    // every cell not covered by an original rectangle is also emitted.
    for (std::size_t i = 0; i + 1 < uCuts.size(); ++i) {
      for (std::size_t j = 0; j + 1 < vCuts.size(); ++j) {
        const double u0 = uCuts[i];
        const double u1 = uCuts[i + 1];
        const double v0 = vCuts[j];
        const double v1 = vCuts[j + 1];

        if (u1 - u0 <= eps || v1 - v0 <= eps)
            continue;

        const double um = 0.5 * (u0 + u1);
        const double vm = 0.5 * (v0 + v1);

        // Find whether this cell is covered by an original
        // rectangle. Because all original boundaries are cuts,
        // testing the midpoint is sufficient for valid input.
        int coverId = -1;

        for (const auto& src : baseGeo.surfColl[f].surf) {
          if (um >= src.uMin - eps &&
              um <= src.uMax + eps &&
              vm >= src.vMin - eps &&
              vm <= src.vMax + eps) {
              coverId = src.id;
              break;
          }
        }

        // Create a 2D rectangle for every cell, whether it was
        // covered before or represents a previously empty hole.
        surface piece{};
        piece.uMin = u0; piece.uMax = u1;
        piece.vMin = v0; piece.vMax = v1;
        piece.id = coverId; // -1: hole (bare filler), else the covering module's tag

        out.push_back(piece);
      }
    }
  }
}

// ---------------------------------------------------------------------------
// flsGeometry: translation from the active ENUDET-sim geometry
// ---------------------------------------------------------------------------

namespace {

struct Rect2D { double u0, u1, v0, v1; };

bool intersect2D(const Rect2D& a, const Rect2D& b)
{
  return a.u0 < b.u1 && a.u1 > b.u0 && a.v0 < b.v1 && a.v1 > b.v0;
}

// remove patch p from the list of empty rectangles (axis-aligned split)
void subtractRect(std::vector<Rect2D>& rects, const Rect2D& p)
{
  std::vector<Rect2D> out;
  for (const auto& r : rects) {
    if (!intersect2D(r, p)) { out.push_back(r); continue; }
    if (p.u0 > r.u0) out.push_back({ r.u0, p.u0, r.v0, r.v1 });
    if (p.u1 < r.u1) out.push_back({ p.u1, r.u1, r.v0, r.v1 });
    const double u0 = std::max(r.u0, p.u0), u1 = std::min(r.u1, p.u1);
    if (p.v0 > r.v0) out.push_back({ u0, u1, r.v0, p.v0 });
    if (p.v1 < r.v1) out.push_back({ u0, u1, p.v1, r.v1 });
  }
  rects.swap(out);
}

// Build the rotation from cfg Euler angles. The angles may be radians or degrees
// depending on the loader path; accept whichever gives an axis-aligned rotation.
// Tolerance 1e-3: cfg angles are stored rounded (e.g. -1.5708 for -pi/2), so
// off-axis components of a 90-degree rotation can be ~4e-6, not 0.
bool flsRotation(double phi, double theta, double psi, G4RotationMatrix& R)
{
  auto isAxisAligned = [](const G4RotationMatrix& M) {
    for (int i = 0; i < 3; i++) {
      double ev[3] = { 0., 0., 0. };
      ev[i] = 1.;
      const G4ThreeVector col = M * G4ThreeVector(ev[0], ev[1], ev[2]);
      int dominant = 0;
      for (int a = 0; a < 3; a++)
        if (std::fabs(col[a]) > 1. - 1e-3) dominant++;
        else if (std::fabs(col[a]) > 1e-3) return false;
      if (dominant != 1) return false;
    }
    return true;
  };
  for (int attempt = 0; attempt < 2; attempt++) {
    const double conv = (attempt == 0) ? 1.0 : M_PI / 180.0; // radians, then degrees
    R = G4RotationMatrix(phi * conv, theta * conv, psi * conv);
    if (isAxisAligned(R)) {
      // G4PVPlacement applies the INVERSE of the given rotation to local
      // coordinates (G4PVPlacement.cc stores getRotation().inverse()), so the
      // world-frame extents/offsets of a placed module use R^-1 = R^T
      R = R.inverse();
      return true;
    }
  }
  return false;
}

} // namespace

void flsGeometry::BuildFromDetector(const SLArDetectorConstruction* det, double binSize)
{
  fBinSize = binSize;
  fBoxes.clear();
  fNextId = 1;

  if (!det) {
    G4Exception("flsGeometry::BuildFromDetector", "FLSGeo000", JustWarning,
        "Null detector construction: the NewModel FLS geometry is empty.");
    return;
  }

  auto* anaMgr = SLArAnalysisManager::Instance();

  // pass 1: one FLS box per TPC
  for (const auto& kv : det->GetDetTPCs()) {
    const int tpcId = kv.first;
    const auto* tpc = kv.second;
    if (tpc->GetShape() != geo::kBox) {
      G4Exception("flsGeometry::BuildFromDetector", "FLSGeo001", FatalException,
          "Only box-shaped TPC volumes are supported by the NewModel fast light sim "
          "geometry (cylindrical/tub volumes are not supported yet).");
    }
    BuildTPC(det, tpcId, binSize);
  }

  // base modules collected first; the box boundaries are then refined from their
  // surface planes (pass 2.5) so that the FLS box is delimited by the PDS/anode
  // module surfaces, and the patches land exactly on the box faces
  struct PendingModule {
    FLSBox* box;
    int tpcId;
    double c[3];          // element center, detector frame (patch placement)
    double msize[3];
    double phi, theta, psi;
    EFLSModuleKind kind;
    int moduleId;
    int elementIdx;
    int face;             // nearest face of the element w.r.t. the provisional box
    double parentPlane[3]; // parent module (array / anode) center, detector frame
    int parentFace;       // nearest face of the parent w.r.t. the provisional box
  };
  std::vector<PendingModule> pending;

  auto nearestFace = [](const FLSBox& box, const double (&c)[3]) {
    int axis = 0, side = 0;
    double best = 1e300;
    for (int a = 0; a < 3; a++) {
      // absolute distance to the face plane: points outside the box must still be
      // assigned to the face they lie beyond (not to an interior face)
      const double d0 = std::fabs(c[a] - box.boundaries[a][0]);
      const double d1 = std::fabs(box.boundaries[a][1] - c[a]);
      const double d  = std::min(d0, d1);
      if (d < best) { best = d; axis = a; side = (d0 < d1) ? 0 : 1; }
    }
    return 2 * axis + side;
  };

  // pass 2: PDS — one surface patch per SUPER CELL (base module), not per array.
  // NOTE: the SCArray cfgs do not carry the tpcID (BuildOpDetArrayCfg never calls
  // SetTPCID), so associate each array to the box containing its physical position.
  // GetPhysX/Y/Z = tpc_center + local pos, i.e. detector-frame coords (box targets
  // have zero rotation). Super-cell positions are array-local; rotate by the array
  // rotation and add the array center.
  for (auto& kv : anaMgr->GetPDSCfg().GetMap()) {
    auto& arr = kv.second;
    const double ac[3] = { arr.GetPhysX(), arr.GetPhysY(), arr.GetPhysZ() };
    FLSBox* box = const_cast<FLSBox*>(FindBoxContaining(ac));
    if (!box) {
      G4cout << "flsGeometry: WARNING - PDS array " << arr.GetID()
             << " at (" << ac[0] << ", " << ac[1] << ", " << ac[2]
             << ") mm is not inside any TPC box; skipped." << G4endl;
      continue;
    }
    G4RotationMatrix Rarr;
    if (!flsRotation(arr.GetPhi(), arr.GetTheta(), arr.GetPsi(), Rarr)) {
      G4cout << "flsGeometry: WARNING - PDS array " << arr.GetID()
             << " is not axis-aligned (phi/theta/psi = " << arr.GetPhi() << ", "
             << arr.GetTheta() << ", " << arr.GetPsi() << "); skipped." << G4endl;
      continue;
    }
    int cellIdx = 0;
    for (const auto& cell : arr.GetConstMap()) {
      const G4ThreeVector cc = Rarr * G4ThreeVector(cell.GetX(), cell.GetY(), cell.GetZ());
      const double c[3]     = { ac[0] + cc.x(), ac[1] + cc.y(), ac[2] + cc.z() };
      const double msize[3] = { cell.GetSizeX(), cell.GetSizeY(), cell.GetSizeZ() };
      pending.push_back({ box, box->tpcId, { c[0], c[1], c[2] }, { msize[0], msize[1], msize[2] },
                          arr.GetPhi(), arr.GetTheta(), arr.GetPsi(),
                          kFLSPDSArray, arr.GetID(), cellIdx, nearestFace(*box, c),
                          { ac[0], ac[1], ac[2] }, nearestFace(*box, ac) });
      cellIdx++;
    }
  }

  // pass 3: anodes — one surface patch per READOUT TILE (base module).
  // Anode cfg map is keyed by tpcID; positions chain anode-local: anode + megatile
  // + tile. NOTE: the anode PV rotation acts about the PV's own origin, so only the
  // sub-element offset gets rotated, not the anode position itself.
  for (auto& kv : anaMgr->GetAnodeCfg()) {
    const int tpcId = kv.first;
    auto& anode = kv.second;
    auto bit = fBoxes.find("TPC" + std::to_string(tpcId));
    if (bit == fBoxes.end()) continue;
    auto* tpc = det->GetDetTPCs().at(tpcId);
    auto* gi  = tpc->GetGeoInfo();
    const double tpcCenter[3] = { gi->GetGeoPar("tpc_pos_x"),
                                  gi->GetGeoPar("tpc_pos_y"),
                                  gi->GetGeoPar("tpc_pos_z") };
    const double ap[3] = { anode.GetX(), anode.GetY(), anode.GetZ() };
    G4RotationMatrix Ra;
    if (!flsRotation(anode.GetPhi(), anode.GetTheta(), anode.GetPsi(), Ra)) {
      G4cout << "flsGeometry: WARNING - anode of TPC " << tpcId
             << " is not axis-aligned; skipped." << G4endl;
      continue;
    }
    const int anodeId = anode.GetID() >= 0 ? anode.GetID() : tpcId;
    int tileIdx = 0;
    for (const auto& mt : anode.GetConstMap()) {
      for (const auto& tile : mt.GetConstMap()) {
        const G4ThreeVector cc = Ra * G4ThreeVector(mt.GetX() + tile.GetX(),
                                                    mt.GetY() + tile.GetY(),
                                                    mt.GetZ() + tile.GetZ());
        const double c[3]     = { tpcCenter[0] + ap[0] + cc.x(),
                                  tpcCenter[1] + ap[1] + cc.y(),
                                  tpcCenter[2] + ap[2] + cc.z() };
        const double msize[3] = { tile.GetSizeX(), tile.GetSizeY(), tile.GetSizeZ() };
        const double apc[3] = { tpcCenter[0] + ap[0], tpcCenter[1] + ap[1], tpcCenter[2] + ap[2] };
        pending.push_back({ &bit->second, tpcId, { c[0], c[1], c[2] }, { msize[0], msize[1], msize[2] },
                            anode.GetPhi(), anode.GetTheta(), anode.GetPsi(),
                            kFLSAnode, anodeId, tileIdx, nearestFace(bit->second, c),
                            { apc[0], apc[1], apc[2] }, nearestFace(bit->second, apc) });
        tileIdx++;
      }
    }
  }

  // pass 2.5: refine the box boundaries from the PDS/anode PARENT surface planes
  // (array / anode center along the face axis), not from individual elements whose
  // rotated positions can scatter: for each face, the boundary is the parent module
  // plane closest to the box center (faces without any module keep the TPC boundary)
  for (auto& bkv : fBoxes) {
    FLSBox& box = bkv.second;
    for (int a = 0; a < 3; a++) {
      for (int side = 0; side < 2; side++) {
        const int f = 2 * a + side;
        bool found = false;
        double plane = 0.;
        for (const auto& m : pending) {
          if (m.box != &box || m.parentFace != f) continue;
          if (m.parentPlane[a] < box.boundaries[a][0] || m.parentPlane[a] > box.boundaries[a][1]) continue;
          if (!found || (side == 0 ? m.parentPlane[a] > plane : m.parentPlane[a] < plane)) {
            plane = m.parentPlane[a];
            found = true;
          }
        }
        if (found) box.boundaries[a][side] = plane;
      }
    }
  }

  // pass 4: add all modules (patches land exactly on the refined faces)
  for (const auto& m : pending) {
    AddModule(*m.box, m.tpcId, m.c, m.msize, m.phi, m.theta, m.psi,
              m.kind, m.moduleId, m.elementIdx);
  }

  // pass 5: filler surfaces on all faces
  for (auto& kv : fBoxes) FillFaces(kv.second);

  G4cout << "flsGeometry: built " << fBoxes.size()
         << " volume box(es) from the detector construction" << G4endl;
}

void flsGeometry::BuildFromConfig(const double (&boundaries)[3][2], double binSize)
{
  fBinSize = binSize;
  fBoxes.clear();
  fNextId = 1;

  FLSBox box;
  for (int a = 0; a < 3; a++) {
    box.boundaries[a][0] = boundaries[a][0];
    box.boundaries[a][1] = boundaries[a][1];
  }
  box.geo.surfColl.resize(kNFaces);
  FillFaces(box); // filler only, no detector patches (testing override)

  fBoxes["default"] = std::move(box);
  G4cout << "flsGeometry: built 1 config box (filler-only faces)" << G4endl;
}

void flsGeometry::BuildTPC(const SLArDetectorConstruction* det, int tpcId, double binSize)
{
  auto* tpc = det->GetDetTPCs().at(tpcId);
  auto* gi  = tpc->GetGeoInfo();

  // TPC PVs are daughters of the LAr target placed at tpc_pos (target-local);
  // the detector frame = target-local frame, so tpc_pos IS the box center there.
  const double tpcCenter[3] = { gi->GetGeoPar("tpc_pos_x"),
                                gi->GetGeoPar("tpc_pos_y"),
                                gi->GetGeoPar("tpc_pos_z") };
  const double tpcSize[3]   = { gi->GetGeoPar("tpc_x"),
                                gi->GetGeoPar("tpc_y"),
                                gi->GetGeoPar("tpc_z") };

  FLSBox box;
  box.tpcId = tpcId;
  for (int a = 0; a < 3; a++) {
    box.boundaries[a][0] = tpcCenter[a] - 0.5 * tpcSize[a];
    box.boundaries[a][1] = tpcCenter[a] + 0.5 * tpcSize[a];
  }
  box.geo.surfColl.resize(kNFaces);

  fBoxes["TPC" + std::to_string(tpcId)] = std::move(box);
}

void flsGeometry::AddModule(FLSBox& box, int tpcId,
                            const double (&c)[3], const double (&msize)[3],
                            double phi, double theta, double psi,
                            EFLSModuleKind kind, int moduleId, int elementIdx)
{
  G4RotationMatrix R;
  if (!flsRotation(phi, theta, psi, R)) {
    G4cout << "flsGeometry::AddModule: WARNING - module " << moduleId << " (TPC " << tpcId
           << ", element " << elementIdx << ") is not axis-aligned (phi/theta/psi = "
           << phi << ", " << theta << ", " << psi
           << "); oblique modules are not supported, skipped." << G4endl;
    return;
  }

  // world(=detector-frame) half extents of the module box
  const G4ThreeVector h(msize[0] / 2, msize[1] / 2, msize[2] / 2);
  const G4ThreeVector hw = R * h; // aligned R => permuted half sizes
  const double hwv[3] = { std::fabs(hw.x()), std::fabs(hw.y()), std::fabs(hw.z()) };

  double bmin[3], bmax[3];
  for (int a = 0; a < 3; a++) {
    bmin[a] = c[a] - hwv[a];
    bmax[a] = c[a] + hwv[a];
  }

  // nearest box face (the one the module is mounted on)
  int axis = -1, side = -1;
  double best = 1e300;
  for (int a = 0; a < 3; a++) {
    const double d0 = std::fabs(c[a] - box.boundaries[a][0]);
    const double d1 = std::fabs(box.boundaries[a][1] - c[a]);
    const double d  = std::min(d0, d1);
    if (d < best) { best = d; axis = a; side = (d0 < d1) ? 0 : 1; }
  }
  if (best > 0.1 * (box.boundaries[axis][1] - box.boundaries[axis][0])) {
    G4cout << "flsGeometry::AddModule: WARNING - module " << moduleId << " (TPC " << tpcId
           << ") is not close to any box face (dist " << best << " mm); skipped." << G4endl;
    return;
  }

  const int f = 2 * axis + side; // fixed face order: face = 2*axis + side
  int uAxis, vAxis;
  FaceInAxes(f, uAxis, vAxis);

  // project the module rectangle onto the face (out-of-face coord dropped)
  surface s;
  s.uMin = std::max(bmin[uAxis], box.boundaries[uAxis][0]);
  s.uMax = std::min(bmax[uAxis], box.boundaries[uAxis][1]);
  s.vMin = std::max(bmin[vAxis], box.boundaries[vAxis][0]);
  s.vMax = std::min(bmax[vAxis], box.boundaries[vAxis][1]);
  if (s.uMax - s.uMin <= 0. || s.vMax - s.vMin <= 0.) {
    G4cout << "flsGeometry::AddModule: WARNING - module " << moduleId << " (TPC " << tpcId
           << ") projects to an empty face area; skipped." << G4endl;
    return;
  }
  s.id = fNextId++;
  box.geo.surfColl[f].surf.push_back(s);
  box.moduleTags[s.id] = FLSModuleRef{ kind, tpcId, moduleId, elementIdx };
}

void flsGeometry::FillFaces(FLSBox& box)
{
  for (int f = 0; f < kNFaces; f++) {
    int uAxis, vAxis;
    FaceInAxes(f, uAxis, vAxis);

    const Rect2D face{ box.boundaries[uAxis][0], box.boundaries[uAxis][1],
                       box.boundaries[vAxis][0], box.boundaries[vAxis][1] };

    // empty area = face minus the detector patches already in the collection
    std::vector<Rect2D> empty{ face };
    for (const auto& s : box.geo.surfColl[f].surf)
      subtractRect(empty, { s.uMin, s.uMax, s.vMin, s.vMax });

    // subdivide the empty area into a regular grid of bare filler surfaces
    for (const auto& r : empty) {
      const long nu = std::max(1L, (long)std::ceil((r.u1 - r.u0) / fBinSize));
      const long nv = std::max(1L, (long)std::ceil((r.v1 - r.v0) / fBinSize));
      const double du = (r.u1 - r.u0) / nu;
      const double dv = (r.v1 - r.v0) / nv;
      for (long i = 0; i < nu; i++) {
        for (long j = 0; j < nv; j++) {
          surface s;
          s.uMin = r.u0 + i * du; s.uMax = s.uMin + du;
          s.vMin = r.v0 + j * dv; s.vMax = s.vMin + dv;
          s.id = -1; // bare filler (absorbing for now; optical properties TODO)
          box.geo.surfColl[f].surf.push_back(s);
        }
      }
    }
  }
}

const FLSBox* flsGeometry::GetBox(const std::string& volumeName) const
{
  const auto it = fBoxes.find(volumeName);
  return it != fBoxes.end() ? &it->second : nullptr;
}

const FLSBox* flsGeometry::FindBoxContaining(const double (&p)[3]) const
{
  for (const auto& kv : fBoxes) {
    const FLSBox& b = kv.second;
    bool inside = true;
    for (int a = 0; a < 3; a++)
      if (p[a] < b.boundaries[a][0] || p[a] > b.boundaries[a][1]) { inside = false; break; }
    if (inside) return &b;
  }
  return nullptr;
}

void flsGeometry::Print() const
{
  for (const auto& kv : fBoxes) {
    const FLSBox& b = kv.second;
    printf("FLS box '%s' [mm]: x [%.1f, %.1f], y [%.1f, %.1f], z [%.1f, %.1f]\n",
        kv.first.c_str(),
        b.boundaries[0][0], b.boundaries[0][1],
        b.boundaries[1][0], b.boundaries[1][1],
        b.boundaries[2][0], b.boundaries[2][1]);
    for (int f = 0; f < kNFaces && f < (int)b.geo.surfColl.size(); f++) {
      size_t nTagged = 0;
      for (const auto& s : b.geo.surfColl[f].surf)
        if (s.id >= 0) nTagged++;
      printf("  face %d (axis %d, %s): %zu surfaces (%zu detector patches, %zu filler)\n",
          f, FaceAxis(f), FaceSide(f) ? "max" : "min",
          b.geo.surfColl[f].surf.size(), nTagged, b.geo.surfColl[f].surf.size() - nTagged);
    }
  }
}
