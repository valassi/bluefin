topdir := $(dir $(lastword $(MAKEFILE_LIST)))
# BlueFin public headers must be included as #include "BlueFin/xxx.h"
BFINC := $(topdir)
topdir := $(topdir:./%=%)
srcdir := $(topdir)src/
utidir := $(topdir)utilities/

#---------------------------------------------------------------------------

# USER-DEFINED CHOICE
# Choose the build mode (opt or dbg): use opt
###LCG_mode := dbg
LCG_mode := opt

# USER-DEFINED CHOICE
# Choose two user-defined locations for LCG AA releases and externals.
# If only one is specified but not the other, an error is generated.
# If both are specified, they must exist, else an error is generated.
# If not specified, LCG AA releases and externals are looked for first 
# in CVMFS and then in AFS; if neither works, an error is generated.
###OPT_LCGAPP := /opt/sw/lcg/app/releases
###OPT_LCGEXT := /opt/sw/lcg/external
OPT_LCGAPP :=
OPT_LCGEXT :=

#---------------------------------------------------------------------------

# Architecture (x86_64 or i686): use __ONLY__ 64-bit builds
LCG_arch := x86_64

# Compiler: use gcc43 on SLC5 and gcc46 on SLC6 (for better tests)
# NB: gcc43 is only supported on SLC5 (use gcc46 or higher on SLC6)
LCG_compiler_slc5 := gcc43
LCG_compiler_slc6 := gcc46

#---------------------------------------------------------------------------

# App release area and external area on CVMFS and AFS
CVMFS_LCGAPP := /cvmfs/sft.cern.ch/lcg/app/releases
CVMFS_LCGEXT := /cvmfs/sft.cern.ch/lcg/external
AFS_LCGAPP := /afs/cern.ch/sw/lcg/app/releases
AFS_LCGEXT := /afs/cern.ch/sw/lcg/external

# App release area - try user-defined directory if specified
LCGAPP = 
ifneq ($(OPT_LCGAPP),)
  ifneq ($(shell ls -d $(OPT_LCGAPP)),)
    LCGAPP = $(OPT_LCGAPP)
  else
    $(warning WARNING! User app directory "$(OPT_LCGAPP)" does not exist - try CVMFS)
  endif
endif
# App release area - try CVMFS
ifeq ($(LCGAPP),)
  ifneq ($(shell ls -d $(CVMFS_LCGAPP) 2> /dev/null),)
    LCGAPP = $(CVMFS_LCGAPP)
  else
    ###$(warning WARNING! CVMFS app directory "$(CVMFS_LCGAPP)" does not exist - try AFS)
  endif
endif
# App release area - try AFS
ifeq ($(LCGAPP),)
  ifneq ($(shell ls -d $(AFS_LCGAPP)),)
    LCGAPP = $(AFS_LCGAPP)
  else
    $(error ERROR! AFS app directory "$(CVMFS_LCGAPP)" does not exist)
  endif
endif
# App release area - info
$(info LCGAPP=$(LCGAPP))

# External area - try user-defined directory if specified
LCGEXT = 
ifneq ($(OPT_LCGEXT),)
  ifneq ($(shell ls -d $(OPT_LCGEXT)),)
    LCGEXT = $(OPT_LCGEXT)
  else
    $(warning WARNING! User ext directory "$(OPT_LCGEXT)" does not exist - try CVMFS)
  endif
endif
# External area - try CVMFS
ifeq ($(LCGEXT),)
  ifneq ($(shell ls -d $(CVMFS_LCGEXT) 2> /dev/null),)
    LCGEXT = $(CVMFS_LCGEXT)
  else
    ###$(warning WARNING! CVMFS ext directory "$(CVMFS_LCGEXT)" does not exist - try AFS)
  endif
endif
# External area - try AFS
ifeq ($(LCGEXT),)
  ifneq ($(shell ls -d $(AFS_LCGEXT)),)
    LCGEXT = $(AFS_LCGEXT)
  else
    $(error ERROR! AFS ext directory "$(CVMFS_LCGEXT)" does not exist)
  endif
endif
# External area - info
$(info LCGEXT=$(LCGEXT))

