/**
 * @author      : Luca_pi_mistral-large-4
 * @file        : geometry.hh
 * @brief       : Simplified 2D-surface geometry model for the LMFast fast light core
 *
 * A detector box is described by its boundaries plus kNFaces surface
 * collections (one per box face, fixed order). Each surface patch is stored
 * in 2D: the out-of-face coordinate is implied by the face index and the box
 * boundaries, so it is never stored per surface.
 */

#ifndef LMFAST_GEOMETRY_HH
#define LMFAST_GEOMETRY_HH

#include <vector>

// Box faces in the fixed order used everywhere a face index appears:
//   0: -X, 1: +X, 2: -Y, 3: +Y, 4: -Z, 5: +Z
// Face f has constant axis FaceAxis(f) (0=x, 1=y, 2=z) and side FaceSide(f)
// (0 = min, 1 = max); the constant (out-of-face) coordinate of face f is
// FaceConstant(boundaries, f).
enum EFace { kFaceXMin = 0, kFaceXMax, kFaceYMin, kFaceYMax, kFaceZMin, kFaceZMax, kNFaces };

constexpr int FaceAxis(int f) { return f / 2; }  // constant axis of face f
constexpr int FaceSide(int f) { return f % 2; }  // 0 = min side, 1 = max side

void FaceInAxes(int f, int& uAxis, int& vAxis); // in-face axes (the two != FaceAxis(f)), ascending
double FaceConstant(const double (&boundaries)[3][2], int f); // out-of-face coordinate of face f
void ExpandSurfacePoint(const double (&boundaries)[3][2], int face, double u, double v, double out[3]); // in-face (u,v) -> 3D point on the face

// 2D surface patch on a box face. (u, v) are the in-face coordinates, i.e. the
// two axes different from FaceAxis(face), in ascending order (see FaceInAxes).
// id tags the detector module the patch belongs to (set by the geometry
// importer); -1 = bare filler surface.
struct surface
  {
    double uMin, uMax, vMin, vMax;
    int id = -1;
  };

struct surfaceCollection  // patches of one box face; the face is the collection's index in geometry::surfColl
  {
    std::vector<surface> surf;
  };

struct geometry  // simplified detector geometry: kNFaces surface collections in the fixed face order
  {
    std::vector<surfaceCollection> surfColl;
  };

void PrintSurface(int face, const surface& surf);
void PrintGeometry(const geometry& geo, const double (&boundaries)[3][2]);

// Segment baseGeo into workGeo for the given active point: every face (except
// the one containing point) is split into cells at the projected origin /
// point / surface boundaries; holes are emitted too, tagged id = -1.
void SegmentGeometry(geometry& baseGeo, geometry& workGeo,
                     const double (&boundaries)[3][2],
                     const double (&origin)[3],
                     const double (&point)[3]);

#endif
