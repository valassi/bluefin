//============================================================================
//
// Copyright 2012-2018 CERN
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
#include <fstream>
#include <iostream>
#include "BlueFin/BlueFish1Obs.h"
#include "BlueFin/InfoAnalyzer.h"
#include "BlueFin/InfoMinimizer.h"
#include "BlueFin/TextReporter.h"

// Namespace
using namespace bluefin;

//-----------------------------------------------------------------------------

namespace bluefin
{
  namespace InfoAnalyzerImpl
  {
    void
    printInfoAnalysisBF1( const BlueFish1Obs& bfNomCor, // nominal correlation
                          std::ostream& tStr, // text stream
                          const std::string& outputLatexFile,
                          const InfoAnalyzer::PrintoutOpts& opts )
    {
      errorStreamForPositiveDefiniteCheck() = &tStr;
      std::vector<std::pair<std::string,BlueFish1Obs> > bfs;
      bool latex = false;
      const bool partial = ( opts.cov == InfoAnalyzer::COV_TOTAL_AND_PARTIAL );
      // 1. NOMINAL CORRELATIONS
      const size_t aObs1 = 0; // there is a single observable in this BF
      const Number infoNomCor = bfNomCor.blueInf()(aObs1,aObs1);
      tStr << std::endl << "1. NOMINAL CORRELATIONS" << std::endl << std::endl;
      TextReporter::printBlueFish( bfNomCor, tStr );
      if ( opts.cov != InfoAnalyzer::COV_NONE )
        TextReporter::printMeaCovs( bfNomCor, tStr, latex, partial );
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
       bfs.push_back( std::make_pair<std::string,BlueFish1Obs>( TextReporter::nominalCorrelationTag(), bfNomCor ) );
#else
       bfs.push_back( std::make_pair( TextReporter::nominalCorrelationTag(), bfNomCor ) );
#endif
      // 2. MINIMIZE WRT GLOBAL FACTOR
      InfoMinimizer imByGF( bfNomCor, infoNomCor, InfoMinimizer::ByGlobalFactorMD );
      BlueFish1Obs bfMinByGF;
      if ( opts.min >= InfoAnalyzer::MIN_ADD_BYGF )
      {
        tStr << std::endl << "2. MINIMIZE WRT GLOBAL FACTOR" << std::endl << std::endl;
        bfMinByGF = imByGF.minimizeBF1( tStr );
        TextReporter::printBlueFish( bfMinByGF, tStr );
        if ( opts.cov != InfoAnalyzer::COV_NONE )
          TextReporter::printMeaCovs( bfMinByGF, tStr, latex, partial );
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
        bfs.push_back( std::make_pair<std::string,BlueFish1Obs>( "Minimize by global factor", bfMinByGF ) );
#else
        bfs.push_back( std::make_pair( "Minimize by global factor", bfMinByGF ) );
#endif
      }
      // 3. MINIMIZE WRT ERROR SOURCES
      InfoMinimizer imByES( bfNomCor, infoNomCor, InfoMinimizer::ByErrorSourceMD );
      BlueFish1Obs bfMinByES;
      if ( opts.min >= InfoAnalyzer::MIN_ADD_BYES )
      {
        tStr << std::endl << "3. MINIMIZE WRT ERROR SOURCES" << std::endl << std::endl;
        bfMinByES = imByES.minimizeBF1( tStr );
        TextReporter::printBlueFish( bfMinByES, tStr );
        if ( opts.cov != InfoAnalyzer::COV_NONE )
          TextReporter::printMeaCovs( bfMinByES, tStr, latex, partial );
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
        bfs.push_back( std::make_pair<std::string,BlueFish1Obs>( "Minimize by error sources", bfMinByES ) );
#else
        bfs.push_back( std::make_pair( "Minimize by error sources", bfMinByES ) );
#endif
      }
      // 4. MINIMIZE WRT OFF DIAGONAL ELEMENTS
      InfoMinimizer imByOD( bfNomCor, infoNomCor, InfoMinimizer::ByOffDiagElemMD );
      BlueFish1Obs bfMinByOD;
      if ( opts.min >= InfoAnalyzer::MIN_ADD_BYOD )
      {
        tStr << std::endl << "4. MINIMIZE WRT OFF DIAGONAL ELEMENTS" << std::endl << std::endl;
        try
        {
          bfMinByOD = imByOD.minimizeBF1( tStr );
          TextReporter::printBlueFish( bfMinByOD, tStr );
          if ( opts.cov != InfoAnalyzer::COV_NONE )
            TextReporter::printMeaCovs( bfMinByOD, tStr, latex, partial );
        }
        catch( BlueFish::CovarianceNotPosDef& )
        {
          tStr << "ERROR! Minimization by off-diagonal element failed (covariance not pos def)" << std::endl;
        }
        catch( std::exception& e )
        {
          tStr << "ERROR! Minimization by off-diagonal element failed (" << e.what() << ")" << std::endl;
        }
        catch( ... )
        {
          tStr << "ERROR! Minimization by off-diagonal element failed (unknown cause)" << std::endl;
        }
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
        bfs.push_back( std::make_pair<std::string,BlueFish1Obs>( "Minimize by off-diagonal elements", bfMinByOD ) );
#else
        bfs.push_back( std::make_pair( "Minimize by off-diagonal elements", bfMinByOD ) );
#endif
      }
      // 5. REMOVE MEASUREMENTS WITH NEGATIVE CVW'S
      tStr << std::endl << "5. REMOVE MEASUREMENTS WITH NEGATIVE CVW'S" << std::endl << std::endl;
      std::vector< std::pair<BlueFish1Obs,size_t> > bfPosCvwsRemoved;
      BlueFish1Obs bfPosCvws = bfNomCor.removeMeasWithNegativeCvwInBF1( bfPosCvwsRemoved );
      TextReporter::printBlueFish( bfPosCvws, tStr );
      if ( opts.cov != InfoAnalyzer::COV_NONE )
        TextReporter::printMeaCovs( bfPosCvws, tStr, latex, partial );
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
      bfs.push_back( std::make_pair<std::string,BlueFish1Obs>( "Remove negative CVWs", bfPosCvws ) );
#else
      bfs.push_back( std::make_pair( "Remove negative CVWs", bfPosCvws ) );
#endif
      // 6. MINIMIZE WRT OFF DIAGONAL ELEMENTS PER ERROR SOURCE
      InfoMinimizer imByOE( bfNomCor, infoNomCor, InfoMinimizer::ByOffDiagPerErrSrcMD );
      BlueFish1Obs bfMinByOE;
      if ( opts.min >= InfoAnalyzer::MIN_ADD_BYOE )
      {
        tStr << std::endl << "6. MINIMIZE WRT OFF DIAGONAL ELEMENTS PER ERROR SOURCE" << std::endl << std::endl;
        try
        {
          bfMinByOE = imByOE.minimizeBF1( tStr );
          TextReporter::printBlueFish( bfMinByOE, tStr );
          if ( opts.cov != InfoAnalyzer::COV_NONE )
            TextReporter::printMeaCovs( bfMinByOE, tStr, latex, partial );
        }
        catch( BlueFish::CovarianceNotPosDef& )
        {
          tStr << "ERROR! Minimization by off-diagonal per error src failed (covariance not pos def)" << std::endl;
        }
        catch( std::exception& e )
        {
          tStr << "ERROR! Minimization by off-diagonal per error src failed (" << e.what() << ")" << std::endl;
        }
        catch( ... )
        {
          tStr << "ERROR! Minimization by off-diagonal per error src failed (unknown cause)" << std::endl;
        }
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
        bfs.push_back( std::make_pair<std::string,BlueFish1Obs>( "Minimize by off-diagonal per error src", bfMinByOE ) );
#else
        bfs.push_back( std::make_pair( "Minimize by off-diagonal per error src", bfMinByOE ) );
#endif
      }
      // 7a. ONIONIZE (RC)
      //tStr << std::endl << "7a. ONIONIZE (RC)" << std::endl << std::endl;
      //BlueFish1Obs bfOnionRC = bfNomCor.onionizeCorrsByErrorSourceBF1( true, &tStr ); // RC onionization
      //TextReporter::printBlueFish( bfOnionRC, tStr );
      //if ( opts.cov != InfoAnalyzer::COV_NONE )
      //  TextReporter::printMeaCovs( bfOnionRC, tStr, latex, partial );
      //bfs.push_back( std::make_pair<std::string,BlueFish1Obs>( "Onionize (1--RC)", bfOnionRC ) );
      // 7b. ONIONIZE (AV)
      //tStr << std::endl << "7b. ONIONIZE (AV)" << std::endl << std::endl;
      tStr << std::endl << "7. ONIONIZE" << std::endl << std::endl;
      BlueFish1Obs bfOnionAV = bfNomCor.onionizeCorrsByErrorSourceBF1( false, &tStr ); // AV onionization
      TextReporter::printBlueFish( bfOnionAV, tStr );
      if ( opts.cov != InfoAnalyzer::COV_NONE )
        TextReporter::printMeaCovs( bfOnionAV, tStr, latex, partial );
      //bfs.push_back( std::make_pair<std::string,BlueFish1Obs>( "Onionize (2--AV)", bfOnionAV ) );
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
      bfs.push_back( std::make_pair<std::string,BlueFish1Obs>( "Onionize", bfOnionAV ) );
#else
      bfs.push_back( std::make_pair( "Onionize", bfOnionAV ) );
#endif
      // 8. ZERO CORRELATIONS
      const BlueFish1Obs bfZerCor = bfNomCor.keepCorrsForSource(-1); // zero correlations
      tStr << std::endl << "8. NO CORRELATIONS" << std::endl << std::endl;
      TextReporter::printBlueFish( bfZerCor, tStr );
      if ( opts.cov != InfoAnalyzer::COV_NONE )
        TextReporter::printMeaCovs( bfZerCor, tStr, latex, partial );
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
      bfs.push_back( std::make_pair<std::string,BlueFish1Obs>( TextReporter::zeroCorrelationTag(), bfZerCor ) );
#else
      bfs.push_back( std::make_pair( TextReporter::zeroCorrelationTag(), bfZerCor ) );
#endif
      errorStreamForPositiveDefiniteCheck() = 0;
      // ==========================================================================
      // === CREATE LATEX REPORT
      // ==========================================================================
      tStr << std::endl;
      tStr << "--------------------------------------------------------------------" << std::endl;
      tStr << std::endl;
      if ( outputLatexFile == "" ) return;
      latex = true;
      tStr << "Create latex report: " << outputLatexFile << std::endl;
      std::ofstream lStr; // latex stream
      lStr.open ( outputLatexFile.c_str() );
      // == 1. NOMINAL CORRELATIONS
      lStr << "\\section{Nominal correlations.}" << std::endl;
      TextReporter::printBlueFish( bfNomCor, lStr, latex );
      TextReporter::printNInfoDerivativesBF1( bfNomCor, lStr, latex, "nominal correlations" );
      if ( opts.cov != InfoAnalyzer::COV_NONE )
        TextReporter::printMeaCovs( bfNomCor, lStr, latex, partial, 0 ); // no need to compare
      std::string newPage = "\\clearpage"; // better than newpage for floats
      lStr << newPage << std::endl;
      // == 2. MODIFIED CORRELATIONS
      lStr << "\\section{Modified correlations.}" << std::endl;
      // == 2.1 SUMMARY TABLE
      lStr << "\\subsection{Summary of results.}" << std::endl;
      TextReporter::printSummaryTableBF1( bfNomCor, bfs, lStr, latex );
      lStr << newPage << std::endl;
      // == 2.2 MINIMIZE BY GF
      const bool preOnly = false;
      if ( opts.min >= InfoAnalyzer::MIN_ADD_BYGF )
      {
        lStr << "\\subsection{Minimize correlations by a global rescaling factor.}" << std::endl;
        TextReporter::printBlueFish( bfMinByGF, lStr, latex );
        TextReporter::printMinimization( imByGF, preOnly, lStr, latex );
        TextReporter::printNInfoDerivativesBF1( bfMinByGF, lStr, latex,
                                                "correlations in minimization by global factor" );
        if ( opts.cov != InfoAnalyzer::COV_NONE )
          TextReporter::printMeaCovs( bfMinByGF, lStr, latex, partial, &bfNomCor );
        lStr << newPage << std::endl;
      }
      // == 2.3 MINIMIZE BY ES
      if ( opts.min >= InfoAnalyzer::MIN_ADD_BYES )
      {
        lStr << "\\subsection{Minimize correlations by one factor per error source.}" << std::endl;
        TextReporter::printBlueFish( bfMinByES, lStr, latex );
        TextReporter::printMinimization( imByES, preOnly, lStr, latex );
        TextReporter::printNInfoDerivativesBF1( bfMinByES, lStr, latex,
                                                "correlations in minimization by error source" );
        if ( opts.cov != InfoAnalyzer::COV_NONE )
          TextReporter::printMeaCovs( bfMinByES, lStr, latex, partial, &bfNomCor );
        lStr << newPage << std::endl;
      }
      // == 2.4 MINIMIZE BY OD
      if ( opts.min >= InfoAnalyzer::MIN_ADD_BYOD )
      {
        lStr << "\\subsection{Minimize correlations by one factor per off-diagonal element.}" << std::endl;
        if ( bfMinByOD.nObs() == 0 )
        {
          lStr << "Not Available (minimization failed)." << std::endl;
        }
        else
        {
          TextReporter::printBlueFish( bfMinByOD, lStr, latex );
          TextReporter::printMinimization( imByOD, preOnly, lStr, latex );
          TextReporter::printNInfoDerivativesBF1( bfMinByOD, lStr, latex,
                                                  "correlations in minimization by off-diagonal elements" );
          if ( opts.cov != InfoAnalyzer::COV_NONE )
            TextReporter::printMeaCovs( bfMinByOD, lStr, latex, partial, &bfNomCor );
        }
        lStr << newPage << std::endl;
      }
      // == 2.5 ONLY POSITIVE CVWS
      lStr << "\\subsection{Remove measurements with negative central value weights.}" << std::endl;
      TextReporter::printBlueFish( bfPosCvws, lStr, latex );
      TextReporter::printPosCvwsBF1( bfPosCvws, bfPosCvwsRemoved, lStr, latex );
      TextReporter::printNInfoDerivativesBF1( bfPosCvws, lStr, latex,
                                              "correlations in combination with CVW$>$0 measurements" );
      if ( opts.cov != InfoAnalyzer::COV_NONE )
        TextReporter::printMeaCovs( bfPosCvws, lStr, latex, partial, 0 ); // cannot compare (different nMea!)
      lStr << newPage << std::endl;
      // == 2.6 MINIMIZE BY OD PER ERRSRC
      if ( opts.min >= InfoAnalyzer::MIN_ADD_BYOE )
      {
        lStr << "\\subsection{Minimize correlations by one factor per off-diagonal element per error source.}" << std::endl;
        if ( bfMinByOE.nObs() == 0 )
        {
          lStr << "Not Available (minimization failed)." << std::endl;
        }
        else
        {
          TextReporter::printBlueFish( bfMinByOE, lStr, latex );
          for ( std::vector<InfoMinimizer*>::const_iterator pim=imByOE.subMinimizers().begin();
                pim!=imByOE.subMinimizers().end(); ++pim )
            TextReporter::printMinimization( *(*pim), preOnly, lStr, latex, (*pim)->bf().combName()+". " );
          TextReporter::printNInfoDerivativesBF1( bfMinByOE, lStr, latex,
                                                  "correlations in minimization by off-diagonal elements per error source" );
          if ( opts.cov != InfoAnalyzer::COV_NONE )
            TextReporter::printMeaCovs( bfMinByOE, lStr, latex, partial, &bfNomCor );
        }
        lStr << newPage << std::endl;
      }
      // == 2.7a ONIONIZE RC
      //lStr << "\\subsection{Onionize correlations (first recipe -- RC).}" << std::endl;
      //TextReporter::printBlueFish( bfOnionRC, lStr, latex );
      //TextReporter::printNInfoDerivativesBF1( bfOnionRC, lStr, latex,
      //                                        "correlations in onionization 2nd recipe" );
      //if ( opts.cov != InfoAnalyzer::COV_NONE )
      //  TextReporter::printMeaCovs( bfOnionRC, lStr, latex, partial, &bfNomCor );
      //lStr << newPage << std::endl;
      // == 2.7b ONIONIZE AV
      lStr << "\\subsection{Onionize correlations.}" << std::endl;
      //lStr << "\\subsection{Onionize correlations (second recipe -- AV).}" << std::endl;
      TextReporter::printBlueFish( bfOnionAV, lStr, latex );
      TextReporter::printNInfoDerivativesBF1( bfOnionAV, lStr, latex,
                                              "correlations in onionization 1st recipe" );
      if ( opts.cov != InfoAnalyzer::COV_NONE )
        TextReporter::printMeaCovs( bfOnionAV, lStr, latex, partial, &bfNomCor );
      lStr << newPage << std::endl;
      // == 2.8 ZERO CORRELATIONS
      lStr << "\\subsection{Zero correlations.}" << std::endl;
      TextReporter::printBlueFish( bfZerCor, lStr, latex );
      TextReporter::printNInfoDerivativesBF1( bfZerCor, lStr, latex,
                                              "zero correlations" );
      if ( opts.cov != InfoAnalyzer::COV_NONE )
        TextReporter::printMeaCovs( bfZerCor, lStr, latex, partial, &bfNomCor );
      lStr.close();
    }
  }
}

