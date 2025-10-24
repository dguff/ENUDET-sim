/*************************************************
 * Filename:   econverter.cc 		         *
 * Author:     Jordan McElwee 			 *
 * Created:    2025-03-24 10:55 		 * 
 * Description:					 *
 *************************************************/

#include <iostream>
#include <getopt.h>
#include <string>

// ROOT Stuff 
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"

#include "TString.h"
#include "TVector3.h"

// ENUDET Stuff
#include "event/SLArMCEvent.hh"

// Fwd Dec.
void status(int, int);

// Output
std::string INFMSG = "\033[34;1m[INFO]\033[0m ";
std::string ERRMSG = "\033[31;1m[ERROR]\033[0m ";
std::string WARMSG = "\033[33;1m[WARNING]\033[0m ";


int main(int argc, char *argv[]) {


  // =========================================================================
  // INPUT INFORMATION
  
  // - - - - - - - - -
  // Defaults
  TString input = argv[1];
  TString flatfileName;

  bool isOverwrite = true;    
  bool isFile = true; 

  
  // - - - - - - - - -
  // Long flag options
  static struct option long_options[] = {
    {"directory", no_argument,       0, 'd'},   // --directory or -d: Input is a directory
    {"output",    required_argument, 0, 'o'},   // --output or -o: Outfile information
    {"replace",   required_argument, 0, 'r'},   // --replace or -r: Overwrite file
    {0, 0, 0, 0} // End of options
  };

  
  // - - - - - - - - -
  // Flag handling 
  int opt=1;
  while ((opt = getopt_long(argc, argv, "dr:o:", long_options, NULL)) != -1) {
    switch (opt) {
    case 'd':
      isFile = false;
      break;
    case 'o':
      flatfileName = optarg;
      break;
    case 'r':
      isOverwrite = std::stoi(optarg);
      break;
    case '?': // Unknown option
      std::cerr << "Unknown option. Use --help for usage.\n";
      return 1;
    }
  }
  
  // =========================================================================


 
  // =========================================================================
  // OUTFILE INFO 
  
  if ( isFile ) {
    if ( flatfileName.IsNull() ) {
      if ( input.EndsWith(".root") ) {
	flatfileName = input;
	flatfileName.ReplaceAll(".root", "_flat.root");
      }
      else {
	std::cerr << ERRMSG << "Not a ROOT file! Exiting." << std::endl;
	return EXIT_FAILURE;
      }
    }
  } 
  else if ( !isFile )
    if ( flatfileName.IsNull() ) {
      flatfileName = input;
      
      while ( flatfileName.EndsWith("/") &&
	      flatfileName.Length() > 1 )
	flatfileName.Chop();

      if (flatfileName == "." || flatfileName == "..")
	flatfileName = "enudet_flat.root";
      else {
	int lastSlash = flatfileName.Last('/');
	TString outname = (lastSlash != kNPOS) ? flatfileName(lastSlash+1, flatfileName.Length()) : flatfileName;
	flatfileName = flatfileName + "/" + outname + "_flat.root";
      }
    }


  // - Overwrite file - - - - - -
  TString sOverwrite = "RECREATE";
  if (!isOverwrite)
    sOverwrite = "CREATE";
    
     

  // =========================================================================

  
  
  // =========================================================================
  
  /*
    This is not the best way to do it,
    but I can't think of a better solution
    right now.
  */
  TFile *infile = nullptr;
  TTree *intree = nullptr;
  TChain *inchain = nullptr;

  std::cout << INFMSG << "Input file is:\n\t" << input << std::endl;
  
  /*
    If a file, use TTree as it's less overhead.
    If a dir, use TChain. 
  */
  if (isFile) {
    infile = TFile::Open(input, "READ");
    if (!infile || infile->IsZombie()) {
      std::cout << ERRMSG << "File " << input << " not loaded correctly." << std::endl;
      return EXIT_FAILURE;
    }    
    intree = (TTree*) infile->Get("EventTree");
  }
  else {
    inchain = new TChain("EventTree");
    inchain->Add(input);
    intree = inchain;
  }

  // =========================================================================


  
  // =========================================================================
  // SETUP OUTFILE

  std::cout << INFMSG << "Flat file is:\n\t" << flatfileName << std::endl;

  if (sOverwrite)
    std::cout << WARMSG << "Potentially overwriting existing flatfile" << std::endl;
  
  TFile *flatfile = new TFile(flatfileName, sOverwrite);
  TTree *flattree = new TTree("enudet_flat", "ENUDET flattened data"); 

  int npart, pdg[100]; 
  double energy[100], energy_dep[100], energy_dep_lar[100], time[100];
  flattree->Branch("npart",  &npart,  "npart/I");
  flattree->Branch("pdg",    &pdg,    "pdg[npart]/I");
  flattree->Branch("time",   &time,   "time[npart]/D");
  flattree->Branch("energy", &energy, "energy[npart]/D");
  flattree->Branch("E_dep",  &energy_dep, "E_dep[npart]/D");
  flattree->Branch("ELAr_dep", &energy_dep_lar, "E_dep[npart]/D");
  
  double momentum[100][3], vertex[100][3];
  flattree->Branch("momentum", &momentum, "momentum[npart][3]/D");
  flattree->Branch("vertex",   &vertex,   "vertex[npart][3]/D");


  

  // =========================================================================
  // EVENT READ
  
  SLArMCEvent *mc_ev = nullptr;
  intree->SetBranchAddress("MCEvent", &mc_ev);

  std::cout << INFMSG << "Processing events..." << std::endl;
  
  for (int evt=0; evt < intree->GetEntries(); evt++) {
    intree->GetEntry(evt);

    if ((evt+1) % (intree->GetEntries()/100) == 0)
      status(evt+1, intree->GetEntries());
      
    const auto &primaries = mc_ev->GetPrimaries();

    npart = 0;
    for (auto &primary : primaries) {

      pdg[npart] = primary.GetCode();
      energy[npart] = primary.GetEnergy();
      energy_dep[npart] = primary.GetTotalEdep();
      energy_dep_lar[npart] = primary.GetTotalLArEdep();

      std::vector<double> mom_temp = primary.GetMomentum();
      momentum[npart][0] = mom_temp.at(0);
      momentum[npart][1] = mom_temp.at(1);
      momentum[npart][2] = mom_temp.at(2);

      std::vector<double> vert_temp = primary.GetVertex();
      vertex[npart][0] = vert_temp.at(0);
      vertex[npart][1] = vert_temp.at(1);
      vertex[npart][2] = vert_temp.at(2);

      npart++;
      
    }
    
    flattree->Fill();
    
  }

  std::cout << INFMSG << "Processing complete." << std::endl;

  flattree->Write();
  flatfile->Close();

  delete flatfile;
  delete infile;
  // =========================================================================

  return EXIT_SUCCESS;

}






//********************************************************************
//***** PROGRESS ***************************************************** 

void status(int evnt, int evntMax)
{
  int barWidth = 50;

  // Calculate the number of '=' for the bar
  int progBar = barWidth * evnt / evntMax;
  int percent = evnt * 100 / evntMax;

  // Sort out cursor position
  std::cout << "\r" << std::flush;

  // Display the progress bar
  std::cout << "\033[32;1m[";
  for (int i = 0; i < barWidth; ++i) {
    if (i < progBar) {
      std::cout << "=";
    } else {
      std::cout << "\033[0m\033[31m-";
    }
  }
  std::cout << "\033[32;1m] " << std::setw(3) << "\033[0m" << percent
            << "%" << std::flush;
  if (evnt == evntMax) std::cout << std::endl;

}

//********************************************************************
