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
# If not specified, LCG AA releases and externals are looked for in CVMFS;
# if this does not work, an error is generated (AFS is no longer supported).
###OPT_LCGAPP := /opt/sw/lcg/app/releases
###OPT_LCGEXT := /opt/sw/lcg/external
OPT_LCGAPP :=
OPT_LCGEXT :=

#---------------------------------------------------------------------------

# Architecture (x86_64 or i686): use __ONLY__ 64-bit builds
LCG_arch := x86_64

# Compiler: use gcc43 on SLC5, gcc49 on SLC6 and gcc62 on CC7 (for better tests)
# NB: gcc43 is only supported on SLC5 (use gcc46 or higher on SLC6)
LCG_compiler_slc5    := gcc43
LCG_compiler_slc6    := gcc49
LCG_compiler_centos7 := gcc62

#---------------------------------------------------------------------------

# LCG releases on CVMFS
# [NB: AFS is no longer supported as of 2018]
CVMFS_LCGAPP := /cvmfs/sft.cern.ch/lcg/app/releases
CVMFS_LCGEXT := /cvmfs/sft.cern.ch/lcg/external
CVMFS_LCGREL := /cvmfs/sft.cern.ch/lcg/releases

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
    $(error ERROR! CVMFS app directory "$(CVMFS_LCGAPP)" does not exist)
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
    $(error ERROR! CVMFS ext directory "$(CVMFS_LCGEXT)" does not exist)
  endif
endif
# External area - info
$(info LCGEXT=$(LCGEXT))

# Private copy of the nightlies for CC7 (temporary)
OPT_LCGNGT := /opt/nightlies/dev3/20150311Wed

#---------------------------------------------------------------------------

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
ifeq ($(wordlist 1,3,$(RH)),CentOS Linux release)
  LCG_os := centos$(firstword $(subst ., ,$(word 4,$(RH))))
else
  $(error ERROR! O/S "$(RH)" is not an SLC/SL/RHEL/CentOS Linux system)
endif
endif
endif
endif
ifeq ($(LCG_os),slc5)
else
ifeq ($(LCG_os),slc6)
else
ifeq ($(LCG_os),centos7)
else
  $(error ERROR! O/S "$(RH)" is not SLC/SL/RHEL5, SLC/SL/RHEL6 or CC7)
endif
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

# Determine the compiler flags for the chosen compiler version
ifeq ($(LCG_compiler),gcc62)
  CPPFLAGS += -std=c++11
else
  CPPFLAGS += -ansi
endif

# Determine additional compiler flags (-fPIC is needed to make shared libs)
CPPFLAGS += -Wall -W -pthread
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

# Set up the latest GCC for the chosen compiler version
ifeq ($(LCG_compiler),gcc43)
  GCCHOMESUFFIX := gcc/4.3.6/$(LCG_arch)-$(LCG_os)
else
ifeq ($(LCG_compiler),gcc49)
  GCCHOMESUFFIX := gcc/4.9.1/$(LCG_arch)-$(LCG_os)
else
ifeq ($(LCG_compiler),gcc62)
  GCCHOMESUFFIX := gcc/6.2.0/$(LCG_arch)-$(LCG_os)
else
  $(error ERROR! The "$(LCG_compiler)" compiler is not supported)
endif
endif
endif
GCCHOME := $(CVMFS_LCGEXT)/$(GCCHOMESUFFIX)
ifeq ($(shell ls -d $(GCCHOME)),)
  $(error INTERNAL ERROR! Directory "$(GCCHOME)" does not exist?)
endif
GCC := $(GCCHOME)/bin/gcc
GCCLINK := -lm -L$(GCCHOME)/lib64 -lstdc++

#---------------------------------------------------------------------------

