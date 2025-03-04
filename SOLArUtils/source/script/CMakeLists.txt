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
  G4SOLAr::SLArMCEvent
  G4SOLAr::SLArMCEventReadout
  G4SOLAr::SLArReadoutSystemConfig 
  SLArRecoHits
  )

add_executable(hit_viewer hit_viewer.cc)
target_link_libraries(hit_viewer 
  PUBLIC SLArEveDisplay
  PRIVATE
  ${ROOT_LIBRARIES} ROOT::Geom
  G4SOLAr::SLArMCPrimaryInfo
  G4SOLAr::SLArMCEvent
  G4SOLAr::SLArMCEventReadout
  G4SOLAr::SLArReadoutSystemConfig 
  )

add_executable(merge_gen merge_gen.cc)
target_link_libraries(merge_gen
  PRIVATE
  ${ROOT_LIBRARIES}
)

install(TARGETS hit_converter hit_viewer merge_gen
  LIBRARY DESTINATION "${SOLAR_UTILS_BIN_DIR}"
  RUNTIME DESTINATION "${SOLAR_UTILS_BIN_DIR}"
  )

