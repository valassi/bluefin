//============================================================================
//
// Copyright 2012 - 2015 CERN
//
// This file is part of the BlueFin software. BlueFin is distributed under
// the terms of the GNU General Public License version 3 (GPL Version 3),
// copied verbatim in file "COPYING". In applying this license, CERN does not
// waive the privileges and immunities granted to it by virtue of its status
// as an Intergovernmental Organization or submit itself to any jurisdiction.
//
// Original author of the software:
//   * Andrea Valassi (CERN) 2012 - 2015
//
// Based on ideas (http://arxiv.org/abs/1307.4003) by:
//   * Andrea Valassi (CERN)
//   * Roberto Chierici (Univ. Lyon)
//
//============================================================================
// Include files
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include "BlueFin/TextReporter.h"

// Namespace
using namespace bluefin;

//-----------------------------------------------------------------------------

namespace bluefin
{
  namespace TextReporterImpl
  {

    /// Print a "%x.2f" number as "0" or "~0" if required
    /// Print a color box if the value is lower than the given reference
    void latexPrintDot2( std::ostream& lStr,
                         const Number& x,
                         const std::string& suffix = "",
                         bool math = false,
                         const Number ref = -1 )
    {
      const bool relax = false;
      const bool quiet = true;
      std::string sOut = " ";
      if ( equals( x, 0, relax, quiet ) )
        sOut += "0";
      else
      {
        char out[80];
        snprintf( out, 10, "%9.2f", (double)x );
        if ( std::string(out) == "     0.00" || std::string(out) == "    -0.00" )
          sOut += ( !math ? "{\\tiny $\\sim$ }0" : "{\\scriptstyle \\sim }0" );
        else
          sOut += out;
      }
      if ( ref > 0 && x < ref )
      {
        if ( math ) sOut = "$" + sOut + "$"; // colorbox destroys outside math environment?
        if ( x < ref/3 ) lStr << " \\colorbox{SkyBlue1}{" << sOut << "}";
        else if ( x < ref/2 ) lStr << " \\colorbox{Cyan1}{" << sOut << "}";
        else lStr << " \\colorbox{Green1}{" << sOut << "}";
      }
      else lStr << sOut;
      lStr << suffix;
    }

    /// Print a "%x.3f" number as "0" or "~0" if required
    /// Print a color box if the value is higher than the given threshold
    void latexPrintDot3( std::ostream& lStr,
                         const Number& x,
                         const std::string& suffix = "",
                         const Number threshold = -1 )
    {
      const bool relax = false;
      const bool quiet = true;
      if ( equals( x, 0, relax, quiet ) )
        lStr << " 0";
      else
      {
        char out[80];
        snprintf( out, 10, "%9.3f", (double)x );
        if ( std::string(out) == "    0.000" || std::string(out) == "   -0.000" )
          lStr << " {\\tiny $\\sim$ }0";
        else
        {
          if ( threshold > 0 && x >= 3*threshold ) lStr << " \\colorbox{Tomato1}{" << out << "}";
          else if ( threshold > 0 && x >= 2*threshold ) lStr << " \\colorbox{Orange1}{" << out << "}";
          else if ( threshold > 0 && x >= threshold ) lStr << " \\colorbox{Yellow1}{" << out << "}";
          else lStr << " " << out;
        }
      }
      lStr << suffix;
    }

    /// Print a "%x.4f" number as "0" or "~0" if required
    void latexPrintDot4( std::ostream& lStr,
                         const Number& x,
                         const std::string& suffix = "" )
    {
      const bool relax = false;
      const bool quiet = true;
      if ( equals( x, 0, relax, quiet ) ) lStr << " 0";
      else
      {
        char out[80];
        snprintf( out, 10, "%9.4f", (double)x );
        if ( std::string(out) == "   0.0000" || std::string(out) == "  -0.0000" )
          lStr << " {\\tiny $\\sim$ }0";
        else lStr << " " << out;
      }
      lStr << suffix;
    }

    unsigned getDownscaleFactor( const Number maxAbs )
    {
      if ( maxAbs > 10000 ) return 10000;
      //else if ( maxAbs > 3000 ) return 1000;
      else if ( maxAbs > 100 ) return 100;
      //else if ( maxAbs > 30 ) return 10;
      else return 1;
    }

  }
}

//-----------------------------------------------------------------------------

unsigned TextReporter::getDownscaleFactor( const BlueFish& bf )
{
  Number maxAbs = 0;
  for ( size_t iMea=0; iMea<bf.nMea(); iMea++ )
    maxAbs = std::max( maxAbs, std::abs( bf.meaVal()(iMea) ) );
  maxAbs /= 10; // Tune for relative errors ~10%
  for ( size_t iMea=0; iMea<bf.nMea(); iMea++ )
    for ( size_t jMea=0; jMea<bf.nMea(); jMea++ )
      maxAbs = std::max( maxAbs, (long double)sqrt( bf.meaCov()(iMea,jMea) ) );
  return TextReporterImpl::getDownscaleFactor( maxAbs );
}

//-----------------------------------------------------------------------------

const std::string TextReporter::getDownscaleFactorTxt( const unsigned dscf, bool cov )
{
  if ( dscf == 10000 ) return ( cov ? "/100M" : "/10000" );
  //else if ( dscf == 1000 ) return ( cov ? "/1M" : "/1k" );
  else if ( dscf == 100 ) return ( cov ? "/10k" : "/100" );
  //else if ( dscf == 10 ) return ( cov ? "/100" : "/10" );
  else if ( dscf == 1 ) return "";
  else throw std::runtime_error( "INTERNAL ERROR! Unknown scale factor?" );
}

//-----------------------------------------------------------------------------

