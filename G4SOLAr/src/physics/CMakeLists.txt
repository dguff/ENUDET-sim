set(SLAR_PHYSICS_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/physics)
set(SLAR_PHYSICS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/physics)

set(SLAR_PHYSICS_HEADERS 
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArStepMax.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArExtraPhysics.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArScintillation.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArOpticalPhysics.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArPhysicsList.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArPhysicsListMessenger.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArCrossSectionBiasing.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArElectronDrift.hh"
)

set(SLAR_PHYSICS_SOURCES
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArStepMax.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArExtraPhysics.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArScintillation.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArOpticalPhysics.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArPhysicsList.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArPhysicsListMessenger.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArCrossSectionBiasing.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArElectronDrift.cc"
)

add_subdirectory(ion_and_scint)


