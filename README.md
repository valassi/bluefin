<img src="utilities/BlueFinLogo.jpg"  width="10%">
# Welcome to BlueFin

BlueFin is a software package for Best Linear Unbiased Estimate Fisher Information aNalysis.

## Overview

This software is supported on SLC6 and CC7 machines (and other variants of RedHat 6 and 7 distributions).  
Support on CERNVM is incomplete because of problems with the pdflatex distribution on that platform.  

The CVMFS file system is required to use the ROOT, Boost, TBB and GCC libraries installed there.

To build the software: type `make`.

You can also use the prebuilt bluefin binary in `prebuilt/slc6` or `prebuilt/centos7`. 
In that case, please use the provided `setup.csh` and `setup.sh` scripts in the same directory.

Some examples of using bluefin are in the `tests.sh` script.

## More details

To download the latest version of the software:
- `git clone https://:@gitlab.cern.ch:8443/valassi/bluefin.git`

The package provides a software library, as well as an executable that builds a pdf report from an input data file in text format. To use the bluefin executable, source the setup.csh or setup.sh script and look at its usage:
- `source setup.sh; bluefin -h`

```
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

The ideas behind BlueFin are documented in the following article:
- "Information and treatment of unknown correlations in the combination of measurements using the BLUE method" ([10.1140/epjc/s10052-014-2717-6](https://doi.org/10.1140/epjc/s10052-014-2717-6))

For more details please refer to the documentation on this TRAC site.
- You may [browse](..) the source code of BlueFin on gitlab.
- Minimal release notes are available in the [release.notes](release.notes) file.
- An input data file example is available in the [xsePaper4.bfin](examples/dataXSE/xsePaper4.bfin)  file.
- The corresponding pdf report for this example is available in the [xsePaper4.pdf](examples/dataXSE/xsePaper4.pdf)  file.

## Contact

*WARNING! This software is in beta version.* 

Please report any problems to Andrea Valassi(at)cern.ch.
