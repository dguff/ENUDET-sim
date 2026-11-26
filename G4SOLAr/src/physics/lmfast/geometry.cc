/**
 * @author      : Luca_pi_mistral-large-4
 * @file        : geometry.cc
 * @brief       : Simplified 2D-surface geometry model for the LMFast fast light core
 */

#include "geometry.hh"

#include <algorithm>
#include <cmath>
#include <iostream>

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
