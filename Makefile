#====================================================================================
# makeESPArduino
#
# A makefile for ESP8286 Arduino projects.
# Edit the contents of this file to suit your project
# or just include it and override the applicable macros.
#
# License: GPL 2.1
# General and full license information is available at:
#    https://github.com/plerup/makeEspArduino
#
# Copyright (c) 2016 Peter Lerup. All rights reserved.
#
#====================================================================================

#====================================================================================
# User editable area
#====================================================================================

#=== Project specific definitions: sketch and list of needed libraries
SKETCH ?= KMEG-WSN-OMI-bridge.ino
LIBS ?= $(ESP_LIBS)/ESP8266WiFi \
        $(ESP_LIBS)/ESP8266HTTPClient \
        $(ESP_LIBS)/ESP8266httpUpdate \
	KMEGLib

# Esp8266 Arduino git location
ESP_ROOT ?= $(HOME)/Arduino/esp8266
# Output directory
BUILD_ROOT ?= /tmp/$(MAIN_NAME)

# Board definitions
FLASH_SIZE ?= 4M
FLASH_MODE ?= dio # qio for v0.9
FLASH_SPEED ?= 40
FLASH_LAYOUT ?= eagle.flash.4m.ld # 1M SPIFFS

# Upload parameters
UPLOAD_SPEED ?= 115200 #230400
UPLOAD_PORT ?= /dev/ttyUSB0
UPLOAD_VERB ?= -v
UPLOAD_RESET ?= nodemcu # none, ck, nodemcu, wifio

################################################################
############# PIN CONNECTION CONFIGURATION #####################
# SERIAL CONFIGURATION:
# Serial  UART0: Usb,           GPIO 1  TX (TX);  GPIO 3  RX (RX)
# -DSWAPSERIAL0 puts Serial to  GPIO 15 D8 (TX);  GPIO 13 D7 (RX)
# Serial1 UART1: RX from flash  GPIO 2  D4 (TX)

# So use Serial1 for debug transmission (can only transmit) and Serial for hacking the bridge.
# Now GPIO 1 is not used but is connected to usb serial for debugging, (not needed for production)
# Connection to wifi-bridge can now send AT commands and receive everything TinyOs is sending
# but cannot communicate to TinyOs MCU, which is not needed. Also everything sent is also echoed back.
# NOTE: Not good, TX is High when not transmitting? => requires modifications to the original bridge
#
# USB Host        ESP8266               WizFi220        TinyOs MCU
# USB RX <---+---< USB     TX >         < TX >----------> RX                
#            |   < GPIO 15 TX >         > RX <-----+----< TX         
#            `---< GPIO 2  TX >                    |  
#                > GPIO 13 RX <--------------------´  
#
# OTHER CONNECTIONS: (restarting in case of crash), D4 is already connected to TX which is connected to VCC via resistor
# D8    |D3    |D4     |
# GPIO15|GPIO0 |GPIO2  |Mode
# ------|------|-------|-----------------
# 0V	|0V    |3.3V   |Uart Bootloader
# 0V	|3.3V  |3.3V   |BOOT SKETCH
# 3.3V	|x     |x      |SDIO mode (not used for Arduino)

EXTRAFLAGS += -Wall -DDBG -DDBGSTREAM=Serial1 -DYGGDRASIL=Serial -DSWAPSERIAL0 # -DDEBUG_ESP_HTTP_CLIENT -DDEBUG_ESP_PORT=Serial1

################################################################

GCC_COLORS=default
#====================================================================================
# The area below should normally not need to be edited
#====================================================================================

MKESPARD_VERSION = 1.0.0

START_TIME := $(shell perl -e "print time();")
# Main output definitions
MAIN_NAME = $(basename $(notdir $(SKETCH)))
MAIN_EXE = $(BUILD_ROOT)/$(MAIN_NAME).bin
MAIN_ELF = $(OBJ_DIR)/$(MAIN_NAME).elf
SRC_GIT_VERSION = $(call git_description,$(dir $(SKETCH)))

# esp8266 arduino directories
ESP_GIT_VERSION = $(call git_description,$(ESP_ROOT))
ESP_LIBS = $(ESP_ROOT)/libraries
TOOLS_ROOT = $(ESP_ROOT)/tools
TOOLS_BIN = $(TOOLS_ROOT)/xtensa-lx106-elf/bin
SDK_ROOT = $(ESP_ROOT)/tools/sdk