void
TextReporter::printMatrix( const std::string& tx7,
                           const Matrix& mat,
                           const std::vector<std::string>& eleNames,
                           unsigned dscf,
                           std::ostream& ostr,
                           bool latex,
                           const Matrix* pMatNomCor )
{
  std::ostream& tStr = ostr; // text stream ONLY
  size_t nEle = mat.size1();
  if ( nEle != mat.size2() )
    throw std::runtime_error( "Matrix is not symmetric in printMatrix" );
  if ( nEle != eleNames.size() )
    throw std::runtime_error( "#element names != matrix size in printMatrix" );
  if ( pMatNomCor )
  {
    if ( pMatNomCor->size1() != pMatNomCor->size2() )
      throw std::runtime_error( "Reference matrix is not symmetric in printMatrix" );
    if ( pMatNomCor->size1() != nEle )
      throw std::runtime_error( "Incompatible reference matrix size in printMatrix" );
  }
  // TEXT PRINTOUT
  if ( !latex )
  {
    char out[80];
    snprintf( out, 8, "%7s", tx7.c_str() );
    if ( dscf == 0 )
      throw std::runtime_error( "Downscale factor = 0 in printMatrix" );
    /*
    if ( dscf == 0 )
    {
      Number maxAbs = 0;
      for ( size_t iEle=0; iEle<nEle; ++iEle )
        for ( size_t jEle=0; jEle<nEle; ++jEle )
          maxAbs = std::max( maxAbs, (long double)sqrt( std::abs( mat(iEle,jEle) ) ) );
      dscf = TextReporterImpl::getDownscaleFactor( maxAbs );
    }
    */
    std::string dscfTxt = getDownscaleFactorTxt( dscf, true );
    dscf = dscf * dscf; // the square of the scale factor is used for covariance matrices
    // Now do the printout
    if ( dscfTxt != "" ) tStr << out << " (VALUES" << dscfTxt << ")" << std::endl; // additional line
    tStr << out; // also if dscfTxt != ""
    for ( size_t iEle=0; iEle<nEle; ++iEle )
    {
      snprintf( out, 8, "%7s", eleNames[iEle].c_str() );
      tStr << "   " << out;
    }
    tStr << std::endl;
    for ( size_t iEle=0; iEle<nEle; ++iEle )
    {
      snprintf( out, 8, "%7s", eleNames[iEle].c_str() );
      tStr << out;
      for ( size_t jEle=0; jEle<nEle; ++jEle )
      {
        snprintf( out, 10, "%9.4f", (double)(mat(iEle,jEle))/dscf );
        tStr << " " << out;
      }
      tStr << std::endl;
    }
  }
  // LATEX PRINTOUT
  else
  {
    std::ostream& lStr = ostr; // latex stream
    std::string dscfTxt = getDownscaleFactorTxt( dscf, true );
    dscf = dscf * dscf; // the square of the scale factor is used for covariance matrices
    // Begin table
    lStr << "\\begin{table}[H]" << std::endl;
    lStr << "\\scriptsize" << std::endl;
    lStr << "\\begin{center}" << std::endl;
    lStr << "\\renewcommand{\\arraystretch}{1.1}" << std::endl;
    // Element name tabular and element value array
    lStr << "\\begin{math}\\left(\\begin{array}{c|";
    bool math = true;
    for ( size_t colEle=0; colEle<nEle; ++colEle ) lStr << "c";
    lStr << "}" << std::endl;
    lStr << " & ";
    for ( size_t colEle=0; colEle<nEle; ++colEle )
      lStr << "\\mathrm{" << eleNames[colEle] << "}"
           << ( colEle < nEle-1 ? " & " : " \\\\" ) << std::endl;
    lStr << "\\hline" << std::endl;
    for ( size_t rowEle=0; rowEle<nEle; ++rowEle )
    {
      lStr << "\\mathrm{" << eleNames[rowEle] << "} \\hspace*{2pt} & ";
      // NB: comparison is only used for matrices where we assume positive correlations only
      for ( size_t colEle=0; colEle<nEle; ++colEle )
        TextReporterImpl::latexPrintDot2( lStr, mat(rowEle,colEle)/dscf,
                                          ( colEle < nEle-1 ? " & " : " \\\\" ), math,
                                          ( pMatNomCor ? (*pMatNomCor)(rowEle,colEle)/dscf : -1 ) );
      lStr << std::endl;
    }
    lStr << "\\end{array}\\right)\\end{math}" << std::endl;
    // Caption
    lStr << "\\caption{" << tx7;
    if ( dscfTxt != "" ) lStr << " Values " << dscfTxt + " are displayed.";
    lStr << "}" << std::endl;
    // End table
    lStr << "\\renewcommand{\\arraystretch}{1}" << std::endl;
    lStr << "\\end{center}" << std::endl;
    lStr << "\\end{table}" << std::endl;
  }
}

//-----------------------------------------------------------------------------

