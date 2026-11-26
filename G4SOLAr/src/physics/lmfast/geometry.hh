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

#include <map>
#include <string>
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

// ---------------------------------------------------------------------------
// Translation from the active ENUDET-sim geometry to the simplified FLS one.
// NOTE: this part of geometry.cc depends on ENUDET-sim / Geant4 headers, so
// geometry.cc is compiled into solar_sim, NOT into the pure SLArLMFast lib.
// The header itself stays G4-free (forward declaration only).
// ---------------------------------------------------------------------------

class SLArDetectorConstruction; // ENUDET-sim detector construction (translator input)

enum EFLSModuleKind { kFLSPDSArray = 0, kFLSAnode = 1 };

struct FLSModuleRef
  {
    EFLSModuleKind kind;   //!< what the tagged surface belongs to
    int tpcId;             //!< TPC the module belongs to
    int moduleId;          //!< parent module id in the ENUDET-sim config (SCArray / anode id)
    int elementIdx = -1;   //!< index of the base element inside the parent (supercell / readout tile)
  };

// Simplified fast-light-sim geometry of one volume (one TPC box): box boundaries
// (detector frame, mm) plus the 2D surface patches of its kNFaces faces.
struct FLSBox
  {
    double boundaries[3][2];                 //!< [axis][min,max], detector frame, mm
    geometry geo;                            //!< kNFaces surface collections, fixed face order
    std::map<int, FLSModuleRef> moduleTags;  //!< surface.id -> owning module (id < 0: filler)
    int tpcId = -1;                          //!< TPC this box belongs to (-1 in config mode)
  };

// Translator: builds the per-volume FLSBox map from the running ENUDET-sim
// detector construction (per-TPC boxes; PDS SCArray + anode patches projected
// onto the box faces; remaining face area filled with a regular grid of bare
// filler surfaces), or from an explicit config box (testing override).
class flsGeometry
  {
    public:
      flsGeometry() = default;

      //! Build one FLSBox per TPC of the detector (boxes only; tub -> fatal).
      //! Detector frame = LAr-target-local frame, mm (same frame as the emission points).
      void BuildFromDetector(const SLArDetectorConstruction* det, double binSize);
      //! Testing override: a single box from explicit boundaries, filler-only faces.
      void BuildFromConfig(const double (&boundaries)[3][2], double binSize);

      const FLSBox* GetBox(const std::string& volumeName) const;
      const FLSBox* FindBoxContaining(const double (&p)[3]) const;
      const std::map<std::string, FLSBox>& Boxes() const { return fBoxes; }
      void Print() const;

    private:
      void BuildTPC(const SLArDetectorConstruction* det, int tpcId, double binSize);
      //! Project one axis-aligned base module (supercell / readout tile) onto the
      //! nearest box face. center/msize are in the detector frame [mm].
      void AddModule(FLSBox& box, int tpcId,
                     const double (&center)[3], const double (&msize)[3],
                     double phi, double theta, double psi,
                     EFLSModuleKind kind, int moduleId, int elementIdx);
      //! Fill the face area not covered by detector patches with a regular
      //! grid of bare filler surfaces (id = -1).
      void FillFaces(FLSBox& box);

      std::map<std::string, FLSBox> fBoxes; //!< volume name ("TPC<id>", or "default" in config mode) -> box
      double fBinSize = 100.;               //!< filler surface grid bin [mm]
      int    fNextId  = 1;                   //!< next detector-patch tag id
  };

#endif
