/**
 * @author      : Luca_pi_mistral-large-4
 * @file        : SLArFLSNewModel.cc
 * @created     : Tuesday Nov 24, 2026 15:22:25 CET
 */

#include "physics/SLArFLSNewModel.hh"

#include "integrator.hh" // LMFast core, vendored in src/physics/lmfast
#include "geometry.hh"   // LMFast 2D geometry model + flsGeometry translator

#include "analysis/SLArAnalysisManager.hh"
#include "event/SLArEventPhotonHit.hh"
#include "geo/detector/SLArDetectorConstruction.hh"

#include "G4Exception.hh"
#include "G4ios.hh"
#include "G4Poisson.hh" // G4Poisson, G4UniformRand
#include "G4RunManager.hh"

#include "TString.h" // Form()
#include "TPRegexp.h"
#include "TObjArray.h"
#include "TObjString.h"

#include <chrono>
#include <cmath>

/**
 * Source the configuration of the NewModel module, build the simplified FLS geometry and
 * the surface.id -> channel target maps.
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
 * | `material` | string | — | material name for the base-class `fTimeSim` (TOF); without it the TOF falls back to the vacuum speed of light (placeholder) |
 * | `time` | object | — | `fTimeSim` configuration (see `SLArFastLightSimTime::Initialize`) |
 * | `targets` | array of objects | all tagged modules, `pde_scale_factor` 1 | which photon detector modules produce hits; each entry has a `pds_module` string (`"anode_<tpcID>"` or `"(apex|scarray|xarapuca|arapuca)_<id>"`, `<id>` = `SLArCfgSuperCellArray` id/copyID) and an optional `pde_scale_factor` (default 1) |
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

  // material + time for the base-class time simulation (TOF), as in SLArFLSPhotonLibrary
  if (config.HasMember("material") && config["material"].IsString()) {
    fTimeSim.SetMaterial(config["material"].GetString());
    fTimeSimReady = true;
  }
  else {
    G4Exception("SLArFLSNewModel::Initialize", "ConfigError", JustWarning,
        "No 'material' given: the time of flight falls back to the vacuum speed of light "
        "(placeholder until a material is configured).");
  }
  if (config.HasMember("time")) {
    fTimeSim.Initialize(config["time"]);
  }

  // targets: which tagged modules produce hits, and with which PDE scale factor.
  // Same pds_module naming as the LUT module: "anode_<tpcID>" selects the anode of that
  // TPC, "(apex|scarray|xarapuca|arapuca)_<id>" selects the PDS super-cell array with
  // id = SLArCfgSuperCellArray id (copyID). Without a "targets" list every tagged
  // module is a target with pde_scale_factor 1.
  TPRegexp rgx_anode("anode_(\\d+)");
  TPRegexp rgx_scarray("^(apex|scarray|xarapuca|arapuca)_([0-9]+)$");
  if (config.HasMember("targets") && config["targets"].IsArray()) {
    fTargetsConfigured = true;
    for (const auto& jt : config["targets"].GetArray()) {
      if (!jt.IsObject() || !jt.HasMember("pds_module") || !jt["pds_module"].IsString()) {
        G4Exception("SLArFLSNewModel::Initialize", "ConfigError", JustWarning,
            "Invalid 'targets' entry (expected an object with a string 'pds_module'). Skipped.");
        continue;
      }
      TString pds_module = jt["pds_module"].GetString();
      const double pde = (jt.HasMember("pde_scale_factor") && jt["pde_scale_factor"].IsNumber())
                           ? jt["pde_scale_factor"].GetDouble() : 1.0;
      if (rgx_anode.MatchB(pds_module)) {
        TObjArray* matches = rgx_anode.MatchS(pds_module);
        const int tpc_id = std::stoi(static_cast<TObjString*>(matches->At(1))->GetString().Data());
        fAnodeTargetPDE[tpc_id] = pde;
        delete matches;
      }
      else if (rgx_scarray.MatchB(pds_module)) {
        TObjArray* matches = rgx_scarray.MatchS(pds_module);
        const int arr_id = std::stoi(static_cast<TObjString*>(matches->At(2))->GetString().Data());
        fPDSArrayTargetPDE[arr_id] = pde;
        delete matches;
      }
      else {
        G4Exception("SLArFLSNewModel::Initialize", "ConfigError", JustWarning,
            Form("Unknown pds_module format '%s' in 'targets' (expected 'anode_<tpcID>' or "
                 "'(apex|scarray|xarapuca|arapuca)_<id>'). Skipped.", pds_module.Data()));
      }
    }
  }

  // anode elementIdx -> (megatile index, tile index): rebuild the same enumeration used by
  // flsGeometry when tagging the anode surfaces (megatile vector, then tile vector; the
  // vector position equals the cfg GetIdx(), which is what RegisterHit expects)
  auto* ana_mgr = SLArAnalysisManager::Instance();
  for (const auto& kv : ana_mgr->GetAnodeCfg()) {
    const int tpcId = kv.first;
    const auto& anode = kv.second;
    auto& tileMap = fAnodeTileIdxMap[tpcId];
    tileMap.clear();
    for (size_t mt = 0; mt < anode.GetConstMap().size(); mt++) {
      const auto& tiles = anode.GetConstMap().at(mt).GetConstMap();
      for (size_t t = 0; t < tiles.size(); t++) {
        tileMap.push_back({static_cast<int>(mt), static_cast<int>(t)});
      }
    }
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
const std::vector<double>& SLArFLSNewModel::ComputeWallVisibility(const FLSBox* box,
                                                                  const G4String& volumeName,
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
    return it->second;
  }

  // cache miss: compute the visibilities of all surfaces of this box for this emission voxel
  const auto tStart = std::chrono::steady_clock::now();
  G4cout << "SLArFLSNewModel::ComputeWallVisibility: computing visibilities for volume '"
         << volumeName << "', emission voxel (" << key[0] << ", " << key[1] << ", " << key[2]
         << ") [bin " << fBinSize << " mm], origin (" << origin[0] << ", " << origin[1] << ", "
         << origin[2] << ") mm" << G4endl;

  // integrator takes boundaries by non-const reference: work on a local copy
  double boundaries[3][2];
  for (int a = 0; a < 3; a++) {
    boundaries[a][0] = box->boundaries[a][0];
    boundaries[a][1] = box->boundaries[a][1];
  }

  double wallpoint[3] = { 0., 0., FaceConstant(boundaries, kFaceZMax) }; // ctor arg (dormant wallpoint)
  integrator ig(boundaries, origin, wallpoint, fRSEnabled, fRSGridBin);
  if (fRSEnabled) {
    G4cout << "  [RSED] computing the RSED grid (bin " << fRSGridBin << " mm) for this origin..."
           << G4endl;
    const auto t0 = std::chrono::steady_clock::now();
    ig.computeRSED(); // Rayleigh scattering energy density grid for this origin
    G4cout << "  [RSED] grid done in "
           << std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count()
           << " s" << G4endl;
  }
  ig.SetBaseGeometry(box->geo);

  const int nFaces = std::min((int)kNFaces, (int)box->geo.surfColl.size());
  size_t nSurf = 0;
  for (int f = 0; f < nFaces; f++) nSurf += box->geo.surfColl[f].surf.size();
  G4cout << "  [integrator] integrating " << nSurf << " surfaces on " << nFaces << " faces..."
         << G4endl;

  std::vector<double> vis;
  double vsum = 0.;
  for (int f = 0; f < nFaces; f++) {
    for (const auto& s : box->geo.surfColl[f].surf) {
      const double v = ig.integrateSurface(f, s);
      vis.push_back(v);
      vsum += v;
    }
    G4cout << "    face " << f << ": " << box->geo.surfColl[f].surf.size() << " surfaces done, "
           << vis.size() << "/" << nSurf << " total (t = "
           << std::chrono::duration<double>(std::chrono::steady_clock::now() - tStart).count()
           << " s)" << G4endl;
  }

  G4cout << "SLArFLSNewModel::ComputeWallVisibility: done - " << vis.size() << " visibilities, sum "
         << vsum << ", total "
         << std::chrono::duration<double>(std::chrono::steady_clock::now() - tStart).count()
         << " s" << G4endl;

  return volCache[key] = std::move(vis);
}

double SLArFLSNewModel::GetPDEScaleFactor(const FLSModuleRef& ref) const
{
  if (!fTargetsConfigured) return 1.0; // no "targets" list: every tagged module is a target
  if (ref.kind == kFLSAnode) {
    const auto it = fAnodeTargetPDE.find(ref.tpcId);
    return (it != fAnodeTargetPDE.end()) ? it->second : -1.;
  }
  const auto it = fPDSArrayTargetPDE.find(ref.moduleId);
  return (it != fPDSArrayTargetPDE.end()) ? it->second : -1.;
}

G4double SLArFLSNewModel::SampleTOF(const G4ThreeVector& emissionPoint,
                                    const G4ThreeVector& opDetPoint,
                                    G4double wvlen) const
{
  if (fTimeSimReady) {
    return fTimeSim.SamplePropagationTime(emissionPoint, opDetPoint, wvlen);
  }
  static G4bool warned = false;
  if (!warned) {
    G4cout << "SLArFLSNewModel::SampleTOF: WARNING - no 'material' configured; using the "
           << "vacuum speed of light for the time of flight (placeholder)." << G4endl;
    warned = true;
  }
  return (opDetPoint - emissionPoint).mag() / CLHEP::c_light;
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
 * The per-surface visibilities are then mapped to the photon detector channels through
 * `FLSBox::moduleTags` (one surface patch per base element: super cell / readout tile;
 * filler surfaces have id -1 and get no hits): per element the detected photons are sampled
 * with `G4Poisson(numPhotons * visibility * pde_scale_factor)` (as in
 * `SLArFLSPhotonLibrary::PropagatePhotons`), the time of flight is sampled with the
 * base-class `fTimeSim` from the channel position (center of the surface patch on the box
 * face, detector frame), and the hits are registered as `SLArEventPhotonHit`s on the anode
 * (`GetEventAnodeByTPCID`) / super-cell-array (`GetOpDetArrayByID`) event objects, so they
 * land in the same output trees as the full optical tracking and the LUT module.
 */
void SLArFLSNewModel::PropagatePhotons(
    const G4ParticleDefinition* particleDef,
    const G4String& volumeName,
    const G4ThreeVector& emissionPoint,
    const int numPhotons,
    const std::vector<double>& emissionTime,
    const std::vector<double>& emissionEnergy)
{
  if (emissionTime.empty() || emissionEnergy.empty()) {
    G4Exception("SLArFLSNewModel::PropagatePhotons", "ConfigError", JustWarning,
        "Emission time and energy vectors must not be empty for photon propagation. "
        "Skipping photon propagation.");
    return;
  }

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
  const std::vector<double>& vis = ComputeWallVisibility(box, volumeName, emissionPoint);

  // output: surface.id -> channel -> sampled hits
  auto* ana_mgr = SLArAnalysisManager::Instance();
  int callHits = 0; //!< hits registered by this PropagatePhotons call

  size_t idx = 0; // face-major surface index, aligned with the visibility vector
  for (int f = 0; f < kNFaces && f < (int)box->geo.surfColl.size(); f++) {
    for (const auto& s : box->geo.surfColl[f].surf) {
      const double visibility = vis.at(idx++);

      const auto tit = box->moduleTags.find(s.id);
      if (tit == box->moduleTags.end()) continue; // filler surface (id < 0): no hits
      const FLSModuleRef& ref = tit->second;

      const double pde_scale_factor = GetPDEScaleFactor(ref);
      if (pde_scale_factor <= 0.) continue; // not a configured target
      const int detected_photons = G4Poisson(numPhotons * visibility * pde_scale_factor);
      if (detected_photons <= 0) continue;

      // channel position = center of the surface patch on the box face (detector frame,
      // mm) — the same frame as the emission point
      double c[3];
      ExpandSurfacePoint(box->boundaries, f,
                         0.5 * (s.uMin + s.uMax), 0.5 * (s.vMin + s.vMax), c);
      const G4ThreeVector opDetPoint(c[0], c[1], c[2]);

      for (int ph = 0; ph < detected_photons; ph++) {
        // sample one entry from the emissionTime/wavelength vector
        size_t photon_entry = static_cast<size_t>(G4UniformRand() * emissionTime.size());
        if (photon_entry >= emissionTime.size()) photon_entry = emissionTime.size() - 1;

        const G4double timeOfFlight =
            SampleTOF(emissionPoint, opDetPoint, emissionEnergy.at(photon_entry));

        SLArEventPhotonHit hit((emissionTime.at(photon_entry) + timeOfFlight) / CLHEP::ps, 0);

        if (ref.kind == kFLSAnode) {
          const auto mit = fAnodeTileIdxMap.find(ref.tpcId);
          if (mit == fAnodeTileIdxMap.end() || ref.elementIdx < 0 ||
              static_cast<size_t>(ref.elementIdx) >= mit->second.size()) {
            static G4bool warnedTile = false;
            if (!warnedTile) {
              G4cout << "SLArFLSNewModel::PropagatePhotons: WARNING - anode element "
                     << ref.elementIdx << " of TPC " << ref.tpcId
                     << " has no (megatile, tile) mapping; skipped." << G4endl;
              warnedTile = true;
            }
            continue;
          }
          const auto& mt_t = mit->second.at(ref.elementIdx);
          auto& evAnodeList = ana_mgr->GetEventAnode();
          if (evAnodeList.GetAnodeMap().count(ref.tpcId) == 0) continue;
          evAnodeList.GetEventAnodeByTPCID(ref.tpcId).RegisterHit(hit, mt_t.first, mt_t.second);
        }
        else { // kFLSPDSArray
          auto& evPDS = ana_mgr->GetEventPDS();
          if (evPDS.GetOpDetArrayMap().count(ref.moduleId) == 0) continue;
          evPDS.GetOpDetArrayByID(ref.moduleId).RegisterHit(hit, ref.elementIdx);
        }
        callHits++;
        fNHitsRegistered++;
      }
    }
  }

  G4cout << "SLArFLSNewModel::PropagatePhotons: volume '" << volumeName << "', n_ph " << numPhotons
         << " -> " << callHits << " hits registered this call (total " << fNHitsRegistered << ")"
         << G4endl;

  return;
}

