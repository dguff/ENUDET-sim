/**
 * @author      : Antonio Branca (antonio.branca@mib.infn.it)
 * @file        : SLArCRTHit
 * @created     : 2025-03-13 11:40
 */

#ifndef SLArCRTHit_h
#define SLArCRTHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "CLHEP/Units/SystemOfUnits.h"

class G4AttDef;
class G4AttValue;

/// CRT hit
///
/// It records:
/// - the CRT ID
/// - the particle time
/// - the particle local and global positions


class SLArCRTHit : public G4VHit
{
public:
    SLArCRTHit();
    virtual ~SLArCRTHit();

    inline void *operator new(size_t);
    inline void operator delete(void *aHit);
    
    virtual void Draw();
    virtual const std::map<G4String,G4AttDef>* GetAttDefs() const;
    virtual std::vector<G4AttValue>* CreateAttValues() const;
    virtual void Print();

    inline void SetPDG(G4int pdg) { fPDG = pdg; }
    inline G4int GetPDG() const { return fPDG; }
    
    inline void SetCRTNo(G4int idx) { fCRTNo = idx; }
    inline G4int GetCRTNo() const { return fCRTNo; }

    void SetTime(G4double t) { fTime = t; }
    G4double GetTime() const { return fTime / CLHEP::ns; }

    void SetLocalPos(G4ThreeVector xyz) { fLocalPos = xyz; }
    G4ThreeVector GetLocalPos() const { return fLocalPos; }

    void SetWorldPos(G4ThreeVector xyz) { fWorldPos = xyz; }
    G4ThreeVector GetWorldPos() const { return fWorldPos; }

private:
    G4int         fCRTNo;
    G4int         fPDG;
    G4double      fTime;
    G4ThreeVector fLocalPos;
    G4ThreeVector fWorldPos;
};

typedef G4THitsCollection<SLArCRTHit> SLArCRTHitsCollection;

extern G4ThreadLocal G4Allocator<SLArCRTHit>* SLArCRTHitAllocator;

inline void* SLArCRTHit::operator new(size_t)
{
    if (!SLArCRTHitAllocator)
        SLArCRTHitAllocator = new G4Allocator<SLArCRTHit>;
    return (void*)SLArCRTHitAllocator->MallocSingle();
}

inline void SLArCRTHit::operator delete(void* aHit)
{
    SLArCRTHitAllocator->FreeSingle((SLArCRTHit*) aHit);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
