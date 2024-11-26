#####################################################################
# @file G4CASCADE_setup.cmake
# @author Daniele Guffanti (University and INFN Milano-Bicocca)
# @date 2024-11-05 10:29
# @brief CMake script to download and setup the Geant4 CASCADE project
#####################################################################

include(FetchContent)

set(G4CASCADE_BRANCH "main")
if (Geant4_VERSION VERSION_GREATER_EQUAL 11.2)
  set(G4CASCADE_BRANCH "geant4_11_2_pXX")
  message(STATUS "CASCADE is not officially supported for Geant4 version >= 10.7. Trying to use experimental branch \"${G4CASCADE_BRANCH}\".")
endif()

FetchContent_Declare(
  G4CASCADE
  GIT_REPOSITORY "https://github.com/SoLAr-Neutrinos/CASCADE.git"
  GIT_TAG ${G4CASCADE_BRANCH}
)

FetchContent_MakeAvailable( G4CASCADE )

set(G4CASCADE_SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/g4cascade-src")
set(G4CASCADE_INCLUDE_DEST "${G4SOLAR_INCLUDE_DIR}/physics/cascade")
set(G4CASCADE_SRC_DEST "${G4SOLAR_SRC_DIR}/physics/cascade")
set(G4CASCADE_DATA_DIR "${PROJECT_SOURCE_DIR}/../g4_extra_data/CapGamData")
get_filename_component(G4CASCADE_DATA_DIR "${G4CASCADE_DATA_DIR}" ABSOLUTE)

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

file(GLOB_RECURSE G4CASCADE_SRC_FILES "${G4CASCADE_SOURCE_DIR}/src/*.cc")
foreach(file_path ${G4CASCADE_SRC_FILES})
    file(COPY ${file_path} DESTINATION ${G4CASCADE_SRC_DEST})
endforeach()

file(COPY "${G4CASCADE_SOURCE_DIR}/CapGamData" DESTINATION ${G4CASCADE_DATA_DIR})

configure_file(${CMAKE_CURRENT_LIST_DIR}/G4CASCADE_CMakeLists.txt.in ${G4CASCADE_SRC_DEST}/CMakeLists.txt @ONLY)

