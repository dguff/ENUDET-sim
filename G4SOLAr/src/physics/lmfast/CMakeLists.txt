# LMFast core (proof-of-concept fast light visibility computation), vendored from the
# standalone LMFast prototype project. Only the library sources are built here
# (integrator / RSED / GLT); the standalone demo main (LMFast.cc, ROOT GUI) is
# intentionally not compiled into SOLAr-sim.

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

# -march=native -mavx2 from the standalone prototype are opt-in for portability
if (SLAR_LMFAST_NATIVE_ARCH)
  target_compile_options(SLArLMFast PRIVATE -march=native -O3 -mavx2)
endif()
