set(SLAR_ACTION_INCLUDE_DIR "${SOLARSIM_INCLUDE_DIR}/action")
set(SLAR_ACTION_SRC_DIR "${SOLARSIM_SRC_DIR}/action")

set(SLAR_ACTION_HEADERS
  "${SLAR_ACTION_INCLUDE_DIR}/SLArSteppingVerbose.hh"
  "${SLAR_ACTION_INCLUDE_DIR}/SLArSteppingAction.hh"
  "${SLAR_ACTION_INCLUDE_DIR}/SLArTrackingAction.hh" 
  "${SLAR_ACTION_INCLUDE_DIR}/SLArStackingAction.hh" 
  "${SLAR_ACTION_INCLUDE_DIR}/SLArEventAction.hh" 
  "${SLAR_ACTION_INCLUDE_DIR}/SLArRunAction.hh" 
  "${SLAR_ACTION_INCLUDE_DIR}/SLArActionInitialization.hh" 
)

set(SLAR_ACTION_SRC
  "${SLAR_ACTION_SRC_DIR}/SLArSteppingVerbose.cc"
  "${SLAR_ACTION_SRC_DIR}/SLArSteppingAction.cc"
  "${SLAR_ACTION_SRC_DIR}/SLArTrackingAction.cc"
  "${SLAR_ACTION_SRC_DIR}/SLArStackingAction.cc"
  "${SLAR_ACTION_SRC_DIR}/SLArEventAction.cc"
  "${SLAR_ACTION_SRC_DIR}/SLArRunAction.cc"
  "${SLAR_ACTION_SRC_DIR}/SLArActionInitialization.cc"
)

set(solarsim_headers ${solarsim_headers} ${SLAR_ACTION_HEADERS} PARENT_SCOPE)
set(solarsim_sources ${solarsim_sources} ${SLAR_ACTION_SRC} PARENT_SCOPE)
