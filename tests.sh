#!/bin/bash

cd `dirname $0`

#-------------------------------------------------------------------------------
# Setup the environment and build the code
#-------------------------------------------------------------------------------

# Setup the build and runtime environment from LCG views
. setupLCG.sh
if [ "$?" != "0" ]; then exit 1; fi

# Print all variables
make -f Makefile print

# Build all targets
make -f Makefile
if [ "$?" != "0" ]; then exit 1; fi

# Setup the runtime environment for BLUEFIN
eval `make -f ./Makefile setup_sh`

#-------------------------------------------------------------------------------
# Execute the tests
#-------------------------------------------------------------------------------

dataDirs=""
dataDirs="$dataDirs examples/dataTOP"
dataDirs="$dataDirs examples/dataXSE"
dataDirs="$dataDirs examples/dataSWW"
dataDirs="$dataDirs examples/dataBRW"

minOpts=
###minOpts=-M4

for dataDir in $dataDirs; do

  dataDir=`cd $dataDir; pwd`
  files=`cd $dataDir; \ls *.bfin`
  ###files=`cd $dataDir; \ls *Paper4*.bfin`

  for file in $files; do
    file=${file%.bfin}
    infile=$dataDir/${file}.bfin
    outfile=$dataDir/${file}_bluefin.log
    reffile=$dataDir/${file}_bluefin.log.ref
    outtexfile=$dataDir/${file}.tex
    reftexfile=$dataDir/${file}.tex.ref
    outtexbody=$dataDir/${file}_body.tex
    reftexbody=$dataDir/${file}_body.tex.ref
    outpdffile=$dataDir/${file}.pdf
    outfile1=$dataDir/${file}_bluefin1.log
    outfile2=$dataDir/${file}_bluefin2.log
    outfile1s=$dataDir/${file}_bluefin1s.log
    outfile2s=$dataDir/${file}_bluefin2s.log
    unset BFEXTRAFOOTER
    if [ "$file" == "lhc2012" ]; then export BFEXTRAFOOTER="WARNING! This is an example where inputs are taken from Table 4 in CMS PAS TOP-12-001. The output results differ because of rounding errors in those inputs."; fi
    if [ "$file" == "sww" ]; then export BFEXTRAFOOTER="LEP Combination for the Summer 2001 Conferences (http://lepewwg.web.cern.ch/LEPEWWG/lepww/4f/Summer01). The output results may differ because of rounding errors."; fi
    echo "----------------------------------------------------------------------"
    echo Create $outfile and $outtexbody and $outpdffile
    if [ -f $outfile ]; then \rm -f $outfile; fi
    if [ -f $outtexfile ]; then \rm -f $outtexbody; fi
    if [ -f $outfile1 ]; then \rm -f $outfile1; fi
    if [ -f $outfile1s ]; then \rm -f $outfile1s; fi
    echo bluefin -q $infile -o $dataDir -t $dataDir -c2 $minOpts
    bluefin -q $infile -o $dataDir -t $dataDir -c2 $minOpts
    if [ "$?" != "0" ]; then 
      echo "ERROR! bluefin -q $infile -o $dataDir -t $dataDir FAILED!"
      ###echo cat $outfile
      ###cat $outfile
      #echo tail $outfile
      #tail -10 $outfile
      continue
    fi
    ###cat $outfile
    \mv $outfile $outfile1
    cat $outfile1 | awk -vp=1 '{if ($1$2$3=="Createlatexreport:"){p=0}; if(p==1) print $0;}' > $outfile1s
    echo "----------------------------------------------------------------------"
    echo "Create $outfile (skip latex)"
    if [ -f $outfile ]; then \rm -f $outfile; fi
    if [ -f $outfile2 ]; then \rm -f $outfile2; fi
    if [ -f $outfile2s ]; then \rm -f $outfile2s; fi
    echo bluefin $infile -o $dataDir -T -c2 $minOpts
    bluefin $infile -o $dataDir -T -c2 $minOpts
    if [ "$?" != "0" ]; then 
      echo "ERROR! bluefin $infile -o $dataDir -T -c2 $minOpts FAILED!"
      ###echo cat $outfile
      ###cat $outfile
      #echo tail $outfile
      #tail -10 $outfile
      continue
    fi
    ###cat $outfile
    \mv $outfile $outfile2
    cat $outfile2 | awk -vp=1 '{if ($1$2$3=="Createlatexreport:"){p=0}; if(p==1) print $0;}' > $outfile2s
    #------------------------------------------------------------
    # Compare logs with and without latex
    #------------------------------------------------------------
    ls -l $outfile1s $outfile2s
    ###echo "Compare $outfile1s to $outfile2s"
    echo "diff $outfile1s $outfile2s"
    diff $outfile1s $outfile2s
    if [ "$?" != "0" ]; then
      echo "ERROR! Logfiles differ with and without pdflatex?"
      exit 1
    fi
    ###echo tkdiff $outfile1s $outfile2s
    ###tkdiff $outfile1s $outfile2s &
    ###sleep 1 # wait until tkdiff has started before removing the file!
    #--- BEGIN cleanup
    \mv $outfile1 $outfile 
    \rm -f $outfile1s
    \rm -f $outfile2
    \rm -f $outfile2s
    #--- END cleanup
    #------------------------------------------------------------
    # Compare ref
    #------------------------------------------------------------
    ls -l $outfile $reffile
    ###echo "Compare $outfile to $reffile"
    echo "diff $outfile $reffile"
    diff $outfile $reffile
    ###echo tkdiff $outfile $reffile
    ###tkdiff $outfile $reffile &
    ###sleep 1 # wait until tkdiff has started before removing the file!
    #--- BEGIN create a new reference
    \cp $outfile $reffile
    #--- END create a new reference
    #\rm -f $outfile
    #------------------------------------------------------------
    # Compare ref tex file
    #------------------------------------------------------------
    ls -l $outtexfile $reftexfile
    ###echo "Compare $outexfile to $reftexfile"
    echo "diff $outtexfile $reftexfile"
    diff $outtexfile $reftexfile
    ###echo "tkdiff $outtexfile $reftexfile"
    ###tkdiff $outtexfile $reftexfile &
    ###sleep 1 # wait until tkdiff has started before removing the file!
    #--- BEGIN create a new reference
    \cp $outtexfile $reftexfile
    #--- END create a new reference
    #\rm -f $outtexfile
    #------------------------------------------------------------
    # Compare ref tex body
    #------------------------------------------------------------
    ls -l $outtexbody $reftexbody
    ###echo "Compare $outexbody to $reftexbody"
    echo "diff $outtexbody $reftexbody"
    diff $outtexbody $reftexbody
    ###echo "tkdiff $outtexbody $reftexbody"
    ###tkdiff $outtexbody $reftexbody &
    ###sleep 1 # wait until tkdiff has started before removing the file!
    #--- BEGIN create a new reference
    \cp $outtexbody $reftexbody
    #--- END create a new reference
    ###\rm -f $outtexbody
    \rm -f $dataDir/${file}.aux
    \rm -f $dataDir/${file}.log
    \rm -f $dataDir/${file}_body.aux
    \rm -f $dataDir/BlueFinLogo.jpg
  done
done

###make clean