void SLArFLSNewModel::Print() const
{
  printf("%s SLArFLSNewModel configuration:\n", fName.c_str());
  printf("\tgeometry source: %s, bin size [mm]: %g, RSED grid bin [mm]: %g, RS enabled: %s\n",
      fGeometrySource.c_str(), fBinSize, fRSGridBin, fRSEnabled ? "true" : "false");
  printf("\ttime sim material: %s\n",
      fTimeSimReady ? "configured" : "not configured (vacuum TOF placeholder)");
  if (fTargetsConfigured) {
    printf("\ttargets (from config):\n");
    for (const auto& kv : fAnodeTargetPDE)
      printf("\t\tanode of TPC %d: pde_scale_factor %g\n", kv.first, kv.second);
    for (const auto& kv : fPDSArrayTargetPDE)
      printf("\t\tPDS array %d: pde_scale_factor %g\n", kv.first, kv.second);
  }
  else {
    printf("\ttargets: all tagged modules, pde_scale_factor 1\n");
  }
  size_t nVoxels = 0;
  for (const auto& kv : fVisCache) nVoxels += kv.second.size();
  printf("\tcached emission voxels: %zu\n", nVoxels);
  printf("\thits registered: %ld\n", fNHitsRegistered);
  fGeometry.Print();
}
