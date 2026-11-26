/**
 * @author      : Luca_pi_mistral-large-4
 * @file        : SLArFLSNewModel.hh
 * @created     : Tuesday Nov 24, 2026 15:22:25 CET
 */

#ifndef SLARFLSNEWMODEL_HH
#define SLARFLSNEWMODEL_HH

#include <array>
#include <map>
#include <string>
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
 * @note TODO(NewModel output): map the computed surface visibilities to the photon detector
 *       channels and register `SLArEventPhotonHit` hits (as in
 *       `SLArFLSPhotonLibrary::PropagatePhotons`).
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
     */
    void ComputeWallVisibility(const FLSBox* box, const G4String& volumeName,
                               const G4ThreeVector& emissionPoint);

    flsGeometry fGeometry; //!< simplified FLS geometry translated from the active detector

    G4String fGeometrySource = "detector"; //!< "detector" (default) or "config" (testing override)
    G4double fBinSize   = 100.;  //!< surface grid bin [mm]
    G4double fRSGridBin = 100.;  //!< RSED grid spacing [mm], uniform in all 3 axes
    G4bool   fRSEnabled = true;  //!< enable Rayleigh scattering (RSED) in the LMFast core

    //! volume -> (emission voxel key -> per-surface visibilities, face-major order)
    std::map<G4String, std::map<std::array<long,3>, std::vector<double>>> fVisCache;
    std::vector<double> fLastVisibility; //!< visibilities of the last computed emission voxel
};

#endif /* end of include guard SLARFLSNEWMODEL_HH */
