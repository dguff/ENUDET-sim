#!/bin/bash

#*************************************************************
#***** Set the ENUDET directory ******************************

# Get the full path of the sourced script
if [[ "${BASH_SOURCE[0]}" != "${0}" ]]; then
    SCRIPT_PATH="${BASH_SOURCE[0]}" 
else
    SCRIPT_PATH="$0"
fi

echo $SCRIPT_PATH

# Get the directory containing the script
# This might be overkill...
if [[ "$SCRIPT_PATH" == */* ]]; then
    ENUDET_DIR=`$(cd "$(dirname "$SCRIPT_PATH")" && pwd)`
else
    ENUDET_DIR="$PWD"
fi

export ENUDET_DIR  # Set the variable

echo "ENUDET_DIR set to: $ENUDET_DIR"

export ENUDET_INSTALL=${ENUDET_DIR}/install

#*************************************************************


#*************************************************************
#***** Set Paths *********************************************

OS_TYPE=`uname`

echo "$OS_TYPE"

case $OS_TYPE in
    Darwin) # macOS
	LIB_VAR="DYLD_LIBRARY_PATH"
	;;
    Linux)
	LIB_VAR="LD_LIBRARY_PATH"
	;;
    *)
	echo "Unknown OS: $OS_TYPE"
	return 1
	;;
esac

export ${LIB_VAR}="${ENUDET_INSTALL}/lib:${!LIB_VAR}"

export PATH="${PATH}:${ENUDET_INSTALL}"

#*************************************************************


#*************************************************************
#***** Setup Externals ***************************************

cd ${ENUDET_DIR}/G4SOLAr/extern/install/marley
source setup_marley.sh

cd ${ENUDET_DIR}/G4SOLAr/extern/install/ecosmics
source ../../src/ecosmics/setup_ecosmic.sh

cd ../../../../

#*************************************************************
