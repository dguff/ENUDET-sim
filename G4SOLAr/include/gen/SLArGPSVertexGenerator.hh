/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGPSVertexGenerator.hh
 * @created     : Tuesday Dec 03, 2024 09:13:50 CET
 */

#ifndef SLARGPSVERTEXGENERATOR_HH

#define SLARGPSVERTEXGENERATOR_HH

#include "gen/SLArVertextGenerator.hh"
#include "G4SPSPosDistribution.hh"
#include "G4SPSRandomGenerator.hh"

namespace gen 
{

/**
 * @class SLArGPSVertexGenerator
 * @brief Interface to the G4SPSPosDistribution class
 */
class SLArGPSVertexGenerator : public SLArVertexGenerator
{
 public: 
    SLArGPSVertexGenerator(); 
    virtual ~SLArGPSVertexGenerator();

    void ShootVertex(G4ThreeVector& vertex) override;
    void Config(const rapidjson::Value& config) override;
    const rapidjson::Document ExportConfig() const override;
    G4String GetType() const override {return "gps_pos";}
    void Print() const override;

  protected:
    std::unique_ptr<G4SPSPosDistribution> fPSPosGen = nullptr;
    std::unique_ptr<G4SPSRandomGenerator> fPSRandGen = nullptr;
    G4String fReferenceVolumeName = {};
    G4Transform3D fTransform = {};

    rapidjson::Document ExportConfigPointSource() const;
    rapidjson::Document ExportConfigPlaneSource() const;
    rapidjson::Document ExportConfigBeamSource() const;
    rapidjson::Document ExportConfigVolumeSource() const;
    void PrintConfigPointSource() const;
    void PrintConfigBeamSource() const;
    void PrintConfigPlaneSource() const;
    void PrintConfigVolumeSource() const;
    
    G4double _beam_sigma_r = 0.0;
    G4double _beam_sigma_x = 0.0;
    G4double _beam_sigma_y = 0.0;
};

}

#endif /* end of include guard SLARGPSVERTEXGENERATOR_HH */

