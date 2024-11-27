######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists
# @created     : venerdì mag 06, 2022 23:01:03 CEST
######################################################################
set(SLAR_EVT_INCLUDE_DIR "${G4SOLAR_INCLUDE_DIR}/event")
set(SLAR_EVT_SOURCE_DIR "${G4SOLAR_SRC_DIR}/event")

set(SLAR_EVT_PRIMARY_HEADERS
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventTrajectory.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArMCPrimaryInfo.hh"
)

set(SLAR_EVT_PRIMARY_SOURCES
  "${SLAR_EVT_SOURCE_DIR}/SLArEventTrajectory.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArMCPrimaryInfo.cc"
)

# ---------------------------------------------------------
# MC truth data library
# ---------------------------------------------------------
add_library(SLArMCPrimaryInfo SHARED
  ${SLAR_EVT_PRIMARY_SOURCES} ${SLAR_EVT_PRIMARY_HEADERS} )
target_include_directories(SLArMCPrimaryInfo 
  PUBLIC 
  $<BUILD_INTERFACE:${G4SOLAR_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${G4SOLAR_INC_DEST}>)
target_link_libraries(SLArMCPrimaryInfo PUBLIC ${ROOT_LIBRARIES})
root_generate_dictionary(G__SLArMCPrimaryInfo
  ${SLAR_EVT_PRIMARY_HEADERS}
  MODULE SLArMCPrimaryInfo
  LINKDEF ${SLAR_EVT_INCLUDE_DIR}/SLArMCPrimaryInfoLinkDef.h
)

# ---------------------------------------------------------
# Detector hits library
# ---------------------------------------------------------
set(SLAR_EVT_HITS_HEADERS 
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventGenericHit.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventChargeHit.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventPhotonHit.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventBacktrackerRecord.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventHitsCollection.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventChargePixel.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventTile.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventMegatile.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventAnode.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventSuperCell.hh"
  "${SLAR_EVT_INCLUDE_DIR}/SLArEventSuperCellArray.hh"
)
set(SLAR_EVT_HITS_SOURCES
  "${SLAR_EVT_SOURCE_DIR}/SLArEventGenericHit.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArEventChargeHit.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArEventPhotonHit.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArEventBacktrackerRecord.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArEventHitsCollection.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArEventChargePixel.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArEventTile.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArEventMegatile.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArEventAnode.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArEventSuperCell.cc"
  "${SLAR_EVT_SOURCE_DIR}/SLArEventSuperCellArray.cc"
)

add_library(SLArMCEventReadout SHARED
  ${SLAR_EVT_HITS_SOURCES} ${SLAR_EVT_HITS_HEADERS} )
target_include_directories(SLArMCEventReadout PUBLIC 
  $<BUILD_INTERFACE:${G4SOLAR_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${G4SOLAR_INC_DEST}>)
target_link_libraries(SLArMCEventReadout PUBLIC 
  SLArReadoutSystemConfig ${ROOT_LIBRARIES})
ROOT_GENERATE_DICTIONARY(G__SLArMCEventReadout
  ${SLAR_EVT_HITS_HEADERS}
  MODULE SLArMCEventReadout
  LINKDEF ${SLAR_EVT_INCLUDE_DIR}/SLArEventReadoutLinkDef.h
  )

# ---------------------------------------------------------
# Master MCEvent library
# ---------------------------------------------------------
set( SLAR_MCEVENT_HEADERS "${SLAR_EVT_INCLUDE_DIR}/SLArMCEvent.hh" )
set( SLAR_MCEVENT_SOURCES "${SLAR_EVT_SOURCE_DIR}/SLArMCEvent.cc" )
add_library(SLArMCEvent SHARED
  ${SLAR_MCEVENT_SOURCES} ${SLAR_MCEVENT_HEADERS} )
target_include_directories(SLArMCEvent 
  PUBLIC 
  $<BUILD_INTERFACE:${G4SOLAR_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${G4SOLAR_INC_DEST}>)
target_link_libraries(SLArMCEvent PUBLIC
  ${ROOT_LIBRARIES} 
  SLArMCPrimaryInfo
  SLArMCEventReadout)
root_generate_dictionary(G__SLArMCEvent
  ${SLAR_MCEVENT_HEADERS}
  MODULE SLArMCEvent
  LINKDEF ${SLAR_EVT_INCLUDE_DIR}/SLArMCEventLinkDef.h
)


# ---------------------------------------------------------
# Generator records library
# ---------------------------------------------------------
set( SLAR_GENRECORDS_HEADERS "${SLAR_EVT_INCLUDE_DIR}/SLArGenRecords.hh" )
set( SLAR_GENRECORDS_SOURCES "${SLAR_EVT_SOURCE_DIR}/SLArGenRecords.cc" )
add_library(SLArGenRecords SHARED
  ${SLAR_GENRECORDS_SOURCES} ${SLAR_GENRECORDS_HEADERS} )
target_link_libraries(SLArGenRecords PUBLIC ${ROOT_LIBRARIES})
target_include_directories(SLArGenRecords 
  PUBLIC 
  $<BUILD_INTERFACE:${G4SOLAR_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${G4SOLAR_INC_DEST}>)
root_generate_dictionary(G__SLArGenRecords
  ${SLAR_GENRECORDS_HEADERS}
  MODULE SLArGenRecords
  LINKDEF ${SLAR_EVT_INCLUDE_DIR}/SLArGenRecordsLinkDef.h
  )

# ---------------------------------------------------------
# Include headers and sources in project lists
# ---------------------------------------------------------
set(solarsim_headers ${solarsim_headers} 
  ${SLAR_EVT_PRIMARY_HEADERS} 
  ${SLAR_EVT_HITS_HEADERS} 
  ${SLAR_MCEVENT_HEADERS} 
  ${SLAR_GENRECORDS_HEADERS} PARENT_SCOPE)
set(solarsim_sources ${solarsim_sources} 
  ${SLAR_EVT_PRIMARY_SOURCES}
  ${SLAR_EVT_HITS_SOURCES}
  ${SLAR_MCEVENT_SOURCES}
  ${SLAR_GENRECORDS_SOURCES} PARENT_SCOPE)

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