# Directory for intermedite build files
OBJ_DIR = $(BUILD_ROOT)/obj
OBJ_EXT = .o
DEP_EXT = .d

# Compiler definitions
CC = $(TOOLS_BIN)/xtensa-lx106-elf-gcc
CPP = $(TOOLS_BIN)/xtensa-lx106-elf-g++
LD =  $(CC)
AR = $(TOOLS_BIN)/xtensa-lx106-elf-ar
ESP_TOOL = $(TOOLS_ROOT)/esptool/esptool

INCLUDE_DIRS += $(SDK_ROOT)/include $(SDK_ROOT)/lwip/include $(CORE_DIR) $(ESP_ROOT)/variants/generic $(OBJ_DIR)
C_DEFINES = -D__ets__ -DICACHE_FLASH -U__STRICT_ANSI__ -DF_CPU=80000000L -DARDUINO=10605 -DARDUINO_ESP8266_ESP01 -DARDUINO_ARCH_ESP8266 -DESP8266
C_INCLUDES = $(foreach dir,$(INCLUDE_DIRS) $(USER_DIRS),-I$(dir))
C_FLAGS ?= -c -Os -g $(EXTRAFLAGS) -Wpointer-arith -Wno-implicit-function-declaration -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -falign-functions=4 -MMD -std=gnu99 -ffunction-sections -fdata-sections
CPP_FLAGS ?= -c -Os -g $(EXTRAFLAGS) -mlongcalls -mtext-section-literals -fno-exceptions -fno-rtti -falign-functions=4 -std=c++11 -MMD -ffunction-sections -fdata-sections
S_FLAGS ?= -c -g -x assembler-with-cpp -MMD
LD_FLAGS ?= -g -w -Os -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static -L$(SDK_ROOT)/lib -L$(SDK_ROOT)/ld -T$(FLASH_LAYOUT) -Wl,--gc-sections -Wl,-wrap,system_restart_local -Wl,-wrap,register_chipv6_phy
LD_STD_LIBS ?= -lm -lgcc -lhal -lphy -lnet80211 -llwip -lwpa -lmain -lpp -lsmartconfig -lwps -lcrypto -laxtls

# Core source files
CORE_DIR = $(ESP_ROOT)/cores/esp8266
CORE_SRC = $(shell find $(CORE_DIR) -name "*.S" -o -name "*.c" -o -name "*.cpp")
CORE_OBJ = $(patsubst %,$(OBJ_DIR)/%$(OBJ_EXT),$(notdir $(CORE_SRC)))
CORE_LIB = $(OBJ_DIR)/core.ar

# User defined compilation units
USER_SRC = $(SKETCH) $(shell find $(LIBS) -name "*.S" -o -name "*.c" -o -name "*.cpp")
# Object file suffix seems to be significant for the linker...
USER_OBJ = $(subst .ino,.cpp,$(patsubst %,$(OBJ_DIR)/%$(OBJ_EXT),$(notdir $(USER_SRC))))
USER_DIRS = $(sort $(dir $(USER_SRC)))

VPATH += $(shell find $(CORE_DIR) -type d) $(USER_DIRS)

# Automatically generated build information data
# Makes the build date and git descriptions at the actual build
# event available as string constants in the program
BUILD_INFO_H = $(OBJ_DIR)/buildinfo.h
BUILD_INFO_CPP = $(OBJ_DIR)/buildinfo.cpp
BUILD_INFO_OBJ = $(BUILD_INFO_CPP)$(OBJ_EXT)

$(BUILD_INFO_H): | $(OBJ_DIR)
	echo "typedef struct { const char *date, *time, *src_version, *env_version;} _tBuildInfo; extern _tBuildInfo _BuildInfo;" >$@

# Utility functions
git_description = $(shell git -C  $(1) describe --tags --always --dirty 2>/dev/null)
time_string = $(shell perl -e 'use POSIX qw(strftime); print strftime($(1), localtime());')
MEM_USAGE = \
  'while (<>) { \
      $$r += $$1 if /^\.(?:data|rodata|bss)\s+(\d+)/;\
		  $$f += $$1 if /^\.(?:irom0\.text|text|data|rodata)\s+(\d+)/;\
	 }\
	 print "\nMemory usage\n";\
	 print sprintf("  %-6s %6d bytes\n" x 2 ."\n", "Ram:", $$r, "Flash:", $$f);'

