/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : test_marley_oscillogram.C
 * @created     : Tuesday Sep 24, 2024 17:01:56 CEST
 */

#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <event/SLArMCEvent.hh>
#include <event/SLArGenRecords.hpp>

int test_marley_oscillogram(const char* file_path)
{
  TFile* file = new TFile(file_path); 
  TTree* gtree = file->Get<TTree>("GenTree"); 

  SLArGenRecordsVector* grecords = new SLArGenRecordsVector(); 
  gtree->SetBranchAddress("GenRecords", &grecords); 

  TH1D* hcosTheta = new TH1D("hcosTheta", 
      "Nadir distribution;cos#it{#theta}_{nadir};Counts", 50, -1, 1); 

  TVector3 vertical(0, 1, 0); 

  for (size_t i = 0; i < gtree->GetEntries(); i++) {
    gtree->GetEntry(i); 

    for (const auto& rec : grecords->GetRecordsVector()) {
      if (rec.GetGenCode() == 3) {
        const auto& status = rec.GetGenStatus(); 
        TVector3 dir( &status.at(1) );
        hcosTheta->Fill( dir.Dot( vertical ) ); 
      }
    }

  }
    
  hcosTheta->Draw(); 
  return 0;
}

