#**************************************************************************************************
#
#   raylib makefile for Desktop platforms, Raspberry Pi, Android and HTML5
#
#   Copyright (c) 2013-2019 Ramon Santamaria (@raysan5)
#
#   This software is provided "as-is", without any express or implied warranty. In no event
#   will the authors be held liable for any damages arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose, including commercial
#   applications, and to alter it and redistribute it freely, subject to the following restrictions:
#
#     1. The origin of this software must not be misrepresented; you must not claim that you
#     wrote the original software. If you use this software in a product, an acknowledgment
#     in the product documentation would be appreciated but is not required.
#
#     2. Altered source versions must be plainly marked as such, and must not be misrepresented
#     as being the original software.
#
#     3. This notice may not be removed or altered from any source distribution.
#
#**************************************************************************************************

.PHONY: all clean

# Define required raylib variables
PROJECT_NAME       ?= game
RAYLIB_VERSION     ?= 4.0.0
RAYLIB_PATH        ?= C:\raylib\raylib

# Define compiler path on Windows
COMPILER_PATH      ?= C:/raylib/w64devkit/bin

# Define default options
# One of PLATFORM_DESKTOP, PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
PLATFORM           ?= PLATFORM_DESKTOP

# Library type used for raylib: STATIC (.a) or SHARED (.so/.dll)
RAYLIB_LIBTYPE        ?= STATIC

# Build mode for project: DEBUG or RELEASE
BUILD_MODE            ?= DEBUG

PLATFORM_OS=WINDOWS
export PATH := $(COMPILER_PATH):$(PATH)


# Define raylib release directory for compiled library.
# RAYLIB_RELEASE_PATH points to provided binaries or your freshly built version
RAYLIB_RELEASE_PATH 	?= $(RAYLIB_PATH)/src

# Define default C compiler: gcc
# NOTE: define g++ compiler if using C++
CC = g++


# Define default make program: Mingw32-make
MAKE = mingw32-make

# Define compiler flags:
#  -O0                  defines optimization level (no optimization, better for debugging)
#  -O1                  defines optimization level
#  -g                   include debug information on compilation
#  -s                   strip unnecessary data from build -> do not use in debug builds
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -D_DEFAULT_SOURCE    use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
CFLAGS += -Wall -std=c++11 -D_DEFAULT_SOURCE -Wno-missing-braces

ifeq ($(BUILD_MODE),DEBUG)
    CFLAGS += -g -O0 -Wno-narrowing -Wno-sign-compare -Wno-enum-compare
else
    CFLAGS += -s -O1 -Wl,--subsystem,windows
endif

# Additional flags for compiler (if desired)
#CFLAGS += -Wextra -Wmissing-prototypes -Wstrict-prototypes
# resource file contains windows executable icon and properties
# -Wl,--subsystem,windows hides the console window
CFLAGS += $(RAYLIB_PATH)/src/raylib.rc.data




# Define include paths for required headers
# NOTE: Several external required libraries (stb and others)
INCLUDE_PATHS = -I. -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external -Isrc -Iinclude -Iinclude/raylib 

# Define library paths containing required libs.
LDFLAGS = -L. -L$(RAYLIB_RELEASE_PATH) -L$(RAYLIB_PATH)/src


# Define any libraries required on linking
# if you want to link libraries (libname.so or libname.a), use the -lname

# Libraries for Windows desktop compilation
# NOTE: WinMM library required to set high-res timer resolution
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
# Required for physac examples
#LDLIBS += -static -lpthread


# Define a recursive wildcard function
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

# Define all source files required
SRC_DIR = src
OBJ_DIR = obj

# Define all object files from source files
SRC = $(call rwildcard, *.c, *.h, *cpp)
#OBJS = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
OBJS ?= main.cpp

MAKEFILE_PARAMS = $(PROJECT_NAME)


# Default target entry
# NOTE: We call this Makefile target or Makefile.Android target
all:
	$(MAKE) $(MAKEFILE_PARAMS)

# Project target defined by PROJECT_NAME
$(PROJECT_NAME): $(OBJS)
	$(CC) -o build/$(PROJECT_NAME)$(EXT) $(OBJS) $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM)

# Compile source files
# NOTE: This pattern will compile every module defined on $(OBJS)
#%.o: %.c
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o build/$@ $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Clean everything
clean:
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
		del *.o *.exe /s
    endif
    
	@echo Cleaning done
endif