# Determine the O/S: use "SLC5" (also SL5/RHEL5) or "SLC6" (also SL6/RHEL6)
# Example: Scientific Linux CERN SLC release 6.4 (Carbon) - lxplus6
# Example: Scientific Linux CERN SLC release 5.9 (Boron) - lxplus5
# Example: Scientific Linux SL release 5.7 (Boron) - cernvm
# Example: Red Hat Enterprise Linux Server release 5.9 (Tikanga) - oracle
RH := $(shell more /etc/redhat-release)
ifeq ($(RH),)
  $(error ERROR! O/S is not a RedHat-based Linux system)
endif
ifeq ($(wordlist 1,5,$(RH)),Scientific Linux CERN SLC release)
  LCG_os := slc$(firstword $(subst ., ,$(word 6,$(RH))))
else
ifeq ($(wordlist 1,4,$(RH)),Scientific Linux SL release)
  LCG_os := slc$(firstword $(subst ., ,$(word 5,$(RH))))
else
ifeq ($(wordlist 1,6,$(RH)),Red Hat Enterprise Linux Server release)
  LCG_os := slc$(firstword $(subst ., ,$(word 7,$(RH))))
else
  $(error ERROR! O/S "$(RH)" is not an SLC/SL/RHEL Linux system)
endif
endif
endif
ifeq ($(LCG_os),slc5)
else
ifeq ($(LCG_os),slc6)
else
  $(error ERROR! O/S "$(RH)" is neither SLC/SL/RHEL5 nor SLC/SL/RHEL6)
endif
endif

# Determine the compiler depending on the O/S
LCG_compiler := $(LCG_compiler_$(LCG_os))

# Determine the compiler flags for the chosen architecture
ifeq ($(LCG_arch),x86_64)
  CPPFLAGS = -m64
  ELFOBJ = elf64-x86-64
else
  $(error ERROR! The "$(LCG_arch)" architecture is not supported)
endif

# Determine the compiler flags for the chosen build mode
ifeq ($(LCG_mode),dbg)
  CPPFLAGS += -g
else
ifeq ($(LCG_mode),opt)
  CPPFLAGS += -O2
else
  $(error ERROR! The "$(LCG_mode)" compiler mode is not supported)
endif
endif

# Determine additional compiler flags (-fPIC is needed to make shared libs)
CPPFLAGS += -ansi -Wall -W -pthread
CPPFLAGS += -fPIC
###CPPFLAGS += -Df2cFortran -D_GNU_SOURCE -Dlinux -Dunix -pipe

# Automatic make dependencies
# See http://mad-scientist.net/make/autodep.html
CPPFLAGS += -MMD

# Determine the LCG platform for the chosen O/S, architecture, compiler, mode
LCG_basesystem := $(LCG_arch)-$(LCG_os)-$(LCG_compiler)-opt
LCG_system := $(LCG_arch)-$(LCG_os)-$(LCG_compiler)-$(LCG_mode)

# Determine the bindir for the chosen LCG platform (mind the trailing "/"!)
bindir := $(topdir)$(LCG_system)/

#---------------------------------------------------------------------------

# Set up GCC for the chosen compiler version as in LCGCMT_64b
# See http://svnweb.cern.ch/world/wsvn/lcgsoft/tags/LCGCMT_64d/lcgcmt/LCG_Settings/cmt/requirements
ifeq ($(LCG_compiler),gcc43)
  GCCHOMESUFFIX := gcc/4.3.6/$(LCG_arch)-$(LCG_os)
  ifeq ($(LCG_os),slc6)
    $(error ERROR! The "$(LCG_compiler)" compiler is not supported on $(LCG_os))
  endif
else
ifeq ($(LCG_compiler),gcc46)
  GCCHOMESUFFIX := gcc/4.6.2/$(LCG_arch)-$(LCG_os)
else
  $(error ERROR! The "$(LCG_compiler)" compiler is not supported)
endif
endif
GCCHOME := $(LCGEXT)/$(GCCHOMESUFFIX)
ifeq ($(shell ls -d $(GCCHOME)),)
  $(error INTERNAL ERROR! Directory "$(GCCHOME)" does not exist?)
endif
GCC := $(GCCHOME)/bin/gcc

# Set up python 2.6.5p2 as in LCGCMT_64b
# See http://svnweb.cern.ch/world/wsvn/lcgsoft/tags/LCGCMT_64b/lcgcmt/LCG_Configuration/cmt/requirements
###PYTHONHOME := $(LCGEXT)/Python/2.6.5p2/$(LCG_basesystem)
###ifeq ($(shell ls -d $(PYTHONHOME)),)
###  $(error INTERNAL ERROR! Directory "$(PYTHONHOME)" does not exist?)
###endif
###[add $(PYTHONHOME)/lib to LD_LIBRARY_PATH]
###[add $(PYTHONHOME)/bin to PATH]