void
TextReporter::printBlueFish( const BlueFish& bf,
                             std::ostream& ostr,
                             bool latex )
{
  if ( bf.nObs() == 0 )
    throw std::runtime_error( "Empty BlueFish1Obs in printBlueFish" );
  // Hack to allow compilation even if this bf is not a bf1; and another
  // hack to avoid recomputing BF1 derivatives if bf already IS a bf1...
  const BlueFish1Obs bf1Copy = ( bf.nObs() == 1 ? BlueFish1Obs( bf ) : BlueFish1Obs() );
  const BlueFish1Obs& bf1 = ( dynamic_cast<const BlueFish1Obs*>( &bf ) != 0?
                              dynamic_cast<const BlueFish1Obs&>( bf ) : bf1Copy );
  const size_t aObs1 = 0; // there is a single observable in this BF
  unsigned dscf = getDownscaleFactor( bf ); // try to guess a scale factor for optimal printout
  std::string dscfTxt = getDownscaleFactorTxt( dscf );
  // TEXT PRINTOUT
  if ( !latex )
  {
    std::ostream& tStr = ostr; // text stream
    char out[80];
#if 1 // standard printout
    if ( dscfTxt == "" )
      tStr << "------------------------ |";
    else
    {
      tStr << "----";
      for ( size_t i=0; i<(6-dscfTxt.size())/2; i++ ) tStr << "-";
      tStr << " (VALUES" << dscfTxt << ") ";
      for ( size_t i=0; i<(6-dscfTxt.size())/2; i++ ) tStr << "-";
      tStr << "---- |";
    }
    for ( size_t colObs=0; colObs<bf.nObs(); colObs++ )
    {
      snprintf( out, 4, "%3s", bf.obsName(colObs).c_str() );
      tStr << " CVW" << out << " |";
    }
    if ( bf.nObs() == 1 )
    {
      tStr << " FIW" << out << " |";
      tStr << " MIW" << out << " |";
      tStr << " RIW" << out << " |";
    }
    for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
    {
      snprintf( out, 5, "%4s", bf.errNames()[iErr].c_str() );
      tStr << " " << out;
    }
    tStr << std::endl;
    for ( size_t rowObs=0; rowObs<bf.nObs(); rowObs++ )
    {
      tStr << std::endl;
      for ( size_t iMea=0; iMea<bf.nMea(); ++iMea )
      {
        if ( rowObs != bf.obsForMea( iMea ) ) continue;
        snprintf( out, 9, "%8s", bf.meaName(iMea).c_str() );
        tStr << out << " ";
        snprintf( out, 7, "%6.2f", (double)(bf.meaVal()(iMea))/dscf );
        tStr << out << " +- ";
        snprintf( out, 6, "%5.2f", (double)(sqrt(bf.meaCov()(iMea,iMea)))/dscf );
        tStr << out;
        tStr << " |";
        for ( size_t colObs=0; colObs<bf.nObs(); colObs++ )
        {
          snprintf( out, 7, "%6.2f", 100*(double)bf.blueCvw()(colObs,iMea) );
          tStr << " " << out;
          tStr << " |";
        }
        if ( bf.nObs() == 1 )
        {
          snprintf( out, 7, "%6.2f", 100*(double)bf1.blueFiw()(aObs1,iMea) );
          tStr << " " << out;
          tStr << " |";
          snprintf( out, 7, "%6.2f", 100*(double)bf1.blueMiw()(aObs1,iMea) );
          tStr << " " << out;
          tStr << " |";
          snprintf( out, 7, "%6.2f", 100*(double)bf1.blueRiw()(aObs1,iMea) );
          tStr << " " << out;
          tStr << " |";
        }
        for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
        {
          snprintf( out, 5, "%4.2f",
                    (double)(sqrt(bf.meaCovSources()[iErr](iMea,iMea)))/dscf );
          tStr << " " << out;
        }
        tStr << std::endl;
      }
      tStr << "------------------------ |";
      if ( bf.nObs() == 1 )
      {
        tStr << " ------ |";
        tStr << " ------ |";
        tStr << " ------ |";
      }
      for ( size_t colObs=0; colObs<bf.nObs(); colObs++ )
        tStr << " ------ |";
      for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
        tStr << " ----";
      tStr << std::endl;
      snprintf( out, 4, "%3s", bf.obsName(rowObs).c_str() );
      tStr << "BLUE " << out << " ";
      snprintf( out, 7, "%6.2f", (double)(bf.blueVal()(rowObs))/dscf );
      tStr << out << " +- ";
      snprintf( out, 6, "%5.2f", (double)(sqrt(bf.blueCov()(rowObs,rowObs)))/dscf );
      tStr << out;
      tStr << " |";
      for ( size_t colObs=0; colObs<bf.nObs(); colObs++ )
      {
        snprintf( out, 7, "%6.2f", 100*(double)bf.blueCvwTot()(colObs,rowObs) );
        tStr << " " << out;
        tStr << " |";
      }
      if ( bf.nObs() == 1 )
      {
        snprintf( out, 7, "%6.2f", 100*(double)bf1.blueFiwTot()(aObs1,aObs1) );
        tStr << " " << out;
        tStr << " |";
        snprintf( out, 7, "%6.2f", 100*(double)bf1.blueMiwTot()(aObs1,aObs1) );
        tStr << " " << out;
        tStr << " |";
        snprintf( out, 7, "%6.2f", 100*(double)bf1.blueRiwTot()(aObs1,aObs1) );
        tStr << " " << out;
        tStr << " |";
      }
      for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
      {
        snprintf( out, 5, "%4.2f",
                  (double)(sqrt(bf.blueCovSources()[iErr](rowObs,rowObs)))/dscf );
        tStr << " " << out;
      }
      tStr << std::endl;
      snprintf( out, 4, "%3s", bf.obsName(rowObs).c_str() );
      tStr << "BLUE " << out << " (full precision): ";
      snprintf( out, 10, "%9.5f", (double)bf.blueVal()(rowObs) );
      tStr << out << " +- ";
      snprintf( out, 8, "%7.5f", (double)sqrt(bf.blueCov()(rowObs,rowObs)) );
      tStr << out << std::endl;
    }
    tStr << "Chi2 / d.o.f = " << (double)bf.blueCh2()
         << " / " << bf.blueCh2Ndof() << std::endl;
    tStr << std::endl;
#else // ~LEP printout
    /*
    for ( size_t rowObs=0; rowObs<bf.nObs(); rowObs++ )
    {
      tStr << std::endl;
      for ( size_t iMea=0; iMea<bf.nMea(); ++iMea )
      {
        if ( rowObs != bf.obsForMea( iMea ) ) continue;
        snprintf( out, 6, "%5s", bf.meaName(iMea).c_str() );
        tStr << out << " ";
        snprintf( out, 7, "%6.0f", (double)(bf.meaVal()(iMea))/dscf );
        tStr << out << " ";
        snprintf( out, 5, "%4.0f", (double)(sqrt(bf.meaCov()(iMea,iMea)))/dscf );
        tStr << out << " ";
        for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
        {
          snprintf( out, 5, "%4.0f",
                    (double)(sqrt(bf.meaCovSources()[iErr](iMea,iMea)))/dscf );
          tStr << out;
        }
        tStr << "  ";
        for ( size_t colObs=0; colObs<bf.nObs(); colObs++ )
        {
          snprintf( out, 6, "%5.3f", (double)bf.blueCvw()(colObs,iMea) );
          tStr << " " << out;
        }
        tStr << std::endl;
      }
      snprintf( out, 4, "%3s", bf.obsName(rowObs).c_str() );
      tStr << " C" << out << " ";
      snprintf( out, 7, "%6.0f", (double)(bf.blueVal()(rowObs))/dscf );
      tStr << out << " ";
      snprintf( out, 5, "%4.0f", (double)(sqrt(bf.blueCov()(rowObs,rowObs)))/dscf );
      tStr << out << " ";
      for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
      {
        snprintf( out, 5, "%4.0f",
                  (double)(sqrt(bf.blueCovSources()[iErr](rowObs,rowObs)))/dscf );
        tStr << out;
      }
      tStr << "  ";
      for ( size_t colObs=0; colObs<bf.nObs(); colObs++ )
      {
        snprintf( out, 6, "%5.0f", (double)bf.blueCvwTot()(colObs,rowObs) );
        tStr << " " << out;
      }
      tStr << std::endl;
    }
    tStr << "Chi2 / d.o.f = " << (double)bf.blueCh2()
         << " / " << bf.blueCh2Ndof() << std::endl;
    tStr << std::endl;
    */
#endif
    // Correlations between BLUE's
    if ( bf.nObs() != 1 )
    {
      printMatrix( "BlueCor", bf.blueCor(), bf.obsNames(), 1, tStr );
      tStr << std::endl;
    }
  }
  // LATEX PRINTOUT
  else
  {
    std::ostream& lStr = ostr; // latex stream
    // LOOP ON OBSERVABLES
    for ( size_t rowObs=0; rowObs<bf.nObs(); rowObs++ )
    {
      // Begin table
      lStr << "\\begin{table}[H]" << std::endl;
      lStr << "\\scriptsize" << std::endl;
      lStr << "\\begin{center}" << std::endl;
      lStr << "\\renewcommand{\\arraystretch}{1.1}" << std::endl;
      // Begin tabular
      lStr << "\\begin{tabular}{|lc|";
      for ( size_t colObs=0; colObs<bf.nObs(); colObs++ ) lStr << "c|";
      if ( bf.nObs() == 1 ) lStr << "c|c|c|";
      for ( size_t iErr=0; iErr<bf.nErr(); ++iErr ) lStr << "c";
      lStr << "|}" << std::endl;
      // Tabular header
      lStr << "\\hline" << std::endl;
      lStr << "\\multicolumn{2}{|c|}{Measurements}";
      for ( size_t colObs=0; colObs<bf.nObs(); colObs++ )
      {
        lStr << " & CVW";
        if ( bf.nObs() != 1 ) lStr << "{\\tiny " << bf.obsName(colObs) << "}";
        lStr << "/\\% ";
      }
      if ( bf.nObs() == 1 )
      {
        lStr << " & IIW/\\% ";
        lStr << " & MIW/\\% ";
        lStr << " & RI/\\% ";
      }
      for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
      {
        std::string errName = bf.errNames()[iErr];
        errName.erase( std::remove( errName.begin(), errName.end(), '_' ), errName.end() );
        lStr << " & {\\tiny " << errName << "}";
      }
      lStr << "\\\\" << std::endl;
      // Tabular details
      lStr << "\\hline" << std::endl;
      for ( size_t iMea=0; iMea<bf.nMea(); ++iMea )
      {
        if ( rowObs != bf.obsForMea( iMea ) ) continue;
        lStr << bf.meaName(iMea) << " & ";
        TextReporterImpl::latexPrintDot2( lStr, bf.meaVal()(iMea)/dscf, " $\\pm$ " );
        TextReporterImpl::latexPrintDot2( lStr, sqrt(bf.meaCov()(iMea,iMea))/dscf, " & " );
        for ( size_t colObs=0; colObs<bf.nObs(); colObs++ )
          TextReporterImpl::latexPrintDot2( lStr, 100*(double)bf.blueCvw()(colObs,iMea), " & " );
        if ( bf.nObs() == 1 )
        {
          TextReporterImpl::latexPrintDot2( lStr, 100*(double)bf1.blueFiw()(aObs1,iMea), " & " );
          TextReporterImpl::latexPrintDot2( lStr, 100*(double)bf1.blueMiw()(aObs1,iMea), " & " );
          TextReporterImpl::latexPrintDot2( lStr, 100*(double)bf1.blueRiw()(aObs1,iMea), " & " );
        }
        for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
          TextReporterImpl::latexPrintDot2( lStr, sqrt(bf.meaCovSources()[iErr](iMea,iMea))/dscf,
                                            ( iErr<bf.nErr()-1 ? " & " : "" ) );
        lStr << "\\\\" << std::endl;
      }
      if ( bf.nObs() == 1 )
      {
        lStr << "Correlations & --- & --- & ";
        TextReporterImpl::latexPrintDot2( lStr, 100*(double)(1 - bf1.blueFiwTot()(aObs1,aObs1)), " & " );
        lStr << "--- & --- & ";
        for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
          lStr << "---" << ( iErr<bf.nErr()-1 ? " & " : "" );
        lStr << "\\\\" << std::endl;
      }
      lStr << "\\hline" << std::endl;
      // Tabular summary
      lStr << "BLUE {\\tiny " << bf.obsName(rowObs) << "} & ";
      TextReporterImpl::latexPrintDot2( lStr, bf.blueVal()(rowObs)/dscf, " $\\pm$ " );
      TextReporterImpl::latexPrintDot2( lStr, sqrt(bf.blueCov()(rowObs,rowObs))/dscf, " & " );
      for ( size_t colObs=0; colObs<bf.nObs(); colObs++ )
        TextReporterImpl::latexPrintDot2( lStr, 100*(double)bf.blueCvwTot()(colObs,rowObs), " & " );
      if ( bf.nObs() == 1 )
      {
        TextReporterImpl::latexPrintDot2( lStr, 100*(double)1, " & " );
        TextReporterImpl::latexPrintDot2( lStr, 100*(double)bf1.blueMiwTot()(aObs1,aObs1), " & " );
        TextReporterImpl::latexPrintDot2( lStr, 100*(double)bf1.blueRiwTot()(aObs1,aObs1), " & " );
      }
      for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
        TextReporterImpl::latexPrintDot2( lStr, sqrt(bf.blueCovSources()[iErr](rowObs,rowObs))/dscf,
                                          ( iErr<bf.nErr()-1 ? " & " : "" ) );
      lStr << "\\\\" << std::endl;
      lStr << "\\hline" << std::endl;
      // End tabular
      lStr << "\\end{tabular}" << std::endl;
      // Caption only on the last table
      if ( rowObs == bf.nObs()-1 )
      {
        if ( bf.nObs() == 1 )
        {
          lStr << "\\caption{BLUE of the combination ($\\chi^2$/ndof=";
          TextReporterImpl::latexPrintDot2( lStr, bf.blueCh2() );
          lStr << "/" << bf.blueCh2Ndof() << ")." << std::endl;
          if ( dscfTxt != "" ) lStr << " Values " << dscfTxt + " are displayed.";
          lStr << " For each input measurement $i$ the following are listed:";
          lStr << " the central value weight CVW$_i$ or $\\lambda_i$,";
          lStr << " the intrinsic information weight IIW$_i$ ,"; // or $\\eta_i$
          lStr << " the marginal information weight MIW$_i$,"; // or $\\mu_i$
          lStr << " the relative importance RI$_i$.";
          lStr << " The intrinsic information weight IIW$_{\\mathrm{corr}}$"
               << " of correlations is also shown on a separate row.}" << std::endl;
        }
        else
        {
          lStr << "\\caption{BLUE's of the combination ($\\chi^2$/ndof=";
          TextReporterImpl::latexPrintDot2( lStr, bf.blueCh2() );
          lStr << "/" << bf.blueCh2Ndof() << ")." << std::endl;
          if ( dscfTxt != "" ) lStr << " Values " << dscfTxt + " are displayed.";
          lStr << " For each input measurement $i$,";
          lStr << " the central value weight CVW or $\\lambda_i^\\alpha$";
          lStr << " with which that measurement contributes to the BLUE for observable $\\alpha$";
          lStr << " is listed.}" << std::endl;
        }
      }
      else
      {
        //lStr << "\\addtocounter{table}{-1}" << std::endl; // NOT NECESSARY WITHOUT CAPTION
      }
      // End table
      lStr << "\\renewcommand{\\arraystretch}{1}" << std::endl;
      lStr << "\\end{center}" << std::endl;
      lStr << "\\end{table}" << std::endl;
      // Adjust vertical spacing
      if ( rowObs != bf.nObs()-1 )
        lStr << "\\vspace*{-0.5cm}" << std::endl;
    }
    // Correlations between BLUE's
    if ( bf.nObs() != 1 )
      printMatrix( "Correlations between the BLUE's.",
                   bf.blueCor(), bf.obsNames(), 1, lStr, latex );
  }
}

