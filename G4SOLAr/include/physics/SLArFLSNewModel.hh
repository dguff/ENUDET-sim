/**
 * @author      : Luca_pi_mistral-large-4
 * @file        : SLArFLSNewModel.hh
 * @created     : Tuesday Nov 24, 2026 15:22:25 CET
 */

#ifndef SLARFLSNEWMODEL_HH
#define SLARFLSNEWMODEL_HH

#include <array>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "SLArFastLightSim.hh"

#include "geometry.hh" // LMFast 2D geometry model + flsGeometry translator, vendored in src/physics/lmfast

/**
 * @class SLArFLSNewModel
 * @brief "NewModel" fast light simulation module.
 *
 * Alternative fast light simulation module with respect to the LUT-based one
 * (`SLArFLSPhotonLibrary`) and to the (planned) `SemiAnalytical` one: it takes as input
 * the scintillation photons produced along the particle tracks (through
 * `SLArFastLightSimDispatcher::PropagatePhotons`) and produces as output the hits on the
 * photon detector modules.
 *
 * The light visibility is computed at runtime with the LMFast core (`integrator` / `RSED`,
 * vendored in `src/physics/lmfast/`), instead of being read from a LUT.
 *
 * The simplified FLS geometry (per-TPC box + 2D surface patches of the PDS/anode modules
 * projected onto the box faces, empty face area filled with regular bare surfaces) is
 * translated from the active ENUDET-sim detector by `flsGeometry` (see `geometry.hh`).
 *
 * The computed per-surface visibilities are mapped to the photon detector channels through
 * `FLSBox::moduleTags` (surface id -> `FLSModuleRef {kind, tpcId, moduleId, elementIdx}`):
 * the detected photons are sampled per channel element with
 * `G4Poisson(numPhotons * visibility * pde_scale_factor)`, the time of flight is sampled with
 * the base-class `fTimeSim`, and the hits are registered as `SLArEventPhotonHit`s on the
 * `SLArEventAnode` / `SLArEventSuperCellArray` event objects (as in
 * `SLArFLSPhotonLibrary::PropagatePhotons`), so they land in the same output trees as the
 * full optical tracking and the LUT module.
 */
class SLArFLSNewModel : public SLArFastLightSim {
  public:
    inline SLArFLSNewModel() : SLArFastLightSim() {
      fType = kNewModel;
    }
    inline ~SLArFLSNewModel() override = default;

    /**
     * @brief Source the configuration and initialize the NewModel engine
     *
     * @param config rapidjson configuration object of this module
     */
    void Initialize(const rapidjson::Value& config) override;

    /**
     * @brief Propagate optical photons from the emission point to the optical detectors
     *
     * @param particleDef particle definition (for particle-specific optical properties)
     * @param volumeName volume name
     * @param emissionPoint emission point (detector frame, mm)
     * @param numPhotons number of photons to be simulated
     * @param emissionTime vector of emission times of the photons to be propagated
     * @param emissionEnergy vector of energies (wavelengths) of the photons to be propagated
     */
    void PropagatePhotons(
        const G4ParticleDefinition* particleDef,
        const G4String& volumeName,
        const G4ThreeVector& emissionPoint,
        const int numPhotons,
        const std::vector<double>& emissionTime,
        const std::vector<double>& emissionEnergy) override;

    void Print() const override;

  private:
    /**
     * @brief Compute (and cache) the visibility of every surface of the given FLS box for
     *        the given emission point, using the LMFast core (`integrator` / `RSED`).
     *
     * @param box FLS box of the emission volume (detector frame, mm)
     * @param volumeName volume name (cache key)
     * @param emissionPoint emission point (detector frame, mm)
     * @return per-surface visibilities, face-major order (face 0..kNFaces-1, then surface
     *         index inside the face collection) — same order as `box->geo.surfColl`
     */
    const std::vector<double>& ComputeWallVisibility(const FLSBox* box, const G4String& volumeName,
                                                    const G4ThreeVector& emissionPoint);

    /**
     * @brief PDE scale factor of the channel owning the given tagged surface.
     *
     * The default PDE of each module is read at `Initialize` from the module's own optical
     * surface (`EFFICIENCY` property of the `G4OpticalSurface` found walking down from the
     * module PV, evaluated at `fPDEWavelengthNm`), i.e. the same PDE the full optical
     * simulation applies. A `pde_scale_factor` in a `targets` entry overrides it.
     * If a `targets` list was given, only the listed modules produce hits (unlisted ones
     * return a negative factor); otherwise every tagged module is a target.
     *
     * @param ref module tag of the surface (`FLSBox::moduleTags`)
     * @return pde_scale_factor, or a negative value if the module is not a target
     */
    double GetPDEScaleFactor(const FLSModuleRef& ref) const;

    /**
     * @brief Sample the propagation (time-of-flight) of one photon from the emission point
     *        to an optical detector point, with the base-class `fTimeSim` (Landau+exp model)
     *        if a `material` was configured; otherwise fall back to the vacuum speed of
     *        light (placeholder, warn once).
     *
     * @param emissionPoint emission point (detector frame, mm)
     * @param opDetPoint optical detector point (detector frame, mm)
     * @param wvlen photon wavelength (as passed in `emissionEnergy`)
     */
    G4double SampleTOF(const G4ThreeVector& emissionPoint, const G4ThreeVector& opDetPoint,
                       G4double wvlen) const;

    flsGeometry fGeometry; //!< simplified FLS geometry translated from the active detector

    G4String fGeometrySource = "detector"; //!< "detector" (default) or "config" (testing override)
    G4double fBinSize   = 100.;  //!< surface grid bin [mm]
    G4double fRSGridBin = 100.;  //!< RSED grid spacing [mm], uniform in all 3 axes
    G4bool   fRSEnabled = true;  //!< enable Rayleigh scattering (RSED) in the LMFast core

    G4bool fTargetsConfigured = false; //!< true if a "targets" list was given in the config
    G4bool fTimeSimReady      = false; //!< true if a "material" was given (fTimeSim usable)
    G4double fPDEWavelengthNm = 128.;  //!< wavelength at which module PDEs are evaluated [nm]

    //! resolved module PDEs from the optical surfaces: key = (EFLSModuleKind, moduleId)
    std::map<std::pair<int,int>, double> fModulePDE;
    //! ids of the modules listed in "targets" (per kind)
    std::set<int> fAnodeTargetsListed;   //!< tpcIds listed as "anode_<tpcID>"
    std::set<int> fPDSArrayTargetsListed; //!< SCArray ids listed as "..._<id>"
    //! explicit "pde_scale_factor" overrides from "targets": tpcId / SCArray id -> pde
    std::map<int, double> fAnodeTargetPDE;
    std::map<int, double> fPDSArrayTargetPDE;
    //! tpcId -> anode elementIdx -> (megatile index, tile index), in the same enumeration
    //! order used by `flsGeometry` when tagging the anode surfaces (vector positions)
    std::map<int, std::vector<std::pair<int,int>>> fAnodeTileIdxMap;

    //! volume -> (emission voxel key -> per-surface visibilities, face-major order)
    std::map<G4String, std::map<std::array<long,3>, std::vector<double>>> fVisCache;

    long fNHitsRegistered = 0; //!< total photon hits registered on the event objects
};

#endif /* end of include guard SLARFLSNEWMODEL_HH */
