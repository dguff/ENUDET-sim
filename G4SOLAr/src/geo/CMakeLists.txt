######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists.txt
# @created     : Tuesday Nov 26, 2024 16:14:13 CET
######################################################################

set( SLAR_GEO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/geo" )
set( SLAR_GEO_SRC_DIR "${PROJECT_SOURCE_DIR}/src/geo" )

set ( SLAR_GEO_HEADERS
  "${SLAR_GEO_INCLUDE_DIR}/SLArUnit.hpp"
)

set( SLAR_GEO_SRC 
  ""
)

add_subdirectory( materials )
add_subdirectory( detector )

