set(SLAR_MATERIALS_HEADERS
  "${SLAR_GEO_INCLUDE_DIR}/material/SLArMaterial.hh"
)

set(SLAR_MATERIALS_SRC
  "${SLAR_GEO_SRC_DIR}/material/SLArMaterial.cc"
)

set(SLAR_GEO_HEADERS ${SLAR_GEO_HEADERS} ${SLAR_MATERIALS_HEADERS} PARENT_SCOPE)
set(SLAR_GEO_SRC ${SLAR_GEO_SRC} ${SLAR_MATERIALS_SRC} PARENT_SCOPE)

