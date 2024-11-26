set(SLAR_ACTION_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/action")
set(SLAR_ACTION_SRC_DIR "${PROJECT_SOURCE_DIR}/src/action")

set(SLAR_ACTION_HEADERS
  "${SLAR_ACTION_INCLUDE_DIR}/action/SLArSteppingVerbose.hh"
  "${SLAR_ACTION_INCLUDE_DIR}/action/SLArSteppingAction.hh"
  "${SLAR_ACTION_INCLUDE_DIR}/action/SLArTrackingAction.hh" 
  "${SLAR_ACTION_INCLUDE_DIR}/action/SLArStackingAction.hh" 
  "${SLAR_ACTION_INCLUDE_DIR}/action/SLArEventAction.hh" 
  "${SLAR_ACTION_INCLUDE_DIR}/action/SLArRunAction.hh" 
  "${SLAR_ACTION_INCLUDE_DIR}/action/SLArActionInitialization.hh" 
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


