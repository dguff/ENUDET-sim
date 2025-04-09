######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists.txt
# @created     : Monday Apr 08, 2024 14:11:35 CEST
######################################################################

list(APPEND CMAKE_INSTALL_RPATH "${G4SOLAR_LIB_DIR}" "${SOLAR_UTILS_LIB_DIR}" )
message(STATUS "CMAKE_INSTALL_RPATH: ${CMAKE_INSTALL_RPATH}")

add_executable(hit_converter hit_converter.cc)
target_link_libraries(hit_converter PUBLIC SLArHitConverter)
target_link_libraries(hit_converter PUBLIC
  SOLARSIM::SLArMCEvent
  SOLARSIM::SLArMCEventReadout
  SOLARSIM::SLArReadoutSystemConfig 
  SLArRecoHits
  )

add_executable(hit_viewer hit_viewer.cc)
target_link_libraries(hit_viewer 
  PUBLIC SLArEveDisplay
  PRIVATE
  ${ROOT_LIBRARIES} ROOT::Geom
  SOLARSIM::SLArMCPrimaryInfo
  SOLARSIM::SLArMCEvent
  SOLARSIM::SLArMCEventReadout
  SOLARSIM::SLArReadoutSystemConfig 
  )

add_executable(hit_calibration hit_calibration.cc)
target_link_libraries(hit_calibration
  PRIVATE
  ${ROOT_LIBRARIES} 
  G4SOLAr::SLArGenRecords
)

install(TARGETS hit_converter hit_viewer hit_calibration
  LIBRARY DESTINATION "${SOLAR_UTILS_BIN_DIR}"
  RUNTIME DESTINATION "${SOLAR_UTILS_BIN_DIR}"
  )

