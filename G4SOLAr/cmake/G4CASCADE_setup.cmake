#####################################################################
# @file G4CASCADE_setup.cmake
# @author Daniele Guffanti (University and INFN Milano-Bicocca)
# @date 2024-11-05 10:29
# @brief CMake script to download and setup the Geant4 CASCADE project
#####################################################################

include(FetchContent)

FetchContent_Declare(
  G4CASCADE
  GIT_REPOSITORY "https://github.com/SoLAr-Neutrinos/CASCADE.git"
  GIT_TAG "main"
)

FetchContent_MakeAvailable( G4CASCADE )

set(G4CASCADE_SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/g4cascade-src")
set(G4CASCADE_INCLUDE_DEST "${G4SOLAR_INCLUDE_DIR}/physics/cascade")
set(G4CASCADE_SRC_DEST "${G4SOLAR_SRC_DIR}/physics/cascade")
set(G4CASCADE_DATA_DIR "${PROJECT_SOURCE_DIR}/../g4_extra_data")

message(STATUS "G4CASCADE_SOURCE_DIR: ${G4CASCADE_SOURCE_DIR}")
message(STATUS "G4CASCADE_INCLUDE_DEST: ${G4CASCADE_INCLUDE_DEST}")
message(STATUS "G4CASCADE_SRC_DEST: ${G4CASCADE_SRC_DEST}")
message(STATUS "G4CASCADE_DATA_DIR: ${G4CASCADE_DATA_DIR}")

file(MAKE_DIRECTORY ${G4CASCADE_INCLUDE_DEST})
file(MAKE_DIRECTORY ${G4CASCADE_SRC_DEST})
file(MAKE_DIRECTORY ${G4CASCADE_DATA_DIR})

file(GLOB_RECURSE G4CASCADE_INCLUDE_FILES "${G4CASCADE_SOURCE_DIR}/include/*.hh")

foreach(file_path ${G4CASCADE_INCLUDE_FILES})
  file(COPY ${file_path} DESTINATION ${G4CASCADE_INCLUDE_DEST})
endforeach()

set(REIMPLEMENTED_G4_FILES
  "${G4CASCADE_SOURCE_DIR}/src/G4ParticleHPManager.cc"
  "${G4CASCADE_SOURCE_DIR}/src/G4ParticleHPMessenger.cc"
  "${G4CASCADE_SOURCE_DIR}/src/G4ParticleHPCaptureFS.cc"
)
foreach(file_path ${REIMPLEMENTED_G4_FILES})
    if (EXISTS ${file_path})
      message(STATUS "Copying ${file_path} to ${G4SOLAR_SRC_DIR}")
      file(COPY ${file_path} DESTINATION ${G4SOLAR_SRC_DIR})
    else()
      message(WARNING "File ${file_path} does not exist and will not be copied in G4SOLAr_SOURCE_DIR")
    endif()
endforeach()

file(GLOB_RECURSE G4CASCADE_SRC_FILES "${G4CASCADE_SOURCE_DIR}/src/*.cc")
foreach(file_path ${G4CASCADE_SRC_FILES})
  if (NOT file_path IN_LIST REIMPLEMENTED_G4_FILES)  # Copia solo se non è nei file specifici
    message(STATUS "Copying ${file_path} to ${G4CASCADE_SRC_DEST}")
    file(COPY ${file_path} DESTINATION ${G4CASCADE_SRC_DEST})
  endif()
endforeach()

foreach(file_path ${G4CASCADE_SRC_FILES})
  file(COPY ${file_path} DESTINATION ${G4CASCADE_SRC_DEST})
endforeach()

file(COPY "${G4CASCADE_SOURCE_DIR}/CapGamData" DESTINATION ${G4CASCADE_DATA_DIR})

# 4. Pulisci i file non necessari
# Se desideri eliminare file o cartelle non necessarie all'interno della directory di download,
# puoi usare `file(REMOVE_RECURSE ...)`

# Esempio: Rimuove una cartella temporanea non necessaria
#set(TEMP_DIR "${ExternalRepo_SOURCE_DIR}/temp_dir_to_remove")
#if(EXISTS ${TEMP_DIR})
    #file(REMOVE_RECURSE ${TEMP_DIR})
#endif()

## Esempio: Rimuove un file specifico non necessario
#set(UNNEEDED_FILE "${ExternalRepo_SOURCE_DIR}/some_unused_file.txt")
#if(EXISTS ${UNNEEDED_FILE})
    #file(REMOVE ${UNNEEDED_FILE})
#endif()