# Set up Boost 1.48 as in LCGCMT_64b
# See http://svnweb.cern.ch/world/wsvn/lcgsoft/tags/LCGCMT_64b/lcgcmt/LCG_Configuration/cmt/requirements
# [NB Boost libraries are only needed for the Boost timer functionalities]
# [NB Boost matrix/vector are inlined in a header and require no libraries]
BOOSTHOME := $(LCGEXT)/Boost/1.48.0_python2.6/$(LCG_basesystem)
ifeq ($(shell ls -d $(BOOSTHOME)),)
  $(error INTERNAL ERROR! Directory "$(BOOSTHOME)" does not exist?)
endif
BOOSTINC := $(BOOSTHOME)/include/boost-1_48
BOOSTLINK := -L$(BOOSTHOME)/lib -lboost_timer-$(LCG_compiler)-mt-1_48

# Set up ROOT 5.34.03 as in LCGCMT_64b
# See http://svnweb.cern.ch/world/wsvn/lcgsoft/tags/LCGCMT_64b/lcgcmt/LCG_Configuration/cmt/requirements
ROOTSYSSUFFIX := ROOT/5.34.03/$(LCG_system)/root
ifeq ($(shell ls -d $(ROOTSYS)),)
  $(error INTERNAL ERROR! Directory "$(ROOTSYS)" does not exist?)
endif
ROOTSYS := $(LCGAPP)/$(ROOTSYSSUFFIX)
ROOTINC := $(ROOTSYS)/include
ROOTLINK := -L$(ROOTSYS)/lib -lMinuit2

# Set up GSL 1.10 as in LCGCMT_64b
# See http://svnweb.cern.ch/world/wsvn/lcgsoft/tags/LCGCMT_64b/lcgcmt/LCG_Configuration/cmt/requirements
###GSLHOME := $(LCGEXT)/GSL/1.10/$(LCG_basesystem)
###ifeq ($(shell ls -d $(GSLHOME)),)
###  $(error INTERNAL ERROR! Directory "$(GSLHOME)" does not exist?)
###endif
###GSLINC := $(GSLHOME)/include
###GSLLINK := -L$(GSLHOME)/lib -lgsl -lgslcblas
###[add $(GSLHOME)/lib to LD_LIBRARY_PATH]

# Set up LD_LIBRARY_PATH and export it to make it available while building in make
# See http://www.cmcrossroads.com/article/basics-getting-environment-variables-gnu-make
export LD_LIBRARY_PATH=$(GCCHOME)/lib64:$(ROOTSYS)/lib:$(BOOSTHOME)/lib:$(realpath $(bindir))

#-----------------------------------------------------------------------------

ALL = libBlueFin.so bluefin

# NB: prebuilds is a separate target ("make prebuilds")
all : $(ALL:%=$(bindir)%)
	@echo "Build completed for $(ALL)"
	@echo "--------------------------------"
	@echo "To set up the environment, type (csh or bash):"
	@echo "  eval \`make -f $(topdir)Makefile setup_csh\`"
	@echo "  eval \`make -f $(topdir)Makefile setup_sh\`"

# NB: prebuilds is a separate target, it should not be cleaned away
clean :
	\rm -rf $(bindir)

.SUFFIXES :

OBJSLIB := BlueFish.o BlueFish1Obs.o InfoMinimizer.o InfoAnalyzer.o TextReporter.o InputParser.o
$(bindir)libBlueFin.so : $(OBJSLIB:%.o=$(bindir)%.o)
	$(GCC) $(CPPFLAGS) -shared $(ROOTLINK) -o $@ $^

OBJSBF := BlueFinLogo.o BlueFinReport.o bluefin.o
$(bindir)bluefin : $(OBJSBF:%.o=$(bindir)%.o) $(OBJSLIB:%.o=$(bindir)%.o)
	$(GCC) $(CPPFLAGS) $(ROOTLINK) -o $@ $(filter %.o,$^) 

BLUEFINLINK := -L$(bindir) -lBlueFin

OBJS := $(OBJSLIB) $(OBJSBF)
$(OBJS) : %.o : $(bindir)%.o

