/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArMCTruth.hh
 * @created     : Wednesday Apr 09, 2025 19:11:04 CEST
 */

#ifndef SLARMCTRUTH_HH

#define SLARMCTRUTH_HH

#include "event/SLArMCPrimaryInfo.hh"

class SLArMCTruth : public TObject {
  public: 
    
    inline SLArMCTruth() : fEvNumber(-1), TObject() {fPrimaries.reserve(100);}
    
    inline SLArMCTruth(const int& evNumber) : fEvNumber(evNumber), TObject() {}
    
    inline SLArMCTruth(const SLArMCTruth& ev) : 
      fEvNumber(ev.fEvNumber), fPrimaries(ev.fPrimaries), TObject(ev) {}
    
    inline ~SLArMCTruth() { Reset(); }

    inline void SetEventNumber(const int& evNumber) {fEvNumber = evNumber;}
    
    inline int GetEventNumber() const {return fEvNumber;}
    
    inline std::vector<SLArMCPrimaryInfo>& GetPrimaries() {return fPrimaries;}
    
    inline void Reset() {fPrimaries.clear(); fEvNumber = -1;}
    
    inline size_t RegisterPrimary(SLArMCPrimaryInfo& p) {
      fPrimaries.push_back( std::move(p) );
      return fPrimaries.size();
    }
    
    inline SLArMCPrimaryInfo& GetPrimary(int ip) {return fPrimaries.at(ip);}

    inline SLArMCPrimaryInfo& GetPrimaryByTrkID(int id) {
      for (auto &p : fPrimaries) {
        if (p.GetTrackID() == id) return p;
      }

      printf("SLArMCTruth::GetPrimaryByTrkID WARNING: Unable to find primary wit track id %i returning the first primary in the list\n", 
          id);
      return fPrimaries.front();
    }

    inline bool  CheckIfPrimary(int trkId) const 
    {
      bool is_primary = false; 
      for (const auto &p : fPrimaries) {
        if (trkId == p.GetTrackID()) {
          is_primary = true; 
          break;
        }
      }
      return is_primary; 
    }

  private: 
    Int_t fEvNumber; 
    std::vector<SLArMCPrimaryInfo> fPrimaries;

  public:
    ClassDef(SLArMCTruth, 1);
};

#endif /* end of include guard SLARMCTRUTH_HH */
