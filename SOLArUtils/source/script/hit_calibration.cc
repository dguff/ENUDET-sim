/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : hit_calibration
 * @created     : Friday Jan 10, 2025 16:48:26 CET
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <getopt.h>
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TPRegexp.h"

#include "event/SLArGenRecords.hh"

void print_usage() {
  printf("SoLAr hit_calibration: Calibrate hit collection\n"); 
  printf("Usage: "); 
  printf("hit_calibration\n"); 
  printf("\t[--input_list | -l] input file list\n"); 
  printf("\t[--output     | -o] output filename\n\n");
  exit( EXIT_SUCCESS );
}

int main (int argc, char *argv[]) {
  // define analysis constants
  const double drift_velocity = 158.2; // cm/s
  const double argon_w = 23.6e-6; // MeV
  const double recombination_factor = 1.0 / 0.6; 
  


  TString hit_file_list = {}; 
  TString output_filename = {};

  // Parse command line options
  const char* short_opts = "l:o:h";
  // Define long options
  static struct option long_options[] = {
    {"hitfile_list", required_argument, 0, 'l'},
    {"output", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;

  while ( (c = getopt_long(argc, argv, short_opts, long_options, &option_index)) != -1) {
    switch(c) {
      case 'l' :
        hit_file_list = optarg;
        break;
      case 'o' :
        output_filename = optarg;
        break;
      case 'h' : 
        print_usage(); 
        exit( EXIT_SUCCESS ); 
        break;
      case '?' : 
        printf("solar_sim error: unknown flag %c\n", optopt);
        print_usage(); 
        exit( EXIT_FAILURE ); 
        break;
    }
  }

  // setup output file
  TFile* output_file = new TFile(output_filename, "recreate");
  TTree* output_tree = new TTree("CalibratedHitTree", "Calibrated hit collection tree");
  double neutrino_energy = 0.0; 
  double collected_energy = 0.0; 
  double calibrated_energy = 0.0;
  double drift_coordinate = 0.0; 
  output_tree->Branch("enu", &neutrino_energy); 
  output_tree->Branch("ecol", &collected_energy); 
  output_tree->Branch("ecal", &calibrated_energy); 
  output_tree->Branch("drift", &drift_coordinate); 

  // setup the chain
  TChain chain_hit("HitTree");
  TChain chain_gen("GenTree");

  std::string line; 
  std::ifstream file_list; 
  file_list.open(hit_file_list.Data());
  if (file_list.is_open() == false) {
    std::cerr << "Error opening file list: " << hit_file_list.Data() << std::endl;
    return 1;
  }
  
  TPRegexp rgx_seed("[0-9]{10}");

  while (std::getline(file_list, line)) {
    TString event_file_path = line;
    const int seed_pos = event_file_path.Index( rgx_seed );
    TString hit_file_path = event_file_path; 
    hit_file_path.Insert(seed_pos, "hit_");

    // Verify that GenTree and HitTree are present and have the same number of entries
    TFile hit_file(hit_file_path);
    TFile event_file(event_file_path);
    TTree* gen_tree = event_file.Get<TTree>("GenTree");
    TTree* hit_tree = hit_file.Get<TTree>("HitTree");
    if (gen_tree == nullptr) {
      std::cerr << "Error: GenTree not found in file " << event_file_path.Data() << std::endl;
      continue;
    }
    if (hit_tree == nullptr) {
      std::cerr << "Error: HitTree not found in file " << hit_file_path.Data() << std::endl;
      continue;
    }

    if (gen_tree->GetEntries() != hit_tree->GetEntries()) {
      std::cerr << "Error: GenTree and HitTree have different number of entries in file " << event_file_path.Data() << std::endl;
      continue;
    }

    hit_file.Close();
    event_file.Close();

    chain_hit.Add(hit_file_path);
    chain_gen.Add(event_file_path);
  }

  Long64_t nentries = chain_hit.GetEntries();
  SLArGenRecordsVector* gen_records = nullptr;
  chain_gen.SetBranchAddress("GenRecords", &gen_records);
  
  std::vector<float>* hit_q = nullptr;
  std::vector<float>* hit_x = nullptr;
  std::vector<float>* hit_y = nullptr;
  std::vector<float>* hit_z = nullptr;
  chain_hit.SetBranchAddress("hit_q", &hit_q);
  chain_hit.SetBranchAddress("hit_x", &hit_x);
  chain_hit.SetBranchAddress("hit_y", &hit_y);
  chain_hit.SetBranchAddress("hit_z", &hit_z);

  for (Long64_t i = 0; i < nentries; i++) {
    chain_hit.GetEntry(i);
    chain_gen.GetEntry(i);

    const auto& records = gen_records->GetRecordsVector();
    if (records.size() != 1) {
      std::cerr << "Error: GenRecords has more than one entry in event " << i << std::endl;
      exit(1);
    }
    const auto& record = records[0];
    neutrino_energy = record.GetGenStatus().at(0);

    double total_q = 0.0; 
    for (const auto& q : *hit_q) {
      total_q += q;
    }

    // compute the weighted mean drift distance
    double x_mean = 0.0; 
    if (total_q > 0) {
      double w = 0.0; 
      for (size_t j = 0; j < hit_y->size(); j++) {
        x_mean += hit_y->at(j) * hit_q->at(j);
        w += hit_q->at(j);
      }
      x_mean /= w;
      x_mean /= 10.0; // convert to cm


      if (x_mean < 0) {
        drift_coordinate = x_mean + 650;
      }
      else {
        drift_coordinate = 650 - x_mean;
      }
    }
    else {
      drift_coordinate = 1.0; // no effect.
    }

    collected_energy = total_q * argon_w * recombination_factor;
    double tau_elec = 0.04586717986370039*collected_energy + 5.278259363488911;

    calibrated_energy = collected_energy * exp(drift_coordinate / (drift_velocity * tau_elec) );

    output_tree->Fill();
  }

  output_file->cd();
  output_tree->Write();
  output_file->Close();

  return 0;
}



