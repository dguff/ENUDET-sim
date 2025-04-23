/// @file
/// @copyright Copyright (C) 2016-2021 Steven Gardiner
/// @license GNU General Public License, version 3
//
// This file is part of MARLEY (Model of Argon Reaction Low Energy Yields)
//
// MARLEY is free software: you can redistribute it and/or modify it under the
// terms of version 3 of the GNU General Public License as published by the
// Free Software Foundation.
//
// For the full text of the license please see COPYING or
// visit http://opensource.org/licenses/GPL-3.0
//
// Please respect the MCnet academic usage guidelines. See GUIDELINES
// or visit https://www.montecarlonet.org/GUIDELINES for details.

#pragma once

// Standard library includes
#include <string>
#include <map>

// Geant4 includes
#include <G4VUserPrimaryGeneratorAction.hh>
#include <SLArVertextGenerator.hh>
#include <SLArBaseGenerator.hh>

// MARLEY includes
#include <marley/Generator.hh>

// ROOT includes
#include <TH2F.h>
#include <TH1F.h>

class G4Event;

namespace gen {
namespace marley {

class SLArMarleyGeneratorAction : public SLArBaseGenerator
{
  public:
    struct TargetConfig_t {
      std::vector<G4int> nuclides; 
      std::vector<G4double> fraction; 
    }; 

    struct MarleyConfig_t : public GenConfig_t {
      G4String neutrino_label = "ve"; 
      std::vector<G4String> reactions; 
      TargetConfig_t target; 
      ExtSourceInfo_t oscillogram_info;
      G4bool weight_flux = true;
    };

    SLArMarleyGeneratorAction(G4String label = "");
    ~SLArMarleyGeneratorAction() {};

    G4String GetGeneratorType() const override {return "marley";}
    EGenerator GetGeneratorEnum() const override {return kMarley;}

    void Configure() override; 
    void SourceConfiguration(const rapidjson::Value& config) override;
    void SetupMarleyGen(const std::string& config_file_name);
    void SetupMarleyGen(); 
    virtual void GeneratePrimaries(G4Event*) override;
    inline virtual void SetGenRecord( SLArGenRecord& record) const override {
      SLArBaseGenerator::SetGenRecord(record, fConfig);
    } 
    //G4String WriteConfig() const override;

  protected:
    // MARLEY event generator object
    MarleyConfig_t fConfig;
    ::marley::Generator fMarleyGenerator;
    std::map<double, double> fHalfLifeTable;
    std::unique_ptr<TH2F> fOscillogram;
    double SampleDecayTime(const double half_life) const;
};
}
}
