/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGenRecord.hpp
 * @created     : Monday Sep 23, 2024 14:17:10 CEST
 */

#ifndef SLARGENRECORDS_HPP

#define SLARGENRECORDS_HPP

#include <cstdio>
#include <vector>
#include <array>
#include <TObject.h>
#include <TString.h>

class SLArGenRecord : public TObject {
  public : 
    inline SLArGenRecord() : fGenCode(99), fGenLabel(), fStatus{} {} 
    inline SLArGenRecord(const UShort_t code, const TString label) : 
      fGenCode(code), fGenLabel(label), fStatus{} {}
    inline ~SLArGenRecord() { Reset(); }

    inline UShort_t& GetGenCode() {return fGenCode;}
    inline TString& GetGenLabel() {return fGenLabel;}
    inline std::vector<Float16_t>& GetGenStatus() {return fStatus;}
    inline const UShort_t& GetGenCode() const {return fGenCode;}
    inline const TString& GetGenLabel() const {return fGenLabel;}
    inline const std::vector<Float16_t>& GetGenStatus() const {return fStatus;}

    inline const Float16_t GetEnergy() const {
      if (fStatus.size() == 0) {
        fprintf(stderr, "SLArGenRecord::GetEnergy() WARNING. Energy not set.\n"); 
        return 0.0;
      }
      return fStatus.at(0);
    }
    inline const std::array<Float16_t, 3> GetDirection() {
      std::array<Float16_t, 3> dir{0, 0, 0}; 
      if (fStatus.size() < 4) {
        fprintf(stderr, "SLArGenRecord::GetDirection() WARNING. Direction not set.\n"); 
      }
      else {
        std::copy( fStatus.begin()+1, fStatus.begin()+4, dir.begin() ); 
      }
      return dir;
    }

    inline void Reset() {
      fGenCode = 99;
      fGenLabel.Clear(); 
      fStatus.clear(); 
    }

  private : 
    UShort_t fGenCode; 
    TString  fGenLabel; 
    std::vector<Float16_t> fStatus; 

  public: 
    ClassDef(SLArGenRecord, 1)
};

class SLArGenRecordsVector : public TObject {
  public: 
    inline SLArGenRecordsVector() : fEvNumber(-1), fStatusVector{} {}
    inline SLArGenRecordsVector(const Int_t iev) : fEvNumber(iev), fStatusVector{} {};
    inline ~SLArGenRecordsVector() { fStatusVector.clear(); }; 

    inline SLArGenRecord& AddRecord(const UShort_t code, const TString label) {
      fStatusVector.push_back( SLArGenRecord(code, label) ); 
      return fStatusVector.back();
    }
    inline void SetEventNumber(const Int_t iev) {fEvNumber = iev;}
    inline Int_t& GetEventNumber() {return fEvNumber;}
    inline const Int_t& GetEventNumber() const {return fEvNumber;}
    inline std::vector<SLArGenRecord>& GetRecordsVector() {return fStatusVector;}
    inline const std::vector<SLArGenRecord>& GetRecordsVector() const {return fStatusVector;}

    inline void Reset() {
      fStatusVector.clear();
    }

  private: 
    Int_t fEvNumber; 
    std::vector<SLArGenRecord> fStatusVector; 

  public: 
    ClassDef(SLArGenRecordsVector, 1)
};


#endif /* end of include guard SLARGENSTATUS_HH */

