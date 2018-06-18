###############################################################################
#
# Simple Makefile wrapper over cmake
#
# Author: Andrea Valassi, 31 August 2016
#
# Targets:
#   cmake     -> create cmake config ("cmake")
#   all       -> also build code ("make")
#   install   -> also install code ("make install")
#
# Additional targets (show build variables):
#   print-<v> -> show the value of variable/macro <v>
#   print     -> show a selection of relevant variables/macros
#
# Mandatory prerequisites:
#   - the cmake executable
#
# Recommended prerequisites:
#   - the CMAKEFLAGS variable
#     (this contains the command line options that are used in the cmake step)
#   - the CMAKE_PREFIX_PATH env variable
#     (this is used by CMake to resolve package dependencies of BLUEFIN)
#
# Optional settings:
#   - the BUILDDIR variable can be used to choose a different build dir
#     (the default BUILDDIR is build.$BINARY_TAG)
#
###############################################################################

#------------------------------------------------------------------------------

# Architecture (e.g. x86_64)
LCG_arch := $(shell uname -m)
#$(info LCG_arch=$(LCG_arch))

# Determine the O/S
ifneq ($(wildcard /etc/os-release),)
# 1. Determine the O/S from /etc/os-release
  LCG_os := $(shell cat /etc/os-release | grep ^ID= | sed 's/^ID="//' | sed 's/"$$//')
  LCG_os := $(LCG_os)$(shell cat /etc/os-release | grep ^VERSION_ID= | sed 's/^VERSION_ID="//' | sed 's/"$$//' | cut -d. -f1)
else
ifneq ($(wildcard /etc/redhat-releases),)
# 2. Determine the O/S from /etc/redhat-release
RH := $(shell more /etc/redhat-release)
ifeq ($(RH),)
  LCG_os := unknown
endif
ifeq ($(wordlist 1,5,$(RH)),Scientific Linux CERN SLC release)
  LCG_os := slc$(firstword $(subst ., ,$(word 6,$(RH))))
else
ifeq ($(wordlist 1,4,$(RH)),Scientific Linux SL release)
  LCG_os := sl$(firstword $(subst ., ,$(word 5,$(RH))))
else
ifeq ($(wordlist 1,6,$(RH)),Red Hat Enterprise Linux Server release)
  LCG_os := rhel$(firstword $(subst ., ,$(word 7,$(RH))))
else
ifeq ($(wordlist 1,3,$(RH)),CentOS Linux release)
  LCG_os := centos$(firstword $(subst ., ,$(word 4,$(RH))))
else
  LCG_os := unknown
endif
endif
endif
endif
else
# 3. Could not determine the O/S
  LCG_os := unknown
endif
endif

# Compiler version
LCG_compiler := gcc$(shell gcc -dumpversion | sed 's/\.//g')

# Build mode (opt or dbg)
ifeq ($(LCG_mode),)
  LCG_mode := opt
endif

# Determine the BINARY_TAG for the build
BINARY_TAG := $(LCG_arch)-$(LCG_os)-$(LCG_compiler)-$(LCG_mode)
###CMAKEFLAGS := "-DBINARY_TAG=$(BINARY_TAG) $(CMAKEFLAGS)"

# Define the default build directory as build.${BINARY_TAG}
BUILDDIR := $(CURDIR)/build.$(BINARY_TAG)

#-------------------------------------------------------------------------------
# Targets
#-------------------------------------------------------------------------------

# Phony targets
.PHONY: cmake print

# Execute the make step (default target)
all: cmake
	cmake --build $(BUILDDIR) --config .

# Execute the cmake step
cmake:
ifeq ($(wildcard $(BUILDDIR)/Makefile),)
ifeq ($(wildcard $(BUILDDIR)/build.ninja),)
	cmake $(CMAKEFLAGS) -H. -B$(BUILDDIR)
endif
endif

# Execute the install step
install: cmake
	@: # noop	
# [Presently a noop]
#	cmake --build $(BUILDDIR) --config . --target install | grep -v "^-- Up-to-date:"

# Print all Makefile variables/macros
# See https://stackoverflow.com/questions/22925380
print:
	$(info cmake is $(shell which cmake))
	$(info )
	$(foreach var,BINARY_TAG CMAKEFLAGS LCG_arch LCG_compiler LCG_mode LCG_os,$(info $(var)=$($(var))))
	$(info )
	@: # noop	

# Print individual Makefile variables/macros
# See https://stackoverflow.com/a/25817631
print-%: 
	@echo $($*) # or @echo $*=$($*)