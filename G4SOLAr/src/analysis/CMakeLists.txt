set(SLAR_ANALYSIS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/analysis")
set(SLAR_ANALYSIS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/analysis")

set(SLAR_ANALYSIS_HEADERS 
  "${SLAR_ANALYSIS_INCLUDE_DIR}/SLArUserTrackInformation.hh"
  "${SLAR_ANALYSIS_INCLUDE_DIR}/SLArUserPhotonTrackInformation.hh"
  "${SLAR_ANALYSIS_INCLUDE_DIR}/SLArTrajectory.hh"
  "${SLAR_ANALYSIS_INCLUDE_DIR}/SLArRun.hh"
  "${SLAR_ANALYSIS_INCLUDE_DIR}/SLArBacktracker.hh"
  "${SLAR_ANALYSIS_INCLUDE_DIR}/SLArBacktrackerManager.hh"
  "${SLAR_ANALYSIS_INCLUDE_DIR}/SLArAnalysisManager.hh"
  "${SLAR_ANALYSIS_INCLUDE_DIR}/SLArAnalysisManagerMsgr.hh"
)

set(SLAR_ANALYSIS_SOURCES
  "${SLAR_ANALYSIS_SOURCE_DIR}/SLArUserTrackInformation.cc"
  "${SLAR_ANALYSIS_SOURCE_DIR}/SLArUserPhotonTrackInformation.cc"
  "${SLAR_ANALYSIS_SOURCE_DIR}/SLArTrajectory.cc"
  "${SLAR_ANALYSIS_SOURCE_DIR}/SLArRun.cc"
  "${SLAR_ANALYSIS_SOURCE_DIR}/SLArBacktracker.cc"
  "${SLAR_ANALYSIS_SOURCE_DIR}/SLArBacktrackerManager.cc"
  "${SLAR_ANALYSIS_SOURCE_DIR}/SLArAnalysisManager.cc"
  "${SLAR_ANALYSIS_SOURCE_DIR}/SLArAnalysisManagerMsgr.cc"
)

add_subdirectory( sd )

