#ifndef SLARCORSIKAGENERATORACTION_HH
#define SLARCORSIKAGENERATORACTION_HH

#include <string>

/* ================ Corsika Gen ================= *
 * Created: 14-06-2024                            *
 * Author:  Jordan McElwee                        *
 * Email: mcelwee@lp2ib.in2p3.fr                  *
 *                                                *
 * Corsika generator action to apply corsika      *
 * events to each event.                          *
 *                                                *
 * Changelog:                                     *
 * ============================================== */

#include "SLArBaseGenerator.hh"

#include "G4Event.hh"
#include "G4ThreeVector.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"


#include "DBReader.hh"
#include "detector.hh"
#include "EParticle.hh"
#include "EShower.hh"


namespace gen{

	class SLArCorsikaGeneratorAction : public SLArBaseGenerator
	{

	public:
    	// Read configuration file
    	struct CorsikaConfig_t {
      		G4String corsika_db_dir {};
            G4double corsika_det_x[2] {-4., 4.};
            G4double corsika_det_y[2] {-4., 4.};
            G4double    corsika_E[2] {50, 10000000};
            G4double corsika_dT = 2.;
    	};

		// ----- Constructors -----
    	SLArCorsikaGeneratorAction(const G4String label = "");
    	~SLArCorsikaGeneratorAction();

    	G4String GetGeneratorType() const override   { return "corsika"; }
    	EGenerator GetGeneratorEnum() const override { return kCorsika; }

        void SourceConfiguration(const rapidjson::Value &config) override;
        void Configure() override;

    	G4String WriteConfig() const override;
    	
    	
    	virtual void GeneratePrimaries(G4Event *ev);
  
  	protected:
  		CorsikaConfig_t fConfig;

	}; // Corsika class

} // Namespace gen


#endif // End of header guard 
