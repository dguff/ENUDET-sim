/**
 * @author      : Luca_pi_mistral-large-4
 * @file        : SLArFLSNewModel.cc
 * @created     : Tuesday Nov 24, 2026 15:22:25 CET
 */

#include "physics/SLArFLSNewModel.hh"

#include "integrator.hh" // LMFast core, vendored in src/physics/lmfast
#include "geometry.hh"   // LMFast 2D geometry model + flsGeometry translator

#include "geo/detector/SLArDetectorConstruction.hh"

#include "G4Exception.hh"
#include "G4ios.hh"
#include "G4RunManager.hh"

#include "TString.h" // Form()

#include <cmath>

/**
 * Source the configuration of the NewModel module and build the simplified FLS geometry.
 *
 * Recognized (all optional) fields of the module config:
 *
 * | Field | Type | Default | Description |
 * |-------|------|---------|-------------|
 * | `geometry_source` | string | `"detector"` | `"detector"`: per-TPC boxes translated from the active detector; `"config"`: use `boundaries` (testing) |
 * | `boundaries` | array of 3 arrays of 2 numbers | LMFast PoC placeholder box | detector box `[[xl,xu],[yl,yu],[zl,zu]]` in mm; only used when `geometry_source == "config"` |
 * | `bin_size` | number | `100` | filler surface grid bin [mm] |
 * | `rs_grid_bin` | number | `100` | RSED grid spacing [mm], uniform in all 3 axes |
 * | `rs_enabled` | bool | `true` | enable Rayleigh scattering (RSED) in the LMFast core |
 *
 * @param config rapidjson configuration object of this module
 */
void SLArFLSNewModel::Initialize(const rapidjson::Value& config)
{
  if (!config.IsObject()) {
    G4Exception("SLArFLSNewModel::Initialize", "ConfigError", JustWarning,
        "NewModel module configuration is not a JSON object. Using defaults.");
  }

  // default box = LMFast PoC placeholder, used only in "config" mode
  double boundaries[3][2] = {{-11000., 11000.}, {-2000., 2000.}, {-2000., 2000.}};

  if (config.HasMember("geometry_source") && config["geometry_source"].IsString()) {
    fGeometrySource = config["geometry_source"].GetString();
  }

  // optional "boundaries": [[xl,xu],[yl,yu],[zl,zu]] in mm (config mode only)
  if (config.HasMember("boundaries")) {
    const auto& jb = config["boundaries"];
    if (jb.IsArray() && jb.Size() == 3) {
      for (int i = 0; i < 3; i++) {
        if (jb[i].IsArray() && jb[i].Size() == 2 &&
            jb[i][0].IsNumber() && jb[i][1].IsNumber()) {
          boundaries[i][0] = jb[i][0].GetDouble();
          boundaries[i][1] = jb[i][1].GetDouble();
        }
        else {
          G4Exception("SLArFLSNewModel::Initialize", "ConfigError", JustWarning,
              Form("Invalid 'boundaries' entry %d (expected [min, max] numbers). Keeping default.", i));
        }
      }
    }
    else {
      G4Exception("SLArFLSNewModel::Initialize", "ConfigError", JustWarning,
          "Invalid 'boundaries' (expected array of 3 [min, max] pairs). Keeping default placeholder box.");
    }
  }

  if (config.HasMember("bin_size") && config["bin_size"].IsNumber()) {
    fBinSize = config["bin_size"].GetDouble();
  }
  if (config.HasMember("rs_grid_bin") && config["rs_grid_bin"].IsNumber()) {
    fRSGridBin = config["rs_grid_bin"].GetDouble();
  }
  if (config.HasMember("rs_enabled") && config["rs_enabled"].IsBool()) {
    fRSEnabled = config["rs_enabled"].GetBool();
  }

  // Build the simplified FLS geometry. Initialize() runs in BeginOfRunAction, i.e. after
  // runManager->Initialize() -> Construct(), so the detector geometry is fully built.
  if (fGeometrySource == "config") {
    fGeometry.BuildFromConfig(boundaries, fBinSize);
  }
  else {
    const auto det = (const SLArDetectorConstruction*)
        G4RunManager::GetRunManager()->GetUserDetectorConstruction();
    fGeometry.BuildFromDetector(det, fBinSize);
  }

  G4cout << "SLArFLSNewModel::Initialize: NewModel fast light sim initialized with the LMFast core."
         << G4endl;
  Print();
}

/**
 * Compute (and cache) the visibility of every surface of the given FLS box for a given
 * emission point with the LMFast core (`integrator`, with the `RSED` grid if Rayleigh
 * scattering is enabled).
 *
 * The emission point arrives in the detector (LAr target) frame in G4 internal units (mm),
 * which is the same frame and unit used by the FLS geometry and the LMFast core.
 * Visibilities are stored per surface, face-major (face 0..kNFaces-1, then surface index).
 */