//-----------------------------------------------------------------------------

void
TextReporter::printNInfoDerivativesBF1( const BlueFish1Obs& bf,
                                        const BlueFish1Obs& ref,
                                        std::ostream& ostr,
                                        bool latex,
                                        const std::string caption )
{
  // TEXT PRINTOUT
  if ( !latex )
  {
    std::ostream& tStr = ostr; // text stream
    tStr << "Text printout of normalized info derivatives is not (yet?) supported" << std::endl;
  }
  // LATEX PRINTOUT
  else
  {
    std::ostream& lStr = ostr; // latex stream
    const std::vector<TriangularMatrix>& d1Norm_ByOD_ByES = bf.blueD1NormInfByOffDiagElemByErrorSourceBF1( ref );
    const TriangularMatrix& d1Norm_ByOD = bf.blueD1NormInfByOffDiagElemBF1( ref );
    const std::vector<Number>& d1Norm_ByES = bf.blueD1NormInfByErrorSourceBF1( ref );
    const Number& d1Norm_ByGF = bf.blueD1NormInfByGlobalFactorBF1( ref );
    std::vector<Number> d1Norm_ByES_new; // sanity check - running total per table colum
    Number d1Norm_ByGF_new = 0; // sanity check - running grand total
    for ( size_t iErr=0; iErr<bf.nErr(); ++iErr ) d1Norm_ByES_new.push_back( 0 );
    lStr << "\\begin{table}[H]" << std::endl;
    lStr << "\\scriptsize" << std::endl;
    lStr << "\\begin{center}" << std::endl;
    lStr << "\\renewcommand{\\arraystretch}{1.1}" << std::endl;
    lStr << "\\begin{tabular}{|r|";
    for ( size_t iErr=0; iErr<bf.nErr(); ++iErr ) lStr << "r";
    lStr << "|r|}" << std::endl;
    lStr << "\\hline" << std::endl;
    lStr << " OffDiag \\& ErrSrc";
    for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
    {
      std::string errName = bf.errNames()[iErr];
      errName.erase( std::remove( errName.begin(), errName.end(), '_' ), errName.end() );
      lStr << " & {\\tiny " << errName << "}";
    }
    lStr << " & OffDiag\\\\" << std::endl;
    lStr << "\\hline" << std::endl;
    // Loop over off-diagonal elements
    for ( size_t iMea=0; iMea<bf.nMea(); ++iMea )
      for ( size_t jMea=0; jMea<iMea; ++jMea )
      {
        Number d1Norm_ByOD_new = 0; // sanity check - running total per table row
        lStr << bf.meaNames()[iMea] + " / " + bf.meaNames()[jMea] << " & ";
        // Loop over error sources
        for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
        {
          TextReporterImpl::latexPrintDot3( lStr, d1Norm_ByOD_ByES[iErr](iMea,jMea), " & ", 0.05 );
          d1Norm_ByOD_new += d1Norm_ByOD_ByES[iErr](iMea,jMea);
          d1Norm_ByES_new[iErr] += d1Norm_ByOD_ByES[iErr](iMea,jMea);
          d1Norm_ByGF_new += d1Norm_ByOD_ByES[iErr](iMea,jMea);
        }
        TextReporterImpl::latexPrintDot3( lStr, d1Norm_ByOD(iMea,jMea), " \\\\", 0.05 );
        lStr << std::endl;
        if ( !equals( d1Norm_ByOD_new, d1Norm_ByOD(iMea,jMea) ) )
          throw std::runtime_error( "d1Norm_ByOD mismatch in printNInfoDerivativesBF1" );
      }
    lStr << "\\hline" << std::endl;
    lStr << "\\multirow{2}{*}{ErrSrc} & ";
    for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
    {
      lStr << "\\multirow{2}{*}{";
      TextReporterImpl::latexPrintDot3( lStr, d1Norm_ByES[iErr], "", 0.05 );
      lStr << "} & ";
      if ( !equals( d1Norm_ByES_new[iErr], d1Norm_ByES[iErr] ) )
        throw std::runtime_error( "d1Norm_ByES mismatch in printNInfoDerivativesBF1" );
    }
    lStr << "GlobFact\\\\" << std::endl;
    lStr << " & ";
    for ( size_t iErr=0; iErr<bf.nErr(); ++iErr ) lStr << "& ";
    TextReporterImpl::latexPrintDot3( lStr, d1Norm_ByGF, " \\\\", 0.05 );
    lStr << std::endl;
    if ( !equals( d1Norm_ByGF_new, d1Norm_ByGF, true ) ) // relax (2E-14 differences found)
      throw std::runtime_error( "d1Norm_ByGF mismatch in printNInfoDerivativesBF1" );
    lStr << "\\hline" << std::endl;
    lStr << "\\end{tabular}" << std::endl;
    lStr << "\\renewcommand{\\arraystretch}{1}" << std::endl;
    lStr << "\\caption{Normalised Fisher information derivatives 1/I*dI/dX"
         << " for the combination under consideration.";
    lStr << " The derivatives in the table are computed with respect to scale factors X,"
         << " representing the ratio of a given correlation to its \"current\" value"
         << " in the combination under consideration,"
         << " and all normalized by the information I for the \"current\" values of correlations.";
    lStr << " They are computed for the \"current\" values of correlations"
         << ( caption != "" ? " (in this case: " + caption + ")" : caption ) << ".";
    lStr << " Color boxes indicate normalised derivatives greater than 0.05 (yellow),"
         << " 0.10 (orange) and 0.15 (red).";
    lStr << " The last column and last row list information derivatives when the same"
         << " rescaling factor is used for a given off-diagonal element or error source,"
         << " which are equal to the sums of individual derivatives in each row and column, respectively.";
    lStr << "}" << std::endl;
    lStr << "\\end{center}" << std::endl;
    lStr << "\\end{table}" << std::endl;
  }
}

