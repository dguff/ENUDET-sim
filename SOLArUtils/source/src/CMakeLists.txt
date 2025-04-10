######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists.txt
# @created     : Friday Apr 12, 2024 11:31:49 CEST
######################################################################

#------------------------------------------------------------------------
# Create RecoHits library
add_library(SLArRecoHits
  SHARED
  ${SOLAR_UTILS_SRC_DIR}/SLArRecoHits.cc
  ${SOLAR_UTILS_INC_DIR}/SLArRecoHits.hh)

target_link_libraries(SLArRecoHits PRIVATE ${ROOT_LIBRARIES})

root_generate_dictionary(G__SLArRecoHits
  ${SOLAR_UTILS_INC_DIR}/SLArRecoHits.hh
  MODULE SLArRecoHits
  LINKDEF ${SOLAR_UTILS_INC_DIR}/SLArRecoHitsLinkDef.h)

#-------------------------------------------------------------------------- 
# Create HitConverter library

add_library(SLArHitConverter
  SHARED
  ${SOLAR_UTILS_SRC_DIR}/TChannelAnalyzer.cc
  ${SOLAR_UTILS_INC_DIR}/TChannelAnalyzer.hh)

target_link_libraries(SLArHitConverter PUBLIC 
  SLArRecoHits
  ${ROOT_LIBRARIES}
  Geant4::G4intercoms)

#----------------------------------------------------------------------
# Create Event Display library
add_library(SLArEveDisplay
  SHARED
  ${SOLAR_UTILS_SRC_DIR}/SLArEveDisplay.cc
  ${SOLAR_UTILS_INC_DIR}/SLArEveDisplay.hh
  )

target_link_libraries(SLArEveDisplay PUBLIC 
  SLArRecoHits
  ${ROOT_LIBRARIES} ROOT::Geom ROOT::GenVector ROOT::Gui ROOT::Eve 
  SOLARSIM::SLArReadoutSystemConfig
  ${Geant4_LIBRARIES} 
  SOLARSIM::SLArMCTruth
  SOLARSIM::SLArMCEventReadout)

root_generate_dictionary(G__SLArEveDisplay 
  ${SOLAR_UTILS_INC_DIR}/SLArEveDisplay.hh
  MODULE SLArEveDisplay
  LINKDEF ${SOLAR_UTILS_INC_DIR}/SLArEveDisplayLinkDef.h)

#----------------------------------------------------------------------
# Install instructions

set(SOLAr_utils_libs
  SLArRecoHits
  SLArHitConverter
  SLArEveDisplay
  )

set(utils_resources)

foreach(lib IN LISTS SOLAr_utils_libs)
  # check if dictionary file exists
  if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/G__${lib}.cxx")
    message(STATUS "Dictionary file for ${lib} found")
    # add dictionary files to the list of resources
    list(APPEND utils_resources 
      "${CMAKE_CURRENT_BINARY_DIR}/G__${lib}.cxx"
      "${CMAKE_CURRENT_BINARY_DIR}/lib${lib}.rootmap"
      "${CMAKE_CURRENT_BINARY_DIR}/lib${lib}_rdict.pcm"
  )
  endif()

  # set the library properties
  set_target_properties(${lib} PROPERTIES
    BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_RPATH ${SOLAR_UTILS_LIB_DIR}
  )

  # install the library
  install(TARGETS ${lib}
    LIBRARY DESTINATION        "${SOLAR_UTILS_LIB_DIR}" 
    RUNTIME DESTINATION        "${SOLAR_UTILS_LIB_DIR}")
endforeach()

# install the dictionary files
install(FILES ${utils_resources} DESTINATION "${SOLAR_UTILS_LIB_DIR}")

