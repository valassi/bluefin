<img src="utilities/BlueFinLogo.jpg"  width="10%">
# Welcome to BlueFin

BlueFin is a software package for Best Linear Unbiased Estimate Fisher Information aNalysis.

## Download

To download the latest version of the software:
- `git clone https://:@gitlab.cern.ch:8443/valassi/bluefin.git`

## Build

The software is built using CMake.

To build the software, some external packages such as Boost, ROOT and tbb are needed. You have two options:
1. Set up and build using the 'LCG view' software installed on cvmfs, using the scripts provided for this purpose.
2. Set up and build using your own preferred Boost, ROOT and tbb installations.

### Build against LCG views

This option is supported on SLC6 and CC7 machines with cvmfs installed, such as lxplus6 and lxplus7 at CERN.

Set up the LCG views appropriate to your O/S, build the software and set up its runtime environment:
```bash
. setupLCG.sh
make 
eval `make -f ./Makefile setup_sh`
```

This will create all relevant libraries and binaries in a separate build directory:
- `build.x86_64-slc6-gcc493-opt` on slc6, using the LCG view `/cvmfs/sft.cern.ch/lcg/views/LCG_72a/x86_64-slc6-gcc49-opt`
- `build.x86_64-centos7-gcc620-opt` on centos7, using the LCG view `/cvmfs/sft.cern.ch/lcg/views/LCG_93/x86_64-centos7-gcc62-opt`

It will also add the build directory to the `$PATH` and `$LD_LIBRARY_PATH` environment variables.

### Build against specific installations of Boost, ROOT and tbb

The following is an example on centos7, using individual Boost, ROOT and tbb installations in the LCG stack.  

You need to use a compiler and a set of compilation options compatible with those used for your chosen installations of ROOT and Boost.

The following example uses ninja instead of GNU make.

```bash
. /cvmfs/sft.cern.ch/lcg/contrib/gcc/6.2binutils/x86_64-centos7/setup.sh
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.6.0/Linux-x86_64/bin:${PATH}
export PATH=/cvmfs/sft.cern.ch/lcg/contrib/ninja/1.4.0/x86_64-slc6:${PATH}
export LD_LIBRARY_PATH=/cvmfs/sft.cern.ch/lcg/releases/LCG_93/Boost/1.66.0/x86_64-centos7-gcc62-opt/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=/cvmfs/sft.cern.ch/lcg/releases/LCG_93/ROOT/6.12.06/x86_64-centos7-gcc62-opt/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=/cvmfs/sft.cern.ch/lcg/releases/LCG_93/tbb/2018_U1/x86_64-centos7-gcc62-opt/lib:${LD_LIBRARY_PATH}
export CMAKE_PREFIX_PATH=/cvmfs/sft.cern.ch/lcg/releases/LCG_93/Boost/1.66.0/x86_64-centos7-gcc62-opt:${CMAKE_PREFIX_PATH}
export CMAKE_PREFIX_PATH=/cvmfs/sft.cern.ch/lcg/releases/LCG_93/ROOT/6.12.06/x86_64-centos7-gcc62-opt:${CMAKE_PREFIX_PATH}
export CMAKEFLAGS="-GNinja -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_CXX_FLAGS='-std=c++14 -m64' -DCMAKE_BUILD_TYPE=Release"
export VERBOSE=1
make

BUILDDIR=$(make print-BUILDDIR)
export PATH=${BUILDDIR}:${PATH}
export LD_LIBRARY_PATH=${BUILDDIR}:${LD_LIBRARY_PATH}
```

## The bluefin executable

The package provides a software library, as well as an executable that builds a pdf report from an input data file in text format. 

To use the `bluefin` executable, look at its usage:
```bash
  bluefin -h`

  Usage: bluefin [options] [<indir>/]<infile>.bfin
    Available options:
      -h           - print help message and exit
      -o <outdir>  - store/overwrite <infile>.pdf and <infile>_bluefin.log in <outdir>
                     [default: use the input directory <indir>]
      -t <texdir>  - store/overwrite tex and other temporary files in <texdir>
                     [default: use a temporary directory]
      -c (0..2)    - covariance printout: 0=none, 1=full, 2=all (full and partial)
                     [default: 2=all]
      -M (0..4)    - minimizations: 0=none, 1+=ByGlobFac, 2+=ByErrSrc, 3+=ByOffDiag, 4+=ByOffDiagPerErrSrc
                     [default: 3=all but the experimental ByOffDiagPerErrSrc]
```

You need to have a reasonably complete installation of `pdflatex`, such as those available on lxplus6 and lxplus7 at CERN.

Some examples of using bluefin are in the `tests.sh` script.

## Additional documentation

The ideas behind BlueFin are documented in the following article:
- "Information and treatment of unknown correlations in the combination of measurements using the BLUE method" ([10.1140/epjc/s10052-014-2717-6](https://doi.org/10.1140/epjc/s10052-014-2717-6))

For more details please refer to the documentation on this TRAC site.
- You may [browse](..) the source code of BlueFin on gitlab.
- Minimal release notes are available in the [release.notes](release.notes) file.
- An input data file example is available in the [xsePaper4.bfin](examples/dataXSE/xsePaper4.bfin)  file.
- The corresponding pdf report for this example is available in the [xsePaper4.pdf](examples/dataXSE/xsePaper4.pdf)  file.

## Contact

Please report any problems to Andrea Valassi(at)cern.ch.
