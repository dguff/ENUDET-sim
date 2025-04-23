/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArRootUtilities
 * @created     : Wednesday Feb 12, 2025 15:47:13 CET
 */

#ifndef SLARROOTUTILITIES_HH

#define SLARROOTUTILITIES_HH

#include "TFile.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TH2F.h"
#include "TGraph.h"

#include "rapidjson/document.h"


template <typename T>
T* get_from_rootfile(const std::string& filename, const std::string& key)
{
  TFile* rootFile = TFile::Open(filename.data(), "READ");
  if (!rootFile || rootFile->IsZombie()) {
    std::fprintf(stderr, "GetFromRootfile ERROR: Cannot open file %s", filename.data()); 
    exit(EXIT_FAILURE);
  }
  T* obj = dynamic_cast<T*>(rootFile->Get(key.data()));
  obj->SetDirectory(0);
  if (!obj) {
    rootFile->Close();
    std::fprintf(stderr, "GetFromRoofile ERROR: Unable to find object with key %s", key.data());
    exit(EXIT_FAILURE);
  }
  rootFile->Close(); 
  return std::move(obj);
}

template <typename T>
T* get_from_rootfile(const rapidjson::Value& file_and_key)
{
  if (!file_and_key.HasMember("file") || !file_and_key.HasMember("key")) {
    std::fprintf(stderr, "GetFromRootfile Error: mandatory 'file' or 'key' field missing."); 
    exit(EXIT_FAILURE);
  } 
  const std::string& filename = file_and_key["file"].GetString();
  const std::string& key = file_and_key["key"].GetString();
  T* obj = get_from_rootfile<T>(filename, key); 
  return std::move(obj);
}

template<> inline TGraph* get_from_rootfile<TGraph>(const std::string& filename, const std::string& key) {
  TFile* rootFile = TFile::Open(filename.data(), "READ");
  if (!rootFile || rootFile->IsZombie()) {
    std::fprintf(stderr, "GetFromRootfile ERROR: Cannot open file %s", filename.data()); 
    exit(EXIT_FAILURE);
  }
  TGraph* obj = dynamic_cast<TGraph*>(rootFile->Get(key.data()));
  if (!obj) {
    rootFile->Close();
    std::fprintf(stderr, "GetFromRoofile ERROR: Unable to find object with key %s", key.data());
    exit(EXIT_FAILURE);
  }
  rootFile->Close(); 
  return std::move(obj);
}

template TH1D*   get_from_rootfile<TH1D>  (const rapidjson::Value& file_and_key); 
template TH1F*   get_from_rootfile<TH1F>  (const rapidjson::Value& file_and_key); 
template TH2D*   get_from_rootfile<TH2D>  (const rapidjson::Value& file_and_key); 
template TH2F*   get_from_rootfile<TH2F>  (const rapidjson::Value& file_and_key); 
template<> inline TGraph* get_from_rootfile<TGraph>(const rapidjson::Value& file_and_key) {
  if (!file_and_key.HasMember("file") || !file_and_key.HasMember("key")) {
    std::fprintf(stderr, "GetFromRootfile Error: mandatory 'file' or 'key' field missing."); 
    exit(EXIT_FAILURE);
  } 
  const std::string& filename = file_and_key["file"].GetString();
  const std::string& key = file_and_key["key"].GetString();
  TGraph* obj = get_from_rootfile<TGraph>(filename, key); 
  return std::move(obj);
} 





#endif /* end of include guard SLARROOTUTILITIES_HH */

