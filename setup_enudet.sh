#!/bin/bash

# Get the full path of the sourced script
if [[ "${BASH_SOURCE[0]}" != "${0}" ]]; then
    SCRIPT_PATH="${BASH_SOURCE[0]}" 
else
    SCRIPT_PATH="$0"
fi

# Get the directory containing the script
if [[ "$SCRIPT_PATH" == */* ]]; then
    ENUDET_DIR="$(cd "$(dirname "$SCRIPT_PATH")" && pwd)"
else
    ENUDET_DIR="$PWD"
fi

export ENUDET_DIR  # Set the variable

echo "ENUDET_DIR set to: $ENUDET_DIR"