//-----------------------------------------------------------------------------

void
TextReporter::printSummaryTableBF1( const BlueFish1Obs& bf1NomCor,
                                    const std::vector<std::pair<std::string,BlueFish1Obs> >& bf1s,
                                    std::ostream& ostr,
                                    bool latex )
{
  const size_t aObs1 = 0; // there is a single observable in these BFs
  unsigned dscf = getDownscaleFactor( bf1NomCor ); // try to guess a scale factor for optimal printout
  std::string dscfTxt = getDownscaleFactorTxt( dscf );
  // TEXT PRINTOUT
  if ( !latex )
  {
    std::ostream& tStr = ostr; // text stream
    tStr << "Text printout of summary table is not (yet?) supported" << std::endl;
  }
  // LATEX PRINTOUT
  else
  {
    std::ostream& lStr = ostr; // latex stream
    lStr << "\\begin{table}[h]" << std::endl;
    lStr << "\\scriptsize" << std::endl;
    lStr << "\\begin{center}" << std::endl;
    lStr << "\\renewcommand{\\arraystretch}{1.1}" << std::endl;
    lStr << "\\begin{tabular}{|l|c|";
    for ( size_t iErr=0; iErr<bf1NomCor.nErr(); ++iErr ) lStr << "c";
    lStr << "|c|}" << std::endl;
    lStr << "\\hline" << std::endl;
    lStr << "Combination & BLUE";
    for ( size_t iErr=0; iErr<bf1NomCor.nErr(); ++iErr )
    {
      std::string errName = bf1NomCor.errNames()[iErr];
      errName.erase( std::remove( errName.begin(), errName.end(), '_' ), errName.end() );
      lStr << " & {\\tiny " << errName << "}";
    }
    lStr << " & {\\tiny$\\chi^2$/ndof} \\\\" << std::endl;
    lStr << "\\hline" << std::endl;
    for ( std::vector<std::pair<std::string,BlueFish1Obs> >::const_iterator bf1i=bf1s.begin(); bf1i!=bf1s.end(); bf1i++ )
    {
      if ( bf1i->first == zeroCorrelationTag() ) lStr << "\\hline" << std::endl;
      lStr << bf1i->first << " & ";
      if ( bf1i->second.nObs() != 0 )
      {
        TextReporterImpl::latexPrintDot2( lStr, bf1i->second.blueVal()(aObs1)/dscf, " $\\pm$" );
        TextReporterImpl::latexPrintDot2( lStr, sqrt(bf1i->second.blueCov()(aObs1,aObs1))/dscf, " & " );
        for ( size_t iErr=0; iErr<bf1NomCor.nErr(); ++iErr )
          TextReporterImpl::latexPrintDot2( lStr, sqrt(bf1i->second.blueCovSources()[iErr](aObs1,aObs1))/dscf, " & " );
        TextReporterImpl::latexPrintDot2( lStr, bf1i->second.blueCh2() );
        lStr << "/" << bf1i->second.blueCh2Ndof();
      }
      else
      {
        lStr << "N/A & ";
        for ( size_t iErr=0; iErr<bf1NomCor.nErr(); ++iErr ) lStr << "N/A & ";
        lStr << "N/A";
      }
      lStr << "\\\\" << std::endl;
      if ( bf1i->first == nominalCorrelationTag() ) lStr << "\\hline" << std::endl;
    }
    lStr << "\\hline" << std::endl;
    lStr << "\\end{tabular}" << std::endl;
    lStr << "\\renewcommand{\\arraystretch}{1}" << std::endl;
    lStr << "\\caption{Summary table. BLUE's of the combinations "
         << "performed with nominal and modified correlations.";
    if ( dscfTxt != "" ) lStr << " Values " << dscfTxt + " are displayed.";
    lStr << "}" << std::endl;
    lStr << "\\end{center}" << std::endl;
    lStr << "\\end{table}" << std::endl;
    lStr << std::endl;
  }
}

//-----------------------------------------------------------------------------

void
TextReporter::printMinimization( const InfoMinimizer& im,
                                 const bool preOnly,
                                 std::ostream& ostr,
                                 bool latex,
                                 const std::string& msg )
{
  const size_t nPar = im.nPar();
  char out[80];
  // Unpack and perform sanity checks on the minimization results
  const std::set<size_t>& parToVary = im.parToVary();
  const std::vector<Number>& d1NInfosNomCor = im.d1NInfosNomCor();
  const std::vector<Number>& d1NInfosZerCor = im.d1NInfosZerCor();
  if ( d1NInfosNomCor.size() != nPar )
    throw std::runtime_error( "invalid d1NInfosNomCor size in printMinimization" );
  if ( d1NInfosZerCor.size() != nPar )
    throw std::runtime_error( "invalid d1NInfosZerCor size in printMinimization" );
  const bool minimized = !preOnly;
  if ( minimized )
  {
    if ( im.parMin().size() != nPar )
      throw std::runtime_error( "invalid parMin size in printMinimization" );
    if ( im.dparMin().size() != nPar )
      throw std::runtime_error( "invalid dparMin size in printMinimization" );
    if ( im.d1NInfosMinCor().size() != nPar )
      throw std::runtime_error( "invalid d1NInfosMinCor size in printMinimization" );
  }
  // TEXT PRINTOUT
  if ( !latex )
  {
    std::ostream& tStr = ostr; // text stream
    tStr << "DERIVATIVE ANALYSIS" << std::endl;
    if ( !minimized )
    {
      if ( !InfoMinimizer::varyAllParameters() )
        tStr << "Vary parameters if 1/I*dI/dX@1 > "
             << im.varParD1NInfoNomCorThreshold()
             << " and dI/dX@0 < 0" << std::endl;
      else
        tStr << "Vary parameters unless dI/dX@0 == dI/dX@1 == 0" << std::endl;
    }
    tStr << "----------------- | ----- | ------------------ |"
         << "-------------------------------|" << std::endl;
    tStr << "  Parameter name  | ParID |   Parameter value  |"
         << "           1/I*dI/dX           |" << std::endl;
    tStr << "                  |       | ScaleFactor X @MIN |"
         << "   @0    ->    @MIN ->      @1 |" << std::endl;
    tStr << "----------------- | ----- | ------------------ |"
         << "-------------------------------|" << std::endl;
    for ( size_t iPar=0, iVary=0; iPar<nPar; ++iPar )
    {
      // Parameter name
      snprintf( out, 16, "%-15s", im.parName(iPar).c_str() );
      tStr << " " << out << "  |";
      // Parameter id (5 char max)
      snprintf( out, 6, "%5s", im.parID(iPar).c_str() );
      tStr << " " << out << " |";
      // Parameter value at minimum
      if ( minimized )
      {
        snprintf( out, 8, "%7.4f", (double)im.parMin()[iPar] );
        if ( std::string(out) == " 0.0000" || std::string(out) == "-0.0000" )
          snprintf( out, 8, "%7s", "~0" );
      }
      else snprintf( out, 8, "%7s", "???" );
      tStr << " " << out << " +-";
      if ( minimized )
      {
        if ( parToVary.find(iPar) == parToVary.end() )
          snprintf( out, 8, "%7s", "N/A" ); // fixed
        else
        {
          snprintf( out, 8, "%7.4f", (double)im.dparMin()[iPar] );
          if ( std::string(out) == " 0.0000" || std::string(out) == "-0.0000" )
            snprintf( out, 8, "%7s", "~0" );
          else if ( std::string(out) == "-2.0000" )
            snprintf( out, 8, "%7s", "N/A" ); // fixed - no path to this point?
        }
      }
      else snprintf( out, 8, "%7s", "???" );
      tStr << " " << out << " |";
      // First derivatives
      snprintf( out, 8, "%7.4f", (double)d1NInfosZerCor[iPar] );
      if ( std::string(out) == " 0.0000" || std::string(out) == "-0.0000" )
        snprintf( out, 8, "%7s", "~0" );
      tStr << " " << out << " ->";
      if ( minimized )
      {
        snprintf( out, 8, "%7.4f", (double)im.d1NInfosMinCor()[iPar] );
        if ( std::string(out) == " 0.0000" || std::string(out) == "-0.0000" )
          snprintf( out, 8, "%7s", "~0" );
      }
      else snprintf( out, 8, "%7s", "???" );
      tStr << " " << out << " ->";
      snprintf( out, 8, "%7.4f", (double)d1NInfosNomCor[iPar] );
      if ( std::string(out) == " 0.0000" || std::string(out) == "-0.0000" )
        snprintf( out, 8, "%7s", "~0" );
      tStr << " " << out << " | ";
      // Dump which parameters will be varied and which will be fixed and why
      // Dump which parameters have been fixed
      // Check which parameters should be varied
      if ( parToVary.find(iPar) == parToVary.end() )
      {
        tStr << "%FIXED%";
        if ( ! InfoMinimizer::varyAllParameters() )
        {
          if ( !minimized && d1NInfosZerCor[iPar] > 0 )
            tStr << " (d/dX@0>0: NEGATIVE CORRELATIONS?)";
        }
      }
      else if ( !minimized )
      {
        tStr << "==> Vary in minimization #" << iVary++;
      }
      tStr << std::endl;
    }
    tStr << std::endl;
  }
  // LATEX PRINTOUT
  else
  {
    std::ostream& lStr = ostr; // latex stream
    //lStr << "\\paragraph{Derivative analysis "
    //     << ( minimized ? "after" : "before" ) << " minimization.}" << std::endl;
    //if ( !minimized )
    //  lStr << "Vary parameters if 1/I*dI/dX@1 $>$ " << im.varParD1NInfoNomCorThreshold()
    //       << " and dI/dX@0 $<$ 0" << std::endl;
    lStr << "\\begin{table}[H]" << std::endl;
    lStr << "\\scriptsize" << std::endl;
    lStr << "\\begin{center}" << std::endl;
    lStr << "\\renewcommand{\\arraystretch}{1.1}" << std::endl;
    lStr << "\\begin{tabular}{|c|c|"
         << ( minimized ? "c|" : "" )
         << ( minimized ? "ccc|" : "cc|" )
      // << ( InfoMinimizer::varyAllParameters() ? "}" : "c|}" )
         << "c|}"
         << std::endl;
    lStr << "\\hline" << std::endl;
    lStr << "Parameter name & ParID &"
         << ( minimized ? " Parameter value &" : "" )
         << "\\multicolumn{" << ( minimized ? "3" : "2" )
         << "}{|c|}{1/I$^\\mathrm{nom}$*dI/dX} "
      // << ( InfoMinimizer::varyAllParameters() ? "" : "&Fixed or" )
         << "& Fixed or"
         << "\\\\" << std::endl;
    lStr << " & &"
         << ( minimized ? " ScaleFactor X @MIN &" : "" )
         << " @0 & " << ( minimized ? "@MIN & " : "" ) << "@1 "
      // << ( InfoMinimizer::varyAllParameters() ? "" : "& Variable" )
         << "& Variable"
         << "\\\\" << std::endl;
    lStr << "\\hline" << std::endl;
    bool noParsWereVaried = true;
    for ( size_t iPar=0, iVary=0; iPar<nPar; ++iPar )
    {
      // Parameter name
      std::string parName = im.parName(iPar);
      parName.erase( std::remove( parName.begin(), parName.end(), '_' ), parName.end() );
      lStr << " {\\tiny " << parName << "} &";
      // Parameter id (5 char max)
      lStr << " \\#" << iPar << " &";
      // Parameter value at minimum
      if ( minimized )
      {
        TextReporterImpl::latexPrintDot4( lStr, im.parMin()[iPar], " $\\pm$" );
        if ( parToVary.find(iPar) == parToVary.end() ) lStr << " N/A"; // fixed
        else if ( im.dparMin()[iPar] == -2 ) lStr << " N/A"; // fixed - no path?
        else TextReporterImpl::latexPrintDot4( lStr, im.dparMin()[iPar] );
        lStr << " &";
      }
      // First derivatives
      TextReporterImpl::latexPrintDot4( lStr, d1NInfosZerCor[iPar], " &" );
      if ( minimized ) TextReporterImpl::latexPrintDot4( lStr, im.d1NInfosMinCor()[iPar], " &" );
      TextReporterImpl::latexPrintDot4( lStr, d1NInfosNomCor[iPar] );
      // Dump which parameters will be varied and which will be fixed and why
      {
        lStr << " &";
        // Dump which parameters have been fixed
        // Check which parameters should be varied
        if ( parToVary.find(iPar) == parToVary.end() )
        {
          lStr << " FIXED";
          if ( ! InfoMinimizer::varyAllParameters() )
          {
            if ( !minimized && d1NInfosZerCor[iPar] > 0 )
              lStr << " (d/dX@0$>$0: negative corr?)";
          }
        }
        else
        {
          noParsWereVaried = false;
          if ( !minimized ) lStr << " Variable \\#" << iVary++;
          else lStr << " Variable";
        }
      }
      lStr << " \\\\" << std::endl;
    }
    lStr << "\\hline" << std::endl;
    lStr << "\\end{tabular}" << std::endl;
    lStr << "\\renewcommand{\\arraystretch}{1}" << std::endl;
    lStr << "\\caption{" << msg << "Normalised Fisher information derivatives 1/I$^\\mathrm{nom}$*dI/dX ";
    if ( !minimized ) lStr << "(before minimization). ";
    else lStr << "(before and after minimization) and minimization results. ";
    lStr << " The derivatives in the table are computed with respect to scale factors X,"
         << " representing the ratio of a given correlation to the corresponding nominal correlation,"
         << " and all normalized by the information I$^\\mathrm{nom}$ at nominal correlations (''@1'').";
    lStr << " They are computed at three different values of the scale factors X:"
         << " for nominal values of all correlations (i.e. when all scale factors are 1: ''@1''),"
         << " for correlations all equal to zero (i.e. when all scale factors are 0: ''@0'')"
         << " and for the scale factors minimizing Fisher information (''@MIN'').";
    lStr << " In the minimization, the scale factors X were varied (between 0 and 1, ";
    if ( im.type() == InfoMinimizer::ByGlobalFactorMD ||
         im.type() == InfoMinimizer::ByErrorSourceMD ) lStr << "starting at 1)";
    else lStr << "starting at onionized covariances)";
    if ( ! InfoMinimizer::varyAllParameters() )
      lStr << " if 1/I$^\\mathrm{nom}$*dI/dX@1 $>$ " << im.varParD1NInfoNomCorThreshold() << " and dI/dX@0 $<$ 0.";
    else
      lStr << " unless dI/dX@0 == dI/dX@1 == 0.";
    if ( im.wasMinimumFound() )
      lStr << " A minimum was found in this minimization.";
    else if ( noParsWereVaried )
      lStr << " No minimization was attempted in this case"
           << " as all parameters were kept fixed.";
    else
      lStr << " {\\em\\color{Orange1}A minimum was NOT found in this minimization.}";
    lStr << "}" << std::endl;
    lStr << "\\end{center}" << std::endl;
    lStr << "\\end{table}" << std::endl;
  }
}

