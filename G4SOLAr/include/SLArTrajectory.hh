/**
 * @author      : guff (guff@guff-gssi)
 * @file        : SLArTrajectory
 * @created     : lunedì ago 31, 2020 12:57:05 CEST
 *
 * Reimplemented from examples/extended/optical/SLAr
 */

#ifndef SLArTRAJECTORY_HH

#define SLArTRAJECTORY_HH

#include "G4Trajectory.hh"
#include "G4Allocator.hh"
#include "G4ios.hh"
#include "globals.hh"
#include "G4ParticleDefinition.hh"
#include "G4TrajectoryPoint.hh"
#include "G4Track.hh"
#include "G4Step.hh"

class G4Polyline;                   // Forward declaration.

class SLArTrajectory : public G4Trajectory
{
  public:

    SLArTrajectory();
    SLArTrajectory(const G4Track* aTrack);
    SLArTrajectory(SLArTrajectory &);
    virtual ~SLArTrajectory();
 
    virtual void DrawTrajectory() const;
 
    inline void* operator new(size_t);
    inline void  operator delete(void*);

    void AddEdep(G4double edep) {fEdepContainer.push_back(edep);}; 

    G4String GetCreatorProcess() {return fCreatorProcess;}
    void SetDrawTrajectory(G4bool b){fDrawit=b;}
    void WLS(){fWls=true;}
    void SetForceDrawTrajectory(G4bool b){fForceDraw=b;}
    void SetForceNoDrawTrajectory(G4bool b){fForceNoDraw=b;}
    std::vector<float>& GetEdep() {return fEdepContainer;}

    G4float GetTime() {return fTime;}

  private:
    G4bool fWls;
    G4bool fDrawit;
    G4bool fForceNoDraw;
    G4bool fForceDraw;
    G4String fCreatorProcess;
    G4float fTime;
    G4ParticleDefinition* fParticleDefinition;
    std::vector<float>  fEdepContainer;
};

extern G4ThreadLocal G4Allocator<SLArTrajectory>* SLArTrajectoryAllocator;

inline void* SLArTrajectory::operator new(size_t)
{
  if(!SLArTrajectoryAllocator)
      SLArTrajectoryAllocator = new G4Allocator<SLArTrajectory>;
  return (void*)SLArTrajectoryAllocator->MallocSingle();
}

inline void SLArTrajectory::operator delete(void* aTrajectory)
{
  SLArTrajectoryAllocator->FreeSingle((SLArTrajectory*)aTrajectory);
}


#endif /* end of include guard SLArTRAJECTORY_HH */

