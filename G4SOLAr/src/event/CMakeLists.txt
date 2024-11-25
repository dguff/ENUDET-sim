######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists
# @created     : venerdì mag 06, 2022 23:01:03 CEST
######################################################################

# SLArMCPrimaryInfo dictionary 
add_library(SLArMCPrimaryInfo
  SHARED
  ${G4SOLAR_SRC_DIR}/event/SLArEventTrajectory.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventTrajectory.hh
  ${G4SOLAR_SRC_DIR}/event/SLArMCPrimaryInfo.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArMCPrimaryInfo.hh
  )
#target_include_directories(SLArMCPrimaryInfo 
  #PUBLIC 
  #"$<BUILD_INTERFACE:${G4SOLAR_INCLUDE_DIR}/event>"
  #"$<INSTALL_INTERFACE:${G4SOLAR_INCLUDE_DIR}/event>")
target_link_libraries(SLArMCPrimaryInfo PUBLIC ${ROOT_LIBRARIES})
root_generate_dictionary(G__SLArMCPrimaryInfo
  ${G4SOLAR_INCLUDE_DIR}/event/SLArMCPrimaryInfo.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventTrajectory.hh
  MODULE SLArMCPrimaryInfo
  LINKDEF
  ${G4SOLAR_INCLUDE_DIR}/event/SLArMCPrimaryInfoLinkDef.h)


#ROOT_GENERATE_DICTIONARY(SLArPhotonHitsDict
  #LINKDEF 
  #${G4SOLAR_INCLUDE_DIR}/event/SLArEventPhotonHitLinkDef.h)
#add_library(SLArEventPhotonHit 
  #SHARED ${G4SOLAR_SRC_DIR}/event/SLArEventPhotonHit.cc)
#target_link_libraries(SLArEventPhotonHit PUBLIC ROOT::Core)
add_library(SLArMCEventReadout
  SHARED
  ${G4SOLAR_SRC_DIR}/event/SLArEventGenericHit.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventGenericHit.hh
  ${G4SOLAR_SRC_DIR}/event/SLArEventChargeHit.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventChargeHit.hh
  ${G4SOLAR_SRC_DIR}/event/SLArEventPhotonHit.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventPhotonHit.hh
  ${G4SOLAR_SRC_DIR}/event/SLArEventBacktrackerRecord.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventBacktrackerRecord.hh
  ${G4SOLAR_SRC_DIR}/event/SLArEventHitsCollection.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventHitsCollection.hh
  ${G4SOLAR_SRC_DIR}/event/SLArEventChargePixel.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventChargePixel.hh
  ${G4SOLAR_SRC_DIR}/event/SLArEventTile.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventTile.hh
  ${G4SOLAR_SRC_DIR}/event/SLArEventMegatile.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventMegatile.hh
  ${G4SOLAR_SRC_DIR}/event/SLArEventAnode.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventAnode.hh
  ${G4SOLAR_SRC_DIR}/event/SLArEventSuperCell.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventSuperCell.hh
  ${G4SOLAR_SRC_DIR}/event/SLArEventSuperCellArray.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventSuperCellArray.hh
  )
#target_include_directories(SLArMCEventReadout 
  #PUBLIC 
  #"$<BUILD_INTERFACE:${G4SOLAR_INCLUDE_DIR}/event>"
  #"$<INSTALL_INTERFACE:${G4SOLAR_INCLUDE_DIR}/event>")
target_link_libraries(
  SLArMCEventReadout 
  PUBLIC 
  SLArReadoutSystemConfig ${ROOT_LIBRARIES})
ROOT_GENERATE_DICTIONARY(G__SLArMCEventReadout
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventGenericHit.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventPhotonHit.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventChargeHit.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventBacktrackerRecord.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventHitsCollection.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventChargePixel.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventTile.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventMegatile.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventAnode.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventSuperCell.hh
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventSuperCellArray.hh
  MODULE SLArMCEventReadout
  LINKDEF
  ${G4SOLAR_INCLUDE_DIR}/event/SLArEventReadoutLinkDef.h
  )

# Master MC EVENT
add_library(SLArMCEvent 
  SHARED
  ${G4SOLAR_SRC_DIR}/event/SLArMCEvent.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArMCEvent.hh
  )
#target_include_directories(SLArMCEvent 
  #PUBLIC 
  #"$<BUILD_INTERFACE:${G4SOLAR_INCLUDE_DIR}/event>"
  #"$<INSTALL_INTERFACE:${G4SOLAR_INCLUDE_DIR}/event>")
target_link_libraries(SLArMCEvent 
                      PUBLIC
                      ${ROOT_LIBRARIES} 
                      SLArMCPrimaryInfo
                      SLArMCEventReadout)
ROOT_GENERATE_DICTIONARY(G__SLArMCEvent
  ${G4SOLAR_INCLUDE_DIR}/event/SLArMCEvent.hh
  MODULE SLArMCEvent
  LINKDEF
  ${G4SOLAR_INCLUDE_DIR}/event/SLArMCEventLinkDef.h
  )

add_library(SLArGenRecords
  SHARED
  ${G4SOLAR_SRC_DIR}/event/SLArGenRecords.cc
  ${G4SOLAR_INCLUDE_DIR}/event/SLArGenRecords.hh
  ) 
target_link_libraries(SLArGenRecords PUBLIC ${ROOT_LIBRARIES})
root_generate_dictionary(G__SLArGenRecords
  ${G4SOLAR_INCLUDE_DIR}/event/SLArGenRecords.hh
  MODULE SLArGenRecords
  LINKDEF
  ${G4SOLAR_INCLUDE_DIR}/event/SLArGenRecordsLinkDef.h
  )


#----------------------------------------------------------------------------
# Install instructions

set(G4SOLAr_event_libs
  SLArMCPrimaryInfo
  SLArMCEventReadout
  SLArMCEvent
  SLArGenRecords
  )

set(G4SOLAr_event_resources 
  ${CMAKE_CURRENT_BINARY_DIR}/G__SLArMCPrimaryInfo.cxx 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArMCPrimaryInfo.rootmap 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArMCPrimaryInfo_rdict.pcm
  #
  ${CMAKE_CURRENT_BINARY_DIR}/G__SLArMCEvent.cxx 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArMCEvent.rootmap 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArMCEvent_rdict.pcm
  #
  ${CMAKE_CURRENT_BINARY_DIR}/G__SLArMCEventReadout.cxx 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArMCEventReadout.rootmap 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArMCEventReadout_rdict.pcm
  #
  ${CMAKE_CURRENT_BINARY_DIR}/G__SLArGenRecords.cxx 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArGenRecords.rootmap 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArGenRecords_rdict.pcm
  )

foreach(lib IN LISTS G4SOLAr_event_libs)
  set_target_properties(${lib} PROPERTIES 
    BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_RPATH ${G4SOLAR_LIB_DIR}
  )
  install(TARGETS ${lib}
    EXPORT                      G4SOLArTargets
    LIBRARY DESTINATION        "${G4SOLAR_LIB_DIR}" 
    RUNTIME DESTINATION        "${G4SOLAR_LIB_DIR}")
endforeach()

install(FILES ${G4SOLAr_event_resources} DESTINATION "${G4SOLAR_LIB_DIR}")