void SLArFLSNewModel::ComputeWallVisibility(const FLSBox* box, const G4String& volumeName,
                                            const G4ThreeVector& emissionPoint)
{
  double origin[3] = { emissionPoint.x(), emissionPoint.y(), emissionPoint.z() };

  // emission voxel key (visibility depends only on the voxel of the emission point)
  const std::array<long,3> key = {
    (long)std::lround(origin[0] / fBinSize),
    (long)std::lround(origin[1] / fBinSize),
    (long)std::lround(origin[2] / fBinSize) };

  auto& volCache = fVisCache[volumeName];
  const auto it = volCache.find(key);
  if (it != volCache.end()) {
    fLastVisibility = it->second;
    return;
  }

  // integrator takes boundaries by non-const reference: work on a local copy
  double boundaries[3][2];
  for (int a = 0; a < 3; a++) {
    boundaries[a][0] = box->boundaries[a][0];
    boundaries[a][1] = box->boundaries[a][1];
  }

  double wallpoint[3] = { 0., 0., FaceConstant(boundaries, kFaceZMax) }; // ctor arg (dormant wallpoint)
  integrator ig(boundaries, origin, wallpoint, fRSEnabled, fRSGridBin);
  if (fRSEnabled) {
    ig.computeRSED(); // Rayleigh scattering energy density grid for this origin
  }
  ig.SetBaseGeometry(box->geo);

  std::vector<double> vis;
  for (int f = 0; f < kNFaces && f < (int)box->geo.surfColl.size(); f++) {
    for (const auto& s : box->geo.surfColl[f].surf) {
      vis.push_back(ig.integrateSurface(f, s));
    }
  }

  volCache[key] = vis;
  fLastVisibility = std::move(vis);
}

/**
 * Propagate the scintillation photons from the emission point to the photon detector
 * modules.
 *
 * The FLS box of the emission volume is selected by `volumeName` (fallback: the box
 * containing the emission point, then the config "default" box); the visibility of every
 * surface of the box is computed at runtime with the LMFast core and cached per emission
 * voxel.
 *
 * @note TODO(NewModel output): sample the detected photons per photon detector channel
 *       with `G4Poisson(numPhotons * visibility * pde)`, compute the time of flight with
 *       `fTimeSim`, and register `SLArEventPhotonHit` hits on the anode / super-cell-array
 *       event objects (as in `SLArFLSPhotonLibrary::PropagatePhotons`). For now no hits
 *       are registered.
 */
void SLArFLSNewModel::PropagatePhotons(
    const G4ParticleDefinition* particleDef,
    const G4String& volumeName,
    const G4ThreeVector& emissionPoint,
    const int numPhotons,
    const std::vector<double>& emissionTime,
    const std::vector<double>& emissionEnergy)
{
  const double p[3] = { emissionPoint.x(), emissionPoint.y(), emissionPoint.z() };

  // select the FLS box: by volume name, else the one containing the point, else the
  // config-mode default box
  const FLSBox* box = fGeometry.GetBox(volumeName.c_str());
  if (!box) box = fGeometry.FindBoxContaining(p);
  if (!box) box = fGeometry.GetBox("default");
  if (!box) {
    static G4bool warnedNoBox = false;
    if (!warnedNoBox) {
      G4cout << "SLArFLSNewModel::PropagatePhotons: WARNING - no FLS box for volume '"
             << volumeName << "' and no box contains the emission point; skipping." << G4endl;
      warnedNoBox = true;
    }
    return;
  }

  // containment check (the emission point is in LAr, hence inside its TPC box by
  // construction; no clamping anymore)
  bool inside = true;
  for (int a = 0; a < 3; a++) {
    if (p[a] < box->boundaries[a][0] - 1e-6 || p[a] > box->boundaries[a][1] + 1e-6) {
      inside = false;
      break;
    }
  }
  if (!inside) {
    static G4bool warnedOutside = false;
    if (!warnedOutside) {
      G4cout << "SLArFLSNewModel::PropagatePhotons: WARNING - emission point ("
             << p[0] << ", " << p[1] << ", " << p[2] << ") mm is outside the FLS box of volume '"
             << volumeName << "'; skipping." << G4endl;
      warnedOutside = true;
    }
    return;
  }

  // machinery: compute (and cache) the surface visibilities for this emission point
  ComputeWallVisibility(box, volumeName, emissionPoint);

  static G4bool warned = false;
  if (!warned) {
    G4cout << "SLArFLSNewModel::PropagatePhotons: WARNING - LMFast visibility machinery is active, "
           << "but photon hits are not registered yet (output mapping TODO)." << G4endl;
    warned = true;
  }

  // TODO(NewModel output): G4Poisson sampling + SLArEventPhotonHit registration, as in
  // SLArFLSPhotonLibrary::PropagatePhotons.

  return;
}

void SLArFLSNewModel::Print() const
{
  printf("%s SLArFLSNewModel configuration:\n", fName.c_str());
  printf("\tgeometry source: %s, bin size [mm]: %g, RSED grid bin [mm]: %g, RS enabled: %s\n",
      fGeometrySource.c_str(), fBinSize, fRSGridBin, fRSEnabled ? "true" : "false");
  size_t nVoxels = 0;
  for (const auto& kv : fVisCache) nVoxels += kv.second.size();
  printf("\tcached emission voxels: %zu\n", nVoxels);
  fGeometry.Print();
}