//-----------------------------------------------------------------------------

void
InfoAnalyzer::printInfoAnalysis( const BlueFish& bfNomCor, // nominal correlation
                                 std::ostream& tStr, // text stream
                                 const std::string& outputLatexFile,
                                 const PrintoutOpts& opts )
{
  tStr << std::endl << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
  if ( bfNomCor.hasNegativeMeaCorr() )
    tStr << std::endl << "WARNING! Some nominal correlations are negative! Skip several analyses..." << std::endl;
  // More complete analysis for single-observable combinations
  if ( bfNomCor.nObs() == 1 && !bfNomCor.hasNegativeMeaCorr() )
  {
    // Hack to allow compilation even if this bf is not a bf1; and another
    // hack to avoid recomputing BF1 derivatives if bf already IS a bf1...
    const BlueFish1Obs bf1NomCorCopy = ( bfNomCor.nObs() == 1 ? BlueFish1Obs( bfNomCor ) : BlueFish1Obs() );
    const BlueFish1Obs& bf1NomCor = ( dynamic_cast<const BlueFish1Obs*>( &bfNomCor ) != 0?
                                      dynamic_cast<const BlueFish1Obs&>( bfNomCor ) : bf1NomCorCopy );
    InfoAnalyzerImpl::printInfoAnalysisBF1( bf1NomCor, tStr, outputLatexFile, opts );
  }
  // Simpler analysis for single-observable combinations and negative correlations
  else
  {
    std::vector<std::pair<std::string,BlueFish> > bfs;
    bool latex = false;
    const bool partial = ( opts.cov == InfoAnalyzer::COV_TOTAL_AND_PARTIAL );
    // 1. NOMINAL CORRELATIONS
    tStr << std::endl << "0. NOMINAL CORRELATIONS" << std::endl << std::endl;
    TextReporter::printBlueFish( bfNomCor, tStr );
    if ( opts.cov != InfoAnalyzer::COV_NONE )
      TextReporter::printMeaCovs( bfNomCor, tStr, latex, partial );
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
    bfs.push_back( std::make_pair<std::string,BlueFish>( TextReporter::nominalCorrelationTag(), bfNomCor ) );
#else
    bfs.push_back( std::make_pair( TextReporter::nominalCorrelationTag(), bfNomCor ) );
#endif
    // 7. ZERO CORRELATIONS
    const BlueFish bfZerCor = bfNomCor.keepCorrsForSource(-1); // zero correlations
    tStr << std::endl << "7. NO CORRELATIONS" << std::endl << std::endl;
    TextReporter::printBlueFish( bfZerCor, tStr );
    if ( opts.cov != InfoAnalyzer::COV_NONE )
      TextReporter::printMeaCovs( bfZerCor, tStr, latex, partial );
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
    bfs.push_back( std::make_pair<std::string,BlueFish>( TextReporter::zeroCorrelationTag(), bfZerCor ) );
#else
    bfs.push_back( std::make_pair( TextReporter::zeroCorrelationTag(), bfZerCor ) );
#endif
    // ==========================================================================
    // === CREATE LATEX REPORT
    // ==========================================================================
    tStr << std::endl;
    tStr << "--------------------------------------------------------------------" << std::endl;
    tStr << std::endl;
    if ( outputLatexFile == "" ) return;
    latex = true;
    tStr << "Create latex report: " << outputLatexFile << std::endl;
    std::ofstream lStr; // latex stream
    lStr.open ( outputLatexFile.c_str() );
    // == 1. NOMINAL CORRELATIONS
    lStr << "\\section{Nominal correlations.}" << std::endl;
    //lStr << "\\subsection{Nominal correlations.}" << std::endl;
    TextReporter::printBlueFish( bfNomCor, lStr, latex );
    if ( opts.cov != InfoAnalyzer::COV_NONE )
      TextReporter::printMeaCovs( bfNomCor, lStr, latex, partial );
    std::string newPage = "\\clearpage"; // better than newpage for floats
    lStr << newPage << std::endl;
    // == 2. MODIFIED CORRELATIONS
    lStr << "\\section{Modified correlations.}" << std::endl;
    // == 2.8 ZERO CORRELATIONS
    lStr << "\\subsection{Zero correlations.}" << std::endl;
    TextReporter::printBlueFish( bfZerCor, lStr, latex );
    if ( opts.cov != InfoAnalyzer::COV_NONE )
      TextReporter::printMeaCovs( bfZerCor, lStr, latex, partial );
    lStr.close();
  }
}

//-----------------------------------------------------------------------------

