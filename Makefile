#
# Copyright 2019-2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# makefile-generator v1.0.3
#

############################## Help Section ##############################
.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86> EDGE_COMMON_SW=<rootfs and kernel image path>"
	$(ECHO) "      Command to generate the design for specified Target and Shell."
	$(ECHO) ""
	$(ECHO) "  make clean "
	$(ECHO) "      Command to remove the generated non-hardware files."
	$(ECHO) ""
	$(ECHO) "  make cleanall"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""
	$(ECHO) "  make test PLATFORM=<FPGA platform>"
	$(ECHO) "      Command to run the application. This is same as 'run' target but does not have any makefile dependency."
	$(ECHO) ""
	$(ECHO) "  make sd_card TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86> EDGE_COMMON_SW=<rootfs and kernel image path>"
	$(ECHO) "      Command to prepare sd_card files."
	$(ECHO) ""
	$(ECHO) "  make run TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86> EDGE_COMMON_SW=<rootfs and kernel image path>"
	$(ECHO) "      Command to run application in emulation."
	$(ECHO) ""
	$(ECHO) "  make build TARGET=<sw_emu/hw_emu/hw> PLATFORM=<FPGA platform> HOST_ARCH=<aarch32/aarch64/x86> EDGE_COMMON_SW=<rootfs and kernel image path>"
	$(ECHO) "      Command to build xclbin application."
	$(ECHO) ""
	$(ECHO) "  make host HOST_ARCH=<aarch32/aarch64/x86> EDGE_COMMON_SW=<rootfs and kernel image path>"
	$(ECHO) "      Command to build host application."
	$(ECHO) "  By default, HOST_ARCH=x86. HOST_ARCH and EDGE_COMMON_SW is required for SoC shells"
	$(ECHO) ""

############################## Setting up Project Variables ##############################
# Points to top directory of Git repository
COMMON_REPO ?= ./
PWD = $(shell readlink -f .)
XF_PROJ_ROOT = $(shell readlink -f $(COMMON_REPO))

TARGET := host
HOST_ARCH := x86
SYSROOT := 

XSA := 
ifneq ($(PLATFORM), )
XSA := $(call device2xsa, $(PLATFORM))
endif
TEMP_DIR := ./_x.$(TARGET).$(XSA)
BUILD_DIR := ./build_dir.$(TARGET).$(XSA)

# SoC variables
RUN_APP_SCRIPT = ./run_app.sh
PACKAGE_OUT = ./package.$(TARGET)

LAUNCH_EMULATOR = $(PACKAGE_OUT)/launch_$(TARGET).sh
RESULT_STRING = TEST PASSED

VPP := v++
VPP_PFLAGS := 
CMD_ARGS ?= 
SDCARD := sd_card

AR = ar

include $(XF_PROJ_ROOT)/opencl.mk
# CXXFLAGS += $(opencl_CXXFLAGS) -Wall -O0 -g -std=c++1y
CXXFLAGS += $(opencl_CXXFLAGS) -Wall -O2 -g -std=c++1y
LDFLAGS += $(opencl_LDFLAGS) 