ifeq "$(FIXDEPEND)" ""
ifeq "$(topdir)" ""
FIXDEPEND = @cat $(@:%.o=%.d) | sed 's/ / \\\n/g' | egrep -v '^(| )\\$$' | egrep -v '($(LCGAPP)|$(LCGEXT))' | sed 's|^$(bindir)|$$\(bindir\)|g' > $(@:%.o=%.P); \rm -f $(@:%.o=%.d)
else
FIXDEPEND = @cat $(@:%.o=%.d) | sed 's/ / \\\n/g' | egrep -v '^(| )\\$$' | egrep -v '($(LCGAPP)|$(LCGEXT))' | sed 's|^$(bindir)|$$\(bindir\)|g' | sed 's|^$(topdir)|$$\(topdir\)|g' > $(@:%.o=%.P); \rm -f $(@:%.o=%.d)
endif
endif

$(bindir)InfoMinimizer.o : $(srcdir)InfoMinimizer.cpp $(bindir).d
	$(GCC) $(CPPFLAGS) -I$(BFINC) -I$(BOOSTINC) -I$(ROOTINC) -c -o $@ $<
	$(FIXDEPEND)

$(bindir)%.o : $(srcdir)%.cpp $(bindir).d
	$(GCC) $(CPPFLAGS) -I$(BFINC) -I$(BOOSTINC) -c -o $@ $<
	$(FIXDEPEND)

$(bindir)%.o : $(utidir)%.cpp $(bindir).d
	$(GCC) $(CPPFLAGS) -I$(BFINC) -I$(BOOSTINC) -c -o $@ $<
	$(FIXDEPEND)

$(bindir)%.o : $(utidir)%.jpg
	cd $(utidir); objcopy -Bi386 --input binary --output $(ELFOBJ) $(notdir $^) ../$(LCG_system)/$(notdir $@)

$(bindir)%.o : $(utidir)%.tex
	cd $(utidir); objcopy -Bi386 --input binary --output $(ELFOBJ) $(notdir $^) ../$(LCG_system)/$(notdir $@)

$(bindir).d :
	if [ ! -d $(bindir) ]; then mkdir -p $(bindir); touch $@; fi

libBlueFin.so : $(bindir)libBlueFin.so

bluefin : $(bindir)bluefin

lib : libBlueFin.so

setup_csh :
	@echo "setenv ROOTSYS $(ROOTSYS); setenv PATH $(realpath $(bindir)):$(GCCHOME)/bin:\$${PATH}; setenv LD_LIBRARY_PATH $(LD_LIBRARY_PATH):\$${LD_LIBRARY_PATH}"

setup_sh :
	@echo "export ROOTSYS=$(ROOTSYS); export PATH=$(realpath $(bindir)):$(GCCHOME)/bin:\$${PATH}; export LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):\$${LD_LIBRARY_PATH}"

# NB: The 'prebuilds' environment ONLY sets LD_LIBRARY_PATH for ROOT and BOOST
prebuilds: bluefin
	@mkdir -p $(topdir)prebuilt/$(LCG_os)/
	@\cp -dpr $(bindir)/bluefin $(topdir)prebuilt/$(LCG_os)/
	@echo "setenv LD_LIBRARY_PATH $(CVMFS_LCGEXT)/$(GCCHOMESUFFIX)/lib64:$(CVMFS_LCGEXT)/$(ROOTSYSSUFFIX)/lib:$(AFS_LCGEXT)/$(GCCHOMESUFFIX)/lib64:$(AFS_LCGEXT)/$(ROOTSYSSUFFIX)/lib:\$${LD_LIBRARY_PATH}" > $(topdir)prebuilt/$(LCG_os)/setup.csh
	@echo "export LD_LIBRARY_PATH=$(CVMFS_LCGEXT)/$(GCCHOMESUFFIX)/lib64:$(CVMFS_LCGEXT)/$(ROOTSYSSUFFIX)/lib:$(AFS_LCGEXT)/$(GCCHOMESUFFIX)/lib64:$(AFS_LCGEXT)/$(ROOTSYSSUFFIX)/lib:\$${LD_LIBRARY_PATH}" > $(topdir)prebuilt/$(LCG_os)/setup.sh

.PHONY : all clean $(ALL) lib $(OBJS) prebuilds

vpath % $(topdir)
-include $(OBJS:%.o=$(bindir)%.P)