# Set up python 2.6.5p2 as in LCGCMT_64b
# See http://svnweb.cern.ch/world/wsvn/lcgsoft/tags/LCGCMT_64b/lcgcmt/LCG_Configuration/cmt/requirements
###PYTHONHOME := $(LCGEXT)/Python/2.6.5p2/$(LCG_basesystem)
###ifeq ($(shell ls -d $(PYTHONHOME)),)
###  $(error INTERNAL ERROR! Directory "$(PYTHONHOME)" does not exist?)
###endif
###[add $(PYTHONHOME)/lib to LD_LIBRARY_PATH]
###[add $(PYTHONHOME)/bin to PATH]

#---------------------------------------------------------------------------

# Set up Boost
# [NB Boost matrix/vector are inlined in a header and require no libraries]
ifeq ($(LCG_os),slc5)
# Set up Boost 1.48 as in LCGCMT_64b on slc5
# See http://svnweb.cern.ch/world/wsvn/lcgsoft/tags/LCGCMT_64b/lcgcmt/LCG_Configuration/cmt/requirements
BOOSTHOME := $(LCGEXT)/Boost/1.48.0_python2.6/$(LCG_basesystem)
BOOSTVERS := 1_48
ifeq ($(shell ls -d $(BOOSTHOME)),)
  $(error INTERNAL ERROR! Directory "$(BOOSTHOME)" does not exist?)
endif
else
ifeq ($(LCG_os),slc6)
# Set up Boost 1.55 as in LCGCMT_72a on slc6
# See http://svnweb.cern.ch/world/wsvn/lcgsoft/tags/LCGCMT_72a/lcgcmt/LCG_Configuration/cmt/requirements
BOOSTHOME := $(AFS_LCGREL)/LCG_72a/Boost/1.55.0_python2.7/$(LCG_basesystem)
BOOSTVERS := 1_55
ifeq ($(shell ls -d $(BOOSTHOME)),)
  $(error INTERNAL ERROR! Directory "$(BOOSTHOME)" does not exist?)
endif
else
ifeq ($(LCG_os),centos7)
# Set up Boost 1.66 as in LCG_93 on centos7
BOOSTHOME := $(CVMFS_LCGREL)/LCG_93/Boost/1.66.0/$(LCG_basesystem)
BOOSTVERS := 1_66
ifeq ($(shell ls -d $(BOOSTHOME)),)
  $(error INTERNAL ERROR! Directory "$(BOOSTHOME)" does not exist?)
endif
else
  $(error ERROR! O/S "$(RH)" is not SLC/SL/RHEL5, SLC/SL/RHEL6 or CC7)
endif
endif
endif
BOOSTINC := $(BOOSTHOME)/include/boost-$(BOOSTVERS)

#---------------------------------------------------------------------------

# Set up ROOT
ifeq ($(LCG_os),slc5)
# Set up ROOT 5.34.03 as in LCGCMT_64b
# See http://svnweb.cern.ch/world/wsvn/lcgsoft/tags/LCGCMT_64b/lcgcmt/LCG_Configuration/cmt/requirements
ROOTSYSSUFFIX := ROOT/5.34.03/$(LCG_system)/root
ROOTSYS := $(LCGAPP)/$(ROOTSYSSUFFIX)
TBBHOME :=
ifeq ($(shell ls -d $(ROOTSYS)),)
  $(error INTERNAL ERROR! Directory "$(ROOTSYS)" does not exist?)
endif
else
ifeq ($(LCG_os),slc6)
# Set up ROOT 5.34.25 as in LCGCMT_72a on slc6
# See http://svnweb.cern.ch/world/wsvn/lcgsoft/tags/LCGCMT_72a/lcgcmt/LCG_Configuration/cmt/requirements
ROOTSYSSUFFIX := ROOT/5.34.25/$(LCG_system)
ROOTSYS := $(AFS_LCGREL)/LCG_72a/$(ROOTSYSSUFFIX)
TBBHOME :=
ifeq ($(shell ls -d $(ROOTSYS)),)
  $(error INTERNAL ERROR! Directory "$(ROOTSYS)" does not exist?)
