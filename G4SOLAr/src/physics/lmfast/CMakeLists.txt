# LMFast core (proof-of-concept fast light visibility computation), vendored from the
# standalone LMFast prototype project. Only the pure library sources are built here
# (integrator / RSED / GLT; std + FFTW3 + Eigen only, no Geant4/ROOT/ENUDET-sim deps);
# the standalone demo main (LMFast.cc, ROOT GUI) is intentionally not compiled into
# SOLAr-sim. NOTE: geometry.cc also lives here but is compiled into solar_sim instead
# (see src/physics/CMakeLists.txt), because its flsGeometry translator part includes
# ENUDET-sim / Geant4 headers.

set(SLAR_LMFAST_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/integrator.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/RSED.cc"
)

add_library(SLArLMFast STATIC ${SLAR_LMFAST_SOURCES})

target_include_directories(SLArLMFast PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${FFTW3_INCLUDE_DIRS}
)

if (TARGET FFTW3::fftw3)
  target_link_libraries(SLArLMFast PUBLIC FFTW3::fftw3 Eigen3::Eigen)
else()
  target_link_libraries(SLArLMFast PUBLIC fftw3 Eigen3::Eigen)
endif()

# Always optimize the fast-sim core. -march=native (let the compiler detect the ISA
# supported by the local machine, replacing the prototype's hardcoded -mavx2) is on by
# default; disable SLAR_LMFAST_NATIVE_ARCH for portable builds.
target_compile_options(SLArLMFast PRIVATE -O3)
if (SLAR_LMFAST_NATIVE_ARCH)
  target_compile_options(SLArLMFast PRIVATE -march=native)
endif()