//-----------------------------------------------------------------------------

void
TextReporter::printPosCvwsBF1( const BlueFish1Obs& bf1PosCvws,
                               const std::vector< std::pair<BlueFish1Obs,size_t> >& bf1PosCvwsRemoved,
                               std::ostream& ostr,
                               bool latex )
{
  const size_t aObs1 = 0; // there is a single observable in these BFs
  // TEXT PRINTOUT
  if ( !latex )
  {
    std::ostream& tStr = ostr; // text stream
    tStr << "Text printout of positive CVW combination is not (yet?) supported" << std::endl;
  }
  // LATEX PRINTOUT
  else
  {
    std::ostream& lStr = ostr; // latex stream
    // Begin table
    lStr << "\\begin{table}[H]" << std::endl;
    lStr << "\\scriptsize" << std::endl;
    lStr << "\\begin{center}" << std::endl;
    lStr << "\\renewcommand{\\arraystretch}{1.2}" << std::endl;
    // Tabular
    lStr << "\\begin{tabular}{|c|c|c|c|c|";
    for ( size_t iErr=0; iErr<bf1PosCvws.nErr(); ++iErr ) lStr << "c";
    lStr << "|c|}" << std::endl;
    lStr << "\\hline" << std::endl;
    lStr << "N {\\tiny meas}";
    lStr << " & \\multicolumn{3}{c|}{Measurement removed in iteration}";
    lStr << " & \\multirow{2}{*}{BLUE}";
    for ( size_t iErr=0; iErr<bf1PosCvws.nErr(); ++iErr )
    {
      std::string errName = bf1PosCvws.errNames()[iErr];
      errName.erase( std::remove( errName.begin(), errName.end(), '_' ), errName.end() );
      lStr << " & \\multirow{2}{*}{\\tiny " << errName << "}";
    }
    lStr << " & \\multirow{2}{*}{\\tiny$\\chi^2$/ndof}\\\\" << std::endl;
    lStr << "\\cline{2-4}" << std::endl;
    lStr << "{\\tiny in BLUE} & Removed & CVW/\\% & MIW/\\%";
    for ( size_t iErr=0; iErr<bf1PosCvws.nErr(); ++iErr ) lStr << " &";
    lStr << " & & \\\\";
    lStr << "\\hline" << std::endl;
    // Add last measurement to the list to include it in the tabular
    std::vector< std::pair<BlueFish1Obs,size_t> > bf1s = bf1PosCvwsRemoved;
    bf1s.push_back( std::pair<BlueFish1Obs,size_t>( bf1PosCvws, bf1PosCvws.nMea() ) );
    unsigned dscf = getDownscaleFactor( bf1s.begin()->first ); // try to guess a scale factor for optimal printout
    // Display the full list in the tabular
    for ( std::vector< std::pair<BlueFish1Obs,size_t> >::const_iterator
            bf1i=bf1s.begin(); bf1i!=bf1s.end(); bf1i++ )
    {
      if ( bf1i->second == bf1i->first.nMea() ) lStr << "\\hline" << std::endl;
      lStr << bf1i->first.nMea() << " & ";
      if ( bf1i->second < bf1i->first.nMea() )
      {
        lStr << bf1i->first.meaNames()[bf1i->second] << " & ";
        TextReporterImpl::latexPrintDot2( lStr, 100*bf1i->first.blueCvw()(aObs1,bf1i->second), " & " );
        TextReporterImpl::latexPrintDot2( lStr, 100*bf1i->first.blueMiw()(aObs1,bf1i->second), " & " );
      }
      else
      {
        lStr << "{\\em NONE} & N/A & N/A & " << std::endl;
      }

      TextReporterImpl::latexPrintDot2( lStr, bf1i->first.blueVal()(aObs1)/dscf, " $\\pm$" );
      TextReporterImpl::latexPrintDot2( lStr, sqrt(bf1i->first.blueCov()(aObs1,aObs1))/dscf, " & " );
      for ( size_t iErr=0; iErr<bf1PosCvws.nErr(); ++iErr )
        TextReporterImpl::latexPrintDot2( lStr, sqrt(bf1i->first.blueCovSources()[iErr](aObs1,aObs1))/dscf, " & " );
      TextReporterImpl::latexPrintDot2( lStr, bf1i->first.blueCh2() );
      lStr << "/" << bf1i->first.blueCh2Ndof() << " \\\\";
      lStr << std::endl;
    }
    lStr << "\\hline" << std::endl;
    lStr << "\\end{tabular}" << std::endl;
    // Caption
    const BlueFish1Obs& bf1NomCor = bf1s.begin()->first;
    lStr << "\\caption{From the original combination of " << bf1NomCor.nMea() << " with nominal correlations,"
         << " a new combination where all remaining " << bf1PosCvws.nMea()
         << " measurements have central value weights CVW$>$0"
         << " was derived by removing measurements iteratively. At each step of the iteration,"
         << " the measurement with the most negative CVW$<=$0 in the combination with N measurements"
         << " was removed until all remainining measurements had CVW$>$0 in the combination of N-1 measurements.";
    if ( bf1PosCvwsRemoved.size() == 0 )
      lStr << " No measurements were removed in this case as no measurements with CVW$<=$0"
           << " were found in the original combination of " << bf1PosCvws.nMea() << " measurements.";
    lStr << " For each iteration and for the final result, the results of the BLUE"
         << " and the name, CVW and MIW of the measurement removed in that iteration"
         << " are displayed.}" << std::endl;
    // End table
    lStr << "\\renewcommand{\\arraystretch}{1}" << std::endl;
    lStr << "\\end{center}" << std::endl;
    lStr << "\\end{table}" << std::endl;
  }
}