endif
else
ifeq ($(LCG_os),centos7)
# Set up ROOT 6.12.06 as in LCG_93 on centos7
ROOTSYSSUFFIX := ROOT/6.12.06/$(LCG_system)
ROOTSYS := $(CVMFS_LCGREL)/LCG_93/$(ROOTSYSSUFFIX)
TBBHOME := $(CVMFS_LCGREL)/LCG_93/tbb/2018_U1/$(LCG_system)
ifeq ($(shell ls -d $(ROOTSYS)),)
  $(error INTERNAL ERROR! Directory "$(ROOTSYS)" does not exist?)
endif
else
  $(error ERROR! O/S "$(RH)" is not SLC/SL/RHEL5, SLC/SL/RHEL6 or CC7)
endif
endif
endif
ROOTINC := $(ROOTSYS)/include
ifeq ($(TBBHOME),)
  ROOTLINK := -L$(ROOTSYS)/lib -lMathCore -lMinuit2
else
  ROOTLINK := -L$(ROOTSYS)/lib -lMathCore -lMinuit2 -L$(TBBHOME)/lib -ltbb
endif

#---------------------------------------------------------------------------

# Set up LD_LIBRARY_PATH and export it to make it available while building in make
# See http://www.cmcrossroads.com/article/basics-getting-environment-variables-gnu-make
ifeq ($(TBBHOME),)
export LD_LIBRARY_PATH=$(GCCHOME)/lib64:$(ROOTSYS)/lib:$(BOOSTHOME)/lib:$(realpath $(bindir))
else
export LD_LIBRARY_PATH=$(GCCHOME)/lib64:$(ROOTSYS)/lib:$(BOOSTHOME)/lib:$(TBBHOME)/lib:$(realpath $(bindir))
endif

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
	$(GCC) $(CPPFLAGS) -shared $(GCCLINK) $(ROOTLINK) -o $@ $^

OBJSBF := BlueFinLogo.o BlueFinReport.o bluefin.o
$(bindir)bluefin : $(OBJSBF:%.o=$(bindir)%.o) $(OBJSLIB:%.o=$(bindir)%.o)
	$(GCC) $(CPPFLAGS) $(GCCLINK) $(ROOTLINK) -o $@ $(filter %.o,$^) 

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
	@echo "if ( \$${?LD_LIBRARY_PATH} == 0 ) setenv LD_LIBRARY_PATH ''; "
	@echo "setenv ROOTSYS $(ROOTSYS); setenv PATH $(realpath $(bindir)):$(GCCHOME)/bin:\$${PATH}; setenv LD_LIBRARY_PATH $(LD_LIBRARY_PATH):\$${LD_LIBRARY_PATH}"

setup_sh :
	@echo "export ROOTSYS=$(ROOTSYS); export PATH=$(realpath $(bindir)):$(GCCHOME)/bin:\$${PATH}; export LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):\$${LD_LIBRARY_PATH}"

# NB: The 'prebuilds' environment ONLY sets LD_LIBRARY_PATH for ROOT and BOOST
# FIXME: The logic of ROOTSYSSUFFIX is now broken on SLC6 and especially on CC7 
prebuilds: bluefin
	@mkdir -p $(topdir)prebuilt/$(LCG_os)/
	@\cp -dpr $(bindir)/bluefin $(topdir)prebuilt/$(LCG_os)/
	@echo "setenv LD_LIBRARY_PATH $(CVMFS_LCGEXT)/$(GCCHOMESUFFIX)/lib64:$(CVMFS_LCGEXT)/$(ROOTSYSSUFFIX)/lib:\$${LD_LIBRARY_PATH}" > $(topdir)prebuilt/$(LCG_os)/setup.csh
	@echo "export LD_LIBRARY_PATH=$(CVMFS_LCGEXT)/$(GCCHOMESUFFIX)/lib64:$(CVMFS_LCGEXT)/$(ROOTSYSSUFFIX)/lib:\$${LD_LIBRARY_PATH}" > $(topdir)prebuilt/$(LCG_os)/setup.sh

.PHONY : all clean $(ALL) lib $(OBJS) prebuilds

vpath % $(topdir)
-include $(OBJS:%.o=$(bindir)%.P)
