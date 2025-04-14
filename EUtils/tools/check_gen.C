/*************************************************                                                  
 * Filename:   check_gen.C                       *                                                                  
 * Author:     Jordan McElwee                    *                                                  
 * Created:    2025-02-18 13:16                  *                                                  
 * Description:                                  *                                                  
 *************************************************/

// - - - - - - - - - - - - - - -
/*
  Check if individual files are
  empty.
*/
void check_file(const char *filename, bool isGen = false)
{

  // Open file  
  TFile *file = TFile::Open(filename, "READ");
    
  // Check if the file is valid
  if (!file || file->IsZombie()) {
    std::cerr << "ERROR: Cannot open file or file is corrupted!" << std::endl;
    return;
  }
  
  // Check if the file contains any objects
  if (file->GetNkeys() == 0) {
    if (!isGen)
      std::cout << "File \'" << filename << "\' is empty." << std::endl;
    else
      std::cout << " - " << filename << std::endl;
  }
  
  file->Close();
  delete file;
}
// - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - 
void check_gen(const char *dirname)
{
    // Open the directory
    TSystemDirectory dir(dirname, dirname);
    TList *files = dir.GetListOfFiles();
    
    if (!files) {
        std::cerr << "ERROR: Cannot open directory or no files found!" << std::endl;
        return;
    }

    
    std::cout << "The empty files are: " << std::endl;
    
    // Loop over all files in the directory
    TIter next(files);
    TSystemFile *file;
    while ((file = (TSystemFile*)next())) {
        std::string filename = file->GetName();

        // Skip directories and non-ROOT files
        if (file->IsDirectory() || filename.find(".root") == std::string::npos) {
            continue;
        }

        std::string fullpath = std::string(dirname) + "/" + filename;
	
	check_file(fullpath.c_str(), true);
	
    }
}
// - - - - - - - - - - - - - - - 

