/**
 * @author      : Luca_pi_mistral-large-4
 * @file        : SLArFLSNewModel.cc
 * @created     : Tuesday Nov 24, 2026 15:22:25 CET
 */

#include "physics/SLArFLSNewModel.hh"

#include "integrator.hh" // LMFast core, vendored in src/physics/lmfast

#include "G4Exception.hh"
#include "G4ios.hh"

#include "TString.h" // Form()

#include <cmath>

/**
 * Source the configuration of the NewModel module and set up the LMFast core machinery.
 *
 * Recognized (all optional) fields of the module config:
 *
 * | Field | Type | Default | Description |
 * |-------|------|---------|-------------|
 * | `boundaries` | array of 3 arrays of 2 numbers | LMFast PoC placeholder box | detector box `[[xl,xu],[yl,yu],[zl,zu]]` in mm |
 * | `bin_size` | number | `100` | wall grid bin / emission voxel size in mm |
 * | `rs_enabled` | bool | `true` | enable Rayleigh scattering (RSED) in the LMFast core |
 *
 * @note The default `boundaries` are the placeholder geometry of the LMFast proof of
 *       concept (`LMFast.cc`). TODO(NewModel geometry): derive them from the active
 *       ENUDET-sim geometry.
 *
 * @param config rapidjson configuration object of this module
 */
void SLArFLSNewModel::Initialize(const rapidjson::Value& config)
{
  if (!config.IsObject()) {
    G4Exception("SLArFLSNewModel::Initialize", "ConfigError", JustWarning,
        "NewModel module configuration is not a JSON object. Using the default placeholder geometry.");
  }

  // optional "boundaries": [[xl,xu],[yl,yu],[zl,zu]] in mm
  if (config.HasMember("boundaries")) {
    const auto& jb = config["boundaries"];
    if (jb.IsArray() && jb.Size() == 3) {
      for (int i = 0; i < 3; i++) {
        if (jb[i].IsArray() && jb[i].Size() == 2 &&
            jb[i][0].IsNumber() && jb[i][1].IsNumber()) {
          fBoundaries[i][0] = jb[i][0].GetDouble();
          fBoundaries[i][1] = jb[i][1].GetDouble();
        }
        else {
          G4Exception("SLArFLSNewModel::Initialize", "ConfigError", JustWarning,
              Form("Invalid 'boundaries' entry %d (expected [min, max] numbers). Keeping default.", i));
        }
      }
    }
    else {
      G4Exception("SLArFLSNewModel::Initialize", "ConfigError", JustWarning,
          "Invalid 'boundaries' (expected array of 3 [min, max] pairs). Keeping default placeholder geometry.");
    }
  }

  if (config.HasMember("bin_size") && config["bin_size"].IsNumber()) {
    fBinSize = config["bin_size"].GetDouble();
  }
  if (config.HasMember("rs_enabled") && config["rs_enabled"].IsBool()) {
    fRSEnabled = config["rs_enabled"].GetBool();
  }

  // Build the placeholder wall grid on the +z face of the box (as in the LMFast PoC,
  // which computes the visibility map on the right wall at z = zu).
  // TODO(NewModel geometry): replace with the actual photon detector surfaces.
  fWallPoints.clear();
  const double z_wall = fBoundaries[2][1];
  const long nx = (long)((fBoundaries[0][1] - fBoundaries[0][0]) / fBinSize);
  const long ny = (long)((fBoundaries[1][1] - fBoundaries[1][0]) / fBinSize);
  fWallPoints.reserve((size_t)(nx * ny));
  for (long ix = 0; ix < nx; ix++) {
    for (long iy = 0; iy < ny; iy++) {
      fWallPoints.push_back({
          fBoundaries[0][0] + (ix + 0.5) * fBinSize,
          fBoundaries[1][0] + (iy + 0.5) * fBinSize,
          z_wall });
    }
  }

  G4cout << "SLArFLSNewModel::Initialize: NewModel fast light sim initialized with the LMFast core."
         << G4endl;
  Print();
}

/**
 * Compute (and cache) the visibility of the placeholder wall grid for a given emission
 * point with the LMFast core (`integrator`, with the `RSED` grid if Rayleigh scattering
 * is enabled).
 *
 * The emission point arrives in the detector (LAr target) frame in G4 internal units (mm),
 * which is the same unit used by the LMFast core.
 */
void SLArFLSNewModel::ComputeWallVisibility(const G4ThreeVector& emissionPoint)
{
  double origin[3] = { emissionPoint.x(), emissionPoint.y(), emissionPoint.z() };

  // TODO(NewModel geometry): the placeholder box may not contain the emission point
  // (the active ENUDET-sim geometry can be larger); clamp for now.
  for (int i = 0; i < 3; i++) {
    if (origin[i] < fBoundaries[i][0]) origin[i] = fBoundaries[i][0];
    if (origin[i] > fBoundaries[i][1]) origin[i] = fBoundaries[i][1];
  }

  // emission voxel key (visibility depends only on the voxel of the emission point)
  const std::array<long,3> key = {
    (long)std::lround(origin[0] / fBinSize),
    (long)std::lround(origin[1] / fBinSize),
    (long)std::lround(origin[2] / fBinSize) };

  const auto it = fVisCache.find(key);
  if (it != fVisCache.end()) {
    fLastVisibility = it->second;
    return;
  }

  double wallpoint[3] = { 0., 0., fBoundaries[2][1] };
  integrator ig(fBoundaries, origin, wallpoint, fRSEnabled);
  if (fRSEnabled) {
    ig.computeRSED(); // Rayleigh scattering energy density grid for this origin
  }

  std::vector<double> vis;
  vis.reserve(fWallPoints.size());
  for (const auto& p : fWallPoints) {
    double pt[3] = { p[0], p[1], p[2] };
    vis.push_back(ig.integrateFromVolume(pt));
  }

  fVisCache[key] = vis;
  fLastVisibility = std::move(vis);
}

/**
 * Propagate the scintillation photons from the emission point to the photon detector
 * modules.
 *
 * The machinery is in place: the visibility of the placeholder wall grid for the emission
 * point is computed at runtime with the LMFast core and cached per emission voxel.
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
  // machinery: compute (and cache) the wall visibilities for this emission point
  ComputeWallVisibility(emissionPoint);

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
  printf("\tboundaries [mm]: x [%g, %g], y [%g, %g], z [%g, %g] (placeholder geometry)\n",
      fBoundaries[0][0], fBoundaries[0][1],
      fBoundaries[1][0], fBoundaries[1][1],
      fBoundaries[2][0], fBoundaries[2][1]);
  printf("\tbin size [mm]: %g, RS enabled: %s\n", fBinSize, fRSEnabled ? "true" : "false");
  printf("\twall grid points (+z face): %zu, cached emission voxels: %zu\n",
      fWallPoints.size(), fVisCache.size());
}