//-----------------------------------------------------------------------------

void
TextReporter::printMeaCovs( const BlueFish& bf,
                            std::ostream& ostr,
                            bool latex,
                            bool partial,
                            const BlueFish* pBfNomCor )
{
  unsigned dscf = getDownscaleFactor( bf ); // try to guess a scale factor for optimal printout
  // TEXT PRINTOUT
  if ( !latex )
  {
    std::ostream& tStr = ostr; // text stream
    printMatrix( "CovM", bf.meaCov(), bf.meaNames(), dscf, tStr, latex );
    // NB: pBfNomCor is ignored for text printout!
  }
  // LATEX PRINTOUT
  else
  {
    const std::string colorMsg = ( pBfNomCor ?
                                   " Color boxes indicate covariances lower than nominal values by a"
                                   " factor up to 2 (green), up to 3 (cyan) or greater than 3 (blue)." : "" );
    std::ostream& lStr = ostr; // latex stream
    printMatrix( "Full input covariance between measurements (summed over error sources)." + colorMsg,
                 bf.meaCov(), bf.meaNames(), dscf, lStr, latex,
                 ( pBfNomCor ? &(pBfNomCor->meaCov()) : 0 ) );
    if ( partial )
    {
      for ( size_t iErr=0; iErr<bf.nErr(); ++iErr )
      {
        std::stringstream msg;
        msg << "Partial input covariance between measurements."
            << " Error source \\#" << iErr << ": " << bf.errNames()[iErr] << "." << colorMsg;
        printMatrix( msg.str(), bf.meaCovSources()[iErr], bf.meaNames(), dscf, lStr, latex,
                     ( pBfNomCor ? &(pBfNomCor->meaCovSources()[iErr]) : 0 ) );
      }
    }
  }
}

//-----------------------------------------------------------------------------