# Build rules
$(OBJ_DIR)/%.cpp$(OBJ_EXT): %.cpp $(BUILD_INFO_H)
	echo  $(<F)
	$(CPP) $(C_DEFINES) $(C_INCLUDES) $(CPP_FLAGS) $< -o $@

$(OBJ_DIR)/%.cpp$(OBJ_EXT): %.ino $(BUILD_INFO_H)
	echo  $(<F)
	$(CPP) -x c++ -include $(CORE_DIR)/Arduino.h $(C_DEFINES) $(C_INCLUDES) $(CPP_FLAGS) $< -o $@

$(OBJ_DIR)/%.c$(OBJ_EXT): %.c
	echo  $(<F)
	$(CC) $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) $< -o $@

$(OBJ_DIR)/%.S$(OBJ_EXT): %.S
	echo  $(<F)
	$(CC) $(C_DEFINES) $(C_INCLUDES) $(S_FLAGS) $< -o $@

$(CORE_LIB): $(CORE_OBJ)
	echo  Creating core archive
	rm -f $@
	$(AR) cru $@  $^

BUILD_DATE = $(call time_string,"%Y-%m-%d")
BUILD_TIME = $(call time_string,"%H:%M:%S")

$(MAIN_EXE): $(CORE_LIB) $(USER_OBJ)
	echo Linking $(MAIN_EXE)
	echo "  Versions: $(SRC_GIT_VERSION), $(ESP_GIT_VERSION)"
	echo "  WHOLE VERSION STRING: $(SRC_GIT_VERSION)_$(BUILD_TIME)"
	echo 	'#include <buildinfo.h>' >$(BUILD_INFO_CPP)
	echo '_tBuildInfo _BuildInfo = {"$(BUILD_DATE)","$(BUILD_TIME)","$(SRC_GIT_VERSION)","$(ESP_GIT_VERSION)"};' >>$(BUILD_INFO_CPP)
	$(CPP) $(C_DEFINES) $(C_INCLUDES) $(CPP_FLAGS) $(BUILD_INFO_CPP) -o $(BUILD_INFO_OBJ)
	$(LD) $(LD_FLAGS) -Wl,--start-group $^ $(BUILD_INFO_OBJ) $(LD_STD_LIBS) -Wl,--end-group -L$(OBJ_DIR) -o $(MAIN_ELF)
	$(ESP_TOOL) -eo $(ESP_ROOT)/bootloaders/eboot/eboot.elf -bo $@ -bm $(FLASH_MODE) -bf $(FLASH_SPEED) -bz $(FLASH_SIZE) -bs .text -bp 4096 -ec -eo $(MAIN_ELF) -bs .irom0.text -bs .text -bs .data -bs .rodata -bc -ec
	$(TOOLS_BIN)/xtensa-lx106-elf-size -A $(MAIN_ELF) | perl -e $(MEM_USAGE)
	perl -e 'print "Build complete. Elapsed time: ", time()-$(START_TIME),  " seconds\n\n"'

upload: all
	#$(ESP_TOOL) $(UPLOAD_VERB) -cd ck              -cb $(UPLOAD_SPEED) -cp $(UPLOAD_PORT) -ca 0x00000 -cf $(MAIN_EXE)
	$(ESP_TOOL) $(UPLOAD_VERB) -cd $(UPLOAD_RESET) -cb $(UPLOAD_SPEED) -cp $(UPLOAD_PORT) -ca 0x00000 -cf $(MAIN_EXE)

clean:
	echo Removing all intermediate build files...
	rm  -f $(OBJ_DIR)/*

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

.PHONY: all cppcheck
all: $(OBJ_DIR) $(BUILD_INFO_H) $(MAIN_EXE)

cppcheck:
	cppcheck $(C_INCLUDES) .

# Include all available dependencies
-include $(wildcard $(OBJ_DIR)/*$(DEP_EXT))

.DEFAULT_GOAL = all

ifndef VERBOSE
# Set silent mode as default
MAKEFLAGS += --silent
endif
