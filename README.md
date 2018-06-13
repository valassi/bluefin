[[Image(source:/bluefin/trunk/utilities/BlueFinLogo.jpg,60px)]] 
= Welcome to !BlueFin =

!BlueFin is a software package for Best Linear Unbiased Estimate Fisher Information aNalysis.

To download the latest version of the software (needs CERN Kerberos authentication):
   * svn co !svn+ssh://svn.cern.ch/reps/bluefin/bluefin/tags/BLUEFIN_01_00_04
The package provides a software library, as well as an executable that builds a pdf report from an input data file in text format. To use the bluefin executable, source the setup.csh or setup.sh script and look at its usage:
   * source setup.csh; bluefin -h
{{{
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
}}}
You can use this software on most !RedHat-based systems (SL5/6, SLC5/6, RHEL5/6) with CVMFS installed. Support on CERNVM is incomplete because of problems with the pdflatex distribution on that platform. Type 'make' to build from sources. Alternatively, you may use a prebuilt version of the bluefin executable for slc6 from the "prebuilt" directory.

The ideas behind !BlueFin are documented in the following article:
   * "Information and treatment of unknown correlations in the combination of measurements using the BLUE method" ([http://link.springer.com/article/10.1140/epjc/s10052-014-2717-6 10.1140/epjc/s10052-014-2717-6])

For more details please refer to the documentation on this TRAC site.
   * You may [https://svnweb.cern.ch/trac/bluefin/browser browse] the source code of !BlueFin on TRAC.
   * Minimal documentation is available in the [source:/bluefin/trunk/README README] file. 
   * Minimal release notes are available in the [source:/bluefin/trunk/release.notes release.notes] file.
   * An input data file example is available in the [source:/bluefin/trunk/examples/dataXSE/xsePaper4.bfin xsePaper4.bfin] file.
   * The corresponding pdf report for this example is available in the [source:/bluefin/trunk/examples/dataXSE/xsePaper4.pdf xsePaper4.pdf] file.

''WARNING! This software is in beta version. Please report any problems to Andrea Valassi(at)cern.ch and Roberto Chierici(at)cern.ch.''

--- A.V. (August 2013)