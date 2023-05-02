CHAINFPGALIB_ROOT=../..

HOST_ARCH := x86

# Compilation flags
include $(CHAINFPGALIB_ROOT)/opencl.mk
CXXFLAGS += $(opencl_CXXFLAGS) -Wall -std=c++1y
LDFLAGS += $(opencl_LDFLAGS) 
ifeq ($(DEBUG),1)
CXXFLAGS += -g
else
CXXFLAGS += -O2
endif

# Compiler
CXX := g++

# Target
TARGET := host
TARGET_DIR := .

# Directories
INC_DIRS := $(CHAINFPGALIB_ROOT)/include $(CHAINFPGALIB_ROOT)/include/opencl
LIB_DIRS := $(CHAINFPGALIB_ROOT)/lib

# Files
INCS := $(wildcard *.h)
SRCS := $(wildcard *.cpp *.c) 
LIBS := rt pthread chainfpga

# Make it all!
all : $(TARGET_DIR)/$(TARGET)

# Host executable target.
$(TARGET_DIR)/$(TARGET) : Makefile $(SRCS) $(INCS) $(TARGET_DIR)
	$(ECHO)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC $(foreach D,$(INC_DIRS),-I$D) \
			$(SRCS) \
			$(foreach D,$(LIB_DIRS),-L$D) \
			$(foreach L,$(LIBS),-l$L) $(LDFLAGS) \
			-o $(TARGET_DIR)/$(TARGET)

$(TARGET_DIR) :
	$(ECHO)mkdir $(TARGET_DIR)
	
# Standard make targets
clean :
	$(ECHO)rm -f $(TARGET_DIR)/$(TARGET)

.PHONY : all clean
