set(SLAR_GEN_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/gen")
set(SLAR_GEN_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/gen")

set(SLAR_GEN_HEADERS
  "${SLAR_GEN_INCLUDE_DIR}/SLArRandomExtra.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArVertextGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArBulkVertexGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArBoxSurfaceVertexGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArBaseGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArPGunGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArPBombGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArDecay0GeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArDecay0GeneratorMessenger.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArExternalGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArGENIEGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArMarleyGeneratorMessenger.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArPrimaryGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArPrimaryGeneratorMessenger.hh"
)

set(SLAR_GEN_SRC
  "${SLAR_GEN_SOURCE_DIR}/SLArRandomExtra.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArVertextGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArBulkVertexGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArBoxSurfaceVertexGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArBaseGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArPGunGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArPBombGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArDecay0GeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArDecay0GeneratorMessenger.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArExternalGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArGENIEGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArMarleyGeneratorMessenger.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArPrimaryGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArPrimaryGeneratorMessenger.cc"
)

if (SLAR_CRY_INTERFACE)
  list(APPEND SLAR_GEN_HEADERS
    "${SLAR_GEN_INCLUDE_DIR}/SLArCRYGeneratorAction.hh"
  )
  list(APPEND SLAR_GEN_SRC
    "${SLAR_GEN_SOURCE_DIR}/SLArCRYGeneratorAction.cc"
  )
endif()

if (SLAR_RADSRC_INTERFACE)
  list(APPEND SLAR_GEN_HEADERS
    "${SLAR_GEN_INCLUDE_DIR}/SLArRadSrcGeneratorAction.hh"
  )
  list(APPEND SLAR_GEN_SRC
    "${SLAR_GEN_SOURCE_DIR}/SLArRadSrcGeneratorAction.cc"
  )
endif()