########################## Checking if PLATFORM in whitelist #######################
PLATFORM_BLOCKLIST += nodma 
############################## Setting up Host Variables ##############################
#Include Required Host Source Files
CXXFLAGS += -I$(XF_PROJ_ROOT)/include/opencl -I$(XF_PROJ_ROOT)/include
HOST_SRCS += $(XF_PROJ_ROOT)/src/opencl/xcl2.cpp ./*.cpp ./main.c
# Host compiler global settings
CXXFLAGS += -fmessage-length=0
LDFLAGS += -lrt -lstdc++ -lm -lz -lpthread

ifneq ($(HOST_ARCH), x86)
	LDFLAGS += --sysroot=$(SYSROOT)
endif

############################## Setting up Kernel Variables ##############################
# Kernel compiler global settings
VPP_FLAGS += -t $(TARGET) --platform $(PLATFORM) --save-temps 
ifneq ($(TARGET), hw)
	VPP_FLAGS += -g
endif



# EXECUTABLE = lib/libchainfpga.a
TARGET_DIR = lib
STATICLIB = $(TARGET_DIR)/libchainfpga.a
OBJ = $(TARGET_DIR)/chain_fpga.o $(TARGET_DIR)/xcl2.o 

EMCONFIG_DIR = $(TEMP_DIR)
EMU_DIR = $(SDCARD)/data/emulation

############################## Declaring Binary Containers ##############################
BINARY_CONTAINERS += $(BUILD_DIR)/minimap2_opencl.xclbin
BINARY_CONTAINER_minimap2_opencl_OBJS += $(TEMP_DIR)/chain0.xo
# BINARY_CONTAINER_minimap2_opencl_OBJS += $(TEMP_DIR)/chain1.xo
# BINARY_CONTAINER_minimap2_opencl_OBJS += $(TEMP_DIR)/chain2.xo
# BINARY_CONTAINER_minimap2_opencl_OBJS += $(TEMP_DIR)/chain3.xo

############################## Setting Targets ##############################
CP = cp -rf

.PHONY: all clean cleanall docs emconfig
all: check-platform check-device $(STATICLIB) $(BINARY_CONTAINERS) emconfig sd_card

.PHONY: host
host: $(STATICLIB)

.PHONY: build
build: check-vitis check-device $(BINARY_CONTAINERS)

.PHONY: xclbin
xclbin: build

############################## Setting Rules for Binary Containers (Building Kernels) ##############################
$(TEMP_DIR)/chain0.xo: device/minimap2_opencl.cl
	mkdir -p $(TEMP_DIR)
	$(VPP) $(VPP_FLAGS) -c -k chain0 --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'
# $(TEMP_DIR)/chain1.xo: device/minimap2_opencl.cl
# 	mkdir -p $(TEMP_DIR)
# 	$(VPP) $(VPP_FLAGS) -c -k chain1 --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'
# $(TEMP_DIR)/chain2.xo: device/minimap2_opencl.cl
# 	mkdir -p $(TEMP_DIR)
# 	$(VPP) $(VPP_FLAGS) -c -k chain2 --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'
# $(TEMP_DIR)/chain3.xo: device/minimap2_opencl.cl
# 	mkdir -p $(TEMP_DIR)
# 	$(VPP) $(VPP_FLAGS) -c -k chain3 --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'
$(BUILD_DIR)/minimap2_opencl.xclbin: $(BINARY_CONTAINER_minimap2_opencl_OBJS)
	mkdir -p $(BUILD_DIR)
ifeq ($(HOST_ARCH), x86)
	$(VPP) $(VPP_FLAGS) -l $(VPP_LDFLAGS) --temp_dir $(TEMP_DIR) -o'$(BUILD_DIR)/minimap2_opencl.link.xclbin' $(+)
	$(VPP) -p $(BUILD_DIR)/minimap2_opencl.link.xclbin -t $(TARGET) --platform $(PLATFORM) --package.out_dir $(PACKAGE_OUT) -o $(BUILD_DIR)/minimap2_opencl.xclbin
else
	$(VPP) $(VPP_FLAGS) -l $(VPP_LDFLAGS) --temp_dir $(TEMP_DIR) -o'$(BUILD_DIR)/minimap2_opencl.xclbin' $(+)
endif

############################## Setting Rules for Host (Building Host Executable) ##############################

# Build static library
$(STATICLIB) : Makefile $(OBJ)
		$(AR) rcs $@ $(OBJ)

$(TARGET_DIR)/xcl2.o : $(XF_PROJ_ROOT)/src/opencl/xcl2.cpp $(XF_PROJ_ROOT)/include/opencl/xcl2.hpp
		$(CXX) -c -o $@ $< $(CXXFLAGS) $(LDFLAGS)

$(TARGET_DIR)/chain_fpga.o : src/chain_fpga.cpp $(wildcard include/*.h)
		$(CXX) -c -o $@ $< $(CXXFLAGS) $(LDFLAGS)

emconfig:$(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(PLATFORM) --od $(EMCONFIG_DIR)

############################## Cleaning Rules ##############################
# Cleaning stuff
clean:
	rm -f $(STATICLIB) $(OBJ)

cleanall: clean
	-$(RMDIR) build_dir* sd_card*
	-$(RMDIR) package.*
	-$(RMDIR) _x* *xclbin.run_summary qemu-memory-_* emulation _vimage pl* start_simulation.sh *.xclbin