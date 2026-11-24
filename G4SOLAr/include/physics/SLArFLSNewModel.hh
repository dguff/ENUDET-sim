/**
 * @author      : Luca_pi_mistral-large-4
 * @file        : SLArFLSNewModel.hh
 * @created     : Tuesday Nov 24, 2026 15:22:25 CET
 */

#ifndef SLARFLSNEWMODEL_HH
#define SLARFLSNEWMODEL_HH

#include <array>
#include <map>
#include <vector>

#include "SLArFastLightSim.hh"

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
 * @note The geometry currently uses the placeholder box of the LMFast proof of concept
 *       (see `LMFast.cc`: [-11000,11000] x [-2000,2000] x [-2000,2000] mm, wall at z = +2000 mm),
 *       overridable via the `boundaries` field of the module config.
 *       TODO(NewModel geometry): build the integrator geometry from the active ENUDET-sim
 *       geometry (detector construction / geometry JSON).
 *       TODO(NewModel output): map the computed wall visibilities to the photon detector
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
     * @brief Compute (and cache) the visibility of the placeholder wall grid for a given
     *        emission point, using the LMFast core (`integrator` / `RSED`).
     *
     * @param emissionPoint emission point (detector frame, mm)
     */
    void ComputeWallVisibility(const G4ThreeVector& emissionPoint);

    double fBoundaries[3][2] = {{-11000., 11000.}, {-2000., 2000.}, {-2000., 2000.}}; //!< detector box boundaries [mm] (default: LMFast PoC placeholder)
    G4double fBinSize = 100.;   //!< wall grid bin / emission voxel size [mm]
    G4bool   fRSEnabled = true; //!< enable Rayleigh scattering (RSED) in the LMFast core

    std::vector<std::array<double,3>> fWallPoints; //!< placeholder wall (+z face) grid points [mm]
    //! emission voxel key -> wall visibilities
    std::map<std::array<long,3>, std::vector<double>> fVisCache;
    std::vector<double> fLastVisibility; //!< visibilities of the last computed emission voxel
};

#endif /* end of include guard SLARFLSNEWMODEL_HH */
