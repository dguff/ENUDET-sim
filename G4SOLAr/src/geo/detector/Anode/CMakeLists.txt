set(SLAR_GEO_ANODE_HEADERS
  "${SLAR_GEO_INCLUDE_DIR}/detector/Anode/SLArDetReadoutTile.hh"
  "${SLAR_GEO_INCLUDE_DIR}/detector/Anode/SLArDetReadoutTileAssembly.hh" 
  "${SLAR_GEO_INCLUDE_DIR}/detector/Anode/SLArDetAnodeAssembly.hh"
)

set(SLAR_GEO_ANODE_SRC
  "${SLAR_GEO_SRC_DIR}/detector/Anode/SLArDetReadoutTile.cc"
  "${SLAR_GEO_SRC_DIR}/detector/Anode/SLArDetReadoutTileAssembly.cc"
  "${SLAR_GEO_SRC_DIR}/detector/Anode/SLArDetAnodeAssembly.cc"
)

set(SLAR_GEO_DETECTOR_HEADERS ${SLAR_GEO_DETECTOR_HEADERS} ${SLAR_GEO_ANODE_HEADERS} PARENT_SCOPE)
set(SLAR_GEO_DETECTOR_SRC ${SLAR_GEO_DETECTOR_SRC} ${SLAR_GEO_ANODE_SRC} PARENT_SCOPE)
