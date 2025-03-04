/*************************************************
 * Filename:   merge_gen.cc 		         *
 * Author:     Jordan McElwee 			 *
 * Created:    2025-02-25 11:32 		 * 
 * Description:					 *
 *************************************************/

// std
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <dirent.h>
#include <getopt.h>
#include <stdexcept>

// ROOT
#include "TROOT.h"
#include "TFile.h"
#include "TChain.h"
#include "TKey.h"
#include "TObjString.h"

// Output messages
const char *ERR_MSG = "\033[31;1m[ERROR]\033[0m ";
const char *INF_MSG = "\033[34;1m[INFO]\033[0m ";
const char *WAR_MSG = "\033[33;1m[WARNING]\033[0m ";


// Forw. Dec.
void check_file(TFile *filename);
void check_file(std::string filename);



//*******************************************************************************
//***** MAIN ********************************************************************

int main(int argc, char *argv[]) {

  // --- Default arguments ---
  std::string mergeFileName = "test.root";
  std::string dirName = "./";
  int buffSize = 256000; 

  // --- Flag names ---
  static struct option long_opts[] = {
    {"directory",   required_argument, nullptr, 'd'}, // --directory
    {"output",      required_argument, nullptr, 'o'}, // --output
    {"buffer_size", required_argument, nullptr, 'b'}, // --buffer_size
    {nullptr, 0, nullptr, 0}
  };

  // --- Flag handling --- 
  int opt;
  while ( (opt = getopt_long(argc, argv, "d:o:b:", long_opts, nullptr)) != -1 ) {
    switch (opt)
      {
      case 'd':
	dirName = optarg;
	break;
      case 'o':
	mergeFileName = optarg;
	break;
      case 'b':
	buffSize = std::stoi(optarg);
	break;
      case '?':
	std::cerr << WAR_MSG << "Unknown flag. Continuing..." << std::endl;
	break;
      }
  }
 
  // ROOT Output Level
  gROOT->ProcessLine( "gErrorIgnoreLevel = 3001;");

  // --- Open directory ---
  DIR *dir = opendir(dirName.c_str());
  if (!dir) {
    std::cerr << ERR_MSG << "Could not open directory: \n\t"
	      << dirName << std::endl;
    return EXIT_FAILURE;
  }

  // --- Load ROOT Names ---
  struct dirent *entry;
  std::vector<std::string> inFiles;

  // Read directory
  while ( (entry = readdir(dir)) != nullptr) {
    // Grab file name
    const char* file = entry->d_name;

    // Check for ROOT files
    if ( strstr(file, ".root") != nullptr )
      inFiles.push_back(file);
  }
  closedir(dir);
 

  // --- Add chains ---
  /*
    Loop over the files. Only add if file is
    filled and with keys
   */
  TChain *eventChain = new TChain("EventTree");
  for (const auto &file : inFiles) {
    try {
      std::string filePath = dirName + "/" + file;

      // Check file is normal
      check_file(filePath);
      std::cout << INF_MSG << "Adding tree from: " << file << std::endl;
      eventChain->Add( filePath.c_str() );
    }
    catch (const std::runtime_error& err) {
      std::cerr << WAR_MSG << "Skipping file: " << err.what() << std::endl;
    }
  }
  
  // --- Create Outfile ---
  TFile *mergeFile = new TFile(mergeFileName.c_str(), "RECREATE");
  if ( !mergeFile || mergeFile->IsZombie() ) {
    std::cerr << ERR_MSG << "Could not create output file: \n\t"
	      << mergeFileName << std::endl;
    return EXIT_FAILURE;
  }
  mergeFile->cd();


  // --- Fill Outfile ---
  // Merge TChain into new tree
  std::cout << INF_MSG << "Merging Event Tree..." << std::endl;
  eventChain->Merge(mergeFile, buffSize, "fastkeep");

  /*
    Fill keys with the G4 info, but make sure it's included
    only once. 
  */ 
  std::cout << INF_MSG << "Loading G4 generation information." << std::endl;
  for (const auto file : inFiles) 
    try {
      // Check the file is uncorrupted 
      std::string filePath = dirName + "/" + file;
      check_file(filePath);
      
      // Open file for keys 
      TFile *keyFile = new TFile(filePath.c_str(), "READ");
      TIter keyIter(keyFile->GetListOfKeys());
      TKey* key;

      std::cout << " - Copying keys from file: "
		<< keyFile->GetName() << std::endl;

      // Loop over keys and copy only TObjects 
      while ((key = (TKey*)keyIter())) {
	if ( strcmp(key->GetClassName(), "TObjString") == 0 ||
	     strcmp(key->GetClassName(), "TParameter<long>") == 0 ||
	     strcmp(key->GetClassName(), "SLArCfg") ) {
	  mergeFile->cd();
	  TObject *obj = (TObject*)key->ReadObj();
	  TObject *objCop = obj->Clone();
	  objCop->Write(key->GetName());
	} // if	
      }
      
      // Good memory practice
      keyFile->Close();
      delete keyFile;
      delete key;
      
      // Filled once, break out of loop
      break;
    } // try
    catch (const std::runtime_error& err) {
      continue;
    } // catch
 

  mergeFile->Close();
 

  return EXIT_SUCCESS;
}

//*******************************************************************************


//*******************************************************************************
//***** FUNCTIONS ***************************************************************

// - - - - - - - - - - - - - - - - - -
/*
  Can take both the filename or the TFile
  and check if it has keys. If not, throw
  an exception. 
 */
void check_file(std::string file)
{
  // Create TFile for other input
  TFile *tInfile = new TFile(file.c_str(), "READ");
  check_file(tInfile);
}

void check_file(TFile *file)
{
  // Check if the file is valid
  if (!file || file->IsZombie()) 
    throw std::runtime_error("Cannot open file or is corrupted:\n\t"
			     + static_cast<std::string>(file->GetName()));
  
  // Check if the file contains any objects
  if (file->GetNkeys() == 0) 
    throw std::runtime_error("File '" + static_cast<std::string>(file->GetName())
			     + "' is empty." );
}
// - - - - - - - - - - - - - - - - - -


//*******************************************************************************
