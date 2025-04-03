#*************************************************
#  Filename:   Makefile 		         *
#  Author:     Jordan McElwee 			 *
#  Created:    2025-03-25 17:05 		 * 
#  Description:					 *
#*************************************************/

# Compiler
CXX = g++

# Directories
INC_DIR = /Users/jmcelwee/Software/enudet/G4SOLAr/include
LIB_DIR = /Users/jmcelwee/Software/enudet/install/lib

# Compiler Flags
CXXFLAGS = -I$(INC_DIR) `root-config --cflags`
LDFLAGS = -L$(LIB_DIR) -lSLArMCEvent `root-config --libs`

# Source and Executable
SRC = econverter.cc
TARGET = econverter

# Rules
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

clean:
	rm -f $(TARGET) *~ core

.PHONY: all clean
