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
#include <fstream>
#include <iostream>
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include "boost_bind_headers.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "Math/Minimizer.h"
#include "BlueFin/BlueFish1Obs.h"
#include "BlueFin/InfoMinimizer.h"
#include "BlueFin/TextReporter.h"

// Static data and functions for ROOT minimization
namespace bluefin
{
  namespace InfoMinimizerImpl
  {
    // The InfoMinimizer instance to be used for ROOT minimizations.
    // Global (static) parameter in the function to minimize.
    // This is set by the same instance and can be used only by one instance at a time.
    static const InfoMinimizer* s_imForROOT = 0;

    // The normalized information function to minimize.
    // [NB: ROOT does not support 'long double', it expects 'double']
    // [NB: this is the ONLY method that needs 'double'!]]
    static double normInfoTM( const double* ddTV ); // size: ALL errors

    // Message stream for the normalized information function to minimize.
    static std::ostream* s_streamForNormInfoTM = 0;

    // The maximum information seen so far in the minimization.
    // This is used when trying to regularize non-pos-definite covariances.
    static Number s_infoMax = -1;
  }
}

// Static functions for ByOffDiagPerErrSrcMD minimization
namespace bluefin
{
  namespace InfoMinimizerImpl
  {
    // Make a symmetric matrix positive definite by reducing correlations
    static Matrix makePositiveDefinite( const Matrix& );

    // Split a BLUE into a vector of BLUEs with one error source each,
    // keeping only measurements with positive variances and
    // reducing correlations till the covariance is positive definite
    static std::vector<BlueFish1Obs> splitBF1PerErrSrc( const BlueFish1Obs& bf1 );

    // Build a BLUE from a reference BLUE and a vector of BLUEs
    // with one error source each and possibly fewer measurements
    static BlueFish1Obs mergeBF1PerErrSrc( const BlueFish1Obs& bf1,
                                           const std::vector<BlueFish1Obs>& bf1s );

    // Minimize by off diag per error src
    static BlueFish1Obs minimizeBF1ByOffDiagPerErrSrcMD( const InfoMinimizer* im,
                                                         std::ostream& tStr,
                                                         std::vector<InfoMinimizer*>& subMinimizers );
  }
}

// Namespace
using namespace bluefin;

//-----------------------------------------------------------------------------

InfoMinimizer::~InfoMinimizer()
{
  for ( std::vector<InfoMinimizer*>::const_iterator pim=m_subMinimizers.begin(); pim!=m_subMinimizers.end(); ++pim )
    delete *pim;
}

//-----------------------------------------------------------------------------

InfoMinimizer::InfoMinimizer( const BlueFish1Obs& bf,
                              Number infoNorm,
                              const FcnType type )
  : m_bf( bf )
  , m_infoNorm( infoNorm )
  , m_type( type )
  , m_d1NInfosNomCor()
  , m_d1NInfosZerCor()
  , m_parToVaryExists( false )
  , m_parToVary()
  , m_minimizationFailed( false )
  , m_wasMinimumFound( false )
  , m_parMin()
  , m_dparMin()
  , m_d1NInfosMinCor()
  , m_minBf()
  , m_subMinimizers()
{
  // Check the BlueFish1Obs
  if ( m_bf.nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in IM ctor" );
  // Check the normalization factor.
  if ( m_infoNorm <= 0 )
    throw std::runtime_error( "infoNorm <= 0 in IM ctor" );
  // Check the functional dependency
  if ( m_type != ByGlobalFactorMD &&
       m_type != ByErrorSourceMD &&
       m_type != ByOffDiagElemMD &&
       m_type != ByOffDiagPerErrSrcMD )
    throw std::runtime_error( "unsupported type in IM ctor" );
}

//-----------------------------------------------------------------------------

bool InfoMinimizer::varyAllParameters()
{
  //return false; // BLUEFIN 01.00.00 behaviour (arxiv v1)
  return true; // BLUEFIN 01.00.01 behaviour (arxiv v2)
}

//-----------------------------------------------------------------------------

double InfoMinimizerImpl::normInfoTM( const double* ddNew )
{
  if ( InfoMinimizerImpl::s_imForROOT == 0 )
    throw std::runtime_error( "Static InfoMinimizer is not initialized" );
  const BlueFish1Obs& bf = InfoMinimizerImpl::s_imForROOT->bf();
  const Number infoNorm = InfoMinimizerImpl::s_imForROOT->infoNorm();
  const InfoMinimizer::FcnType type = InfoMinimizerImpl::s_imForROOT->type();
  const size_t nPar = InfoMinimizer::nPar( bf, type );
  // Check parameter boundaries (generic to all types)
  Number info = 0;
  Number bound = 1;
  std::vector<Number> sfs; // size: nPar
  for ( size_t iPar=0; iPar<nPar; ++iPar )
  {
    sfs.push_back( ddNew[iPar] );
    if ( sfs[iPar] < -2*BlueFish1Obs::Delta ) // relax to -2*delta just in case
    {
      if ( InfoMinimizerImpl::s_streamForNormInfoTM )
        *InfoMinimizerImpl::s_streamForNormInfoTM << "WARNING! x_" << iPar << "<0: " << sfs[iPar] << std::endl;
      bound *= ( 1 + 10000*(-sfs[iPar]) ); // contain within boundary
      sfs[iPar] = 0;
    }
    else if ( sfs[iPar] > 1 )
    {
      if ( InfoMinimizerImpl::s_streamForNormInfoTM )
        *InfoMinimizerImpl::s_streamForNormInfoTM << "WARNING! x_" << iPar << ">1: " << sfs[iPar] << std::endl;
      bound *= ( 1 + 10000*(sfs[iPar]-1) ); // contain within boundary
      sfs[iPar] = 1;
    }
  }
  // Try to tame non-positive-definite covariances during minimization
  try
  {
    if ( InfoMinimizerImpl::s_infoMax < 0 ) InfoMinimizerImpl::s_infoMax = infoNorm;
    // Scale factor on each error source
    if ( type == InfoMinimizer::ByErrorSourceMD )
    {
      info = bf.infoRescaledCorrsByErrorSourceBF1( sfs );
    }
    // Scale factor on off diagonal elements
    else if ( type == InfoMinimizer::ByOffDiagElemMD )
    {
      const size_t aObs1 = 0; // there is a single observable in this BF
      info = bf.rescaleCorrsByOffDiagElemBF1( sfs ).blueInf()(aObs1,aObs1);
    }
    // Global scale factor
    else if ( type == InfoMinimizer::ByGlobalFactorMD )
    {
      const size_t aObs1 = 0; // there is a single observable in this BF
      info = bf.rescaleCorrsByGlobalFactorBF1( sfs[0] ).blueInf()(aObs1,aObs1);
    }
    // Unknown functional dependency
    else
    {
      throw std::runtime_error( "unsupported type in normInfoTM" );
    }
    InfoMinimizerImpl::s_infoMax = std::max( InfoMinimizerImpl::s_infoMax, info );
  }
  catch( BlueFish::CovarianceNotPosDef& )
  {
    //throw; // BLUEFIN 01.00.00 behaviour (arxiv v1)
    InfoMinimizerImpl::s_infoMax *= 10;
    info = InfoMinimizerImpl::s_infoMax; // BLUEFIN 01.00.02 behaviour (arxiv v2)
    // NB: this is useful in partial covariance minimization (e.g. bJES only)
    // but it only works if starting value is far from the lower/upper bound!
  }
  // Optional dump  to std::cout
  //if ( InfoMinimizerImpl::s_streamForNormInfoTM )
  //{
  //  char out[80];
  //  snprintf( out, 12, "%11.8f", (double)(bound*info/infoNorm) );
  //  *InfoMinimizerImpl::s_streamForNormInfoTM << "-----> f=" << out << " at";
  //  for ( size_t iPar=0; iPar<nPar; ++iPar )
  //  {
  //    snprintf( out, 6, "%.5f", (double)sfs[iPar] );
  //    *InfoMinimizerImpl::s_streamForNormInfoTM << (iPar==0?" ":", ") << out;
  //  }
  //  *InfoMinimizerImpl::s_streamForNormInfoTM << std::endl;
  //}
  // Normalize, optionally dump, and return
  return bound*info/infoNorm; // Normalize by the given factor
}

//-----------------------------------------------------------------------------

Matrix InfoMinimizerImpl::makePositiveDefinite( const Matrix& mOld )
{
  const Number scf = 0.999;
  const size_t nLoop = 50;
  size_t iLoop = 0;
  Matrix mNew = mOld;
  while( !isPositiveDefinite( mNew ) )
  {
    if ( iLoop++ == nLoop )
    {
      std::stringstream msg;
      msg << "Could not make matrix positive definite after " << nLoop << " attempts (scf=" << scf << ")";
      throw std::runtime_error( msg.str() );
    }
    for ( size_t i=0; i<mNew.size1(); i++ )
    {
      if ( mNew(i,i) <= 0 )
        throw std::runtime_error( "diagonal term <=0 in makePositiveDefinite" );
      for ( size_t j=0; j<i; j++ )
      {
        if ( mNew(j,i) != mNew(i,j) )
          throw std::runtime_error( "non-symmetric matrix in makePositiveDefinite" );
        mNew(i,j) *= scf;
        mNew(j,i) = mNew(i,j);
      }
    }
  }
  return mNew;
}

//-----------------------------------------------------------------------------

std::vector<BlueFish1Obs>
InfoMinimizerImpl::splitBF1PerErrSrc( const BlueFish1Obs& bf1 )
{
  std::vector<BlueFish1Obs> bf1s;
  for ( size_t iErr=0; iErr<bf1.nErr(); ++iErr )
  {
    const std::string& errName = bf1.errName(iErr);
    const SymmetricMatrix& covAll = bf1.meaCovSources()[iErr];
    // Select only the measurements with positive variance
    std::set<size_t> errMeaSetPosVar;
    for ( size_t iMea=0; iMea<bf1.nMea(); ++iMea )
    {
      if ( covAll(iMea,iMea) < 0 )
        throw std::runtime_error( "Variance<0 in error source " + errName + " in splitBF1PerErrSrc" );
      else if ( covAll(iMea,iMea) > 0 )
        errMeaSetPosVar.insert( iMea );
    }
    // Build the reduced vectors and matrices with fewer measurements
    if ( errMeaSetPosVar.size() > 0 ) // some error sources are all 0!
    {
      std::vector<std::string> meaNames;
      Vector meaVal = ZeroVector( errMeaSetPosVar.size() );
      SymmetricMatrix covRed = ZeroMatrix( errMeaSetPosVar.size() );
      for ( size_t iMea=0, iMea2=0; iMea<bf1.nMea(); ++iMea )
        if ( errMeaSetPosVar.find(iMea) != errMeaSetPosVar.end() )
        {
          covRed(iMea2,iMea2) = covAll(iMea,iMea);
          for ( size_t jMea=0, jMea2=0; jMea<iMea; ++jMea )
            if ( errMeaSetPosVar.find(jMea) != errMeaSetPosVar.end() )
            {
              covRed(iMea2,jMea2) = covAll(iMea,jMea);
              covRed(jMea2,iMea2) = covAll(jMea,iMea);
              ++jMea2;
            }
          meaNames.push_back( bf1.meaName(iMea) );
          meaVal(iMea2) = bf1.meaVal()(iMea);
          ++iMea2;
        }
      covRed = makePositiveDefinite( covRed ); // reduce correlations to make cov posdef
      bf1s.push_back( BlueFish1Obs( "Minimization by off-diagonal elements in error source " + errName,
                                    bf1.obsNames()[0], meaNames, meaVal,
                                    std::vector<std::string>( 1, errName ),
                                    std::vector<SymmetricMatrix>( 1, covRed ) ) );
    }
  }
  return bf1s;
}

//-----------------------------------------------------------------------------

BlueFish1Obs
InfoMinimizerImpl::mergeBF1PerErrSrc( const BlueFish1Obs& bf1,
                                      const std::vector<BlueFish1Obs>& bf1s )
{
  std::vector<SymmetricMatrix> errMeaCovs;
  if ( bf1.nErr() < bf1s.size() )
    throw std::runtime_error( "nErr mismatch in mergeBF1PerErrSrc" );
  size_t iErr2=0;
  for ( size_t iErr=0; iErr<bf1.nErr(); ++iErr )
  {
    SymmetricMatrix covAll = bf1.meaCovSources()[iErr];
    const std::string& errName = bf1.errName(iErr);
    // Select only the measurements with positive variance
    std::set<size_t> errMeaSetPosVar;
    for ( size_t iMea=0; iMea<bf1.nMea(); ++iMea )
    {
      if ( covAll(iMea,iMea) < 0 )
        throw std::runtime_error( "Variance<0 in error source " + errName + " in mergeBF1PerErrSrc" );
      else if ( covAll(iMea,iMea) > 0 )
        errMeaSetPosVar.insert( iMea );
    }
    // Expect a match for this error source unless all errors are 0
    if ( errMeaSetPosVar.size() > 0 )
    {
      if ( iErr2 >= bf1s.size() )
        throw std::runtime_error( "too few error sources in mergeBF1PerErrSrc" );
      const BlueFish1Obs& bf1Red = bf1s[iErr2];
      if ( bf1Red.nErr() != 1 )
        throw std::runtime_error( "nErr != 1 in mergeBF1PerErrSrc" );
      if ( bf1Red.errName(0) != errName )
        throw std::runtime_error( "errName mismatch in mergeBF1PerErrSrc" );
      if ( bf1Red.nMea() != errMeaSetPosVar.size() )
        throw std::runtime_error( "nMea mismatch in mergeBF1PerErrSrc" );
      // Modify the original covariances
      const SymmetricMatrix& covRed = bf1Red.meaCovSources()[0];
      for ( size_t iMea=0, iMea2=0; iMea<bf1.nMea(); ++iMea )
        if ( errMeaSetPosVar.find(iMea) != errMeaSetPosVar.end() )
        {
          if ( bf1Red.meaName(iMea2) != bf1.meaName(iMea) )
            throw std::runtime_error( "meaName mismatch in mergeBF1PerErrSrc" );
          if ( covRed(iMea2,iMea2) != covAll(iMea,iMea) )
            throw std::runtime_error( "variance mismatch in mergeBF1PerErrSrc" );
          for ( size_t jMea=0, jMea2=0; jMea<iMea; ++jMea )
            if ( errMeaSetPosVar.find(jMea) != errMeaSetPosVar.end() )
            {
              covAll(iMea,jMea) = covRed(iMea2,jMea2);
              covAll(jMea,iMea) = covRed(jMea2,iMea2);
              ++jMea2;
            }
          ++iMea2;
        }
      ++iErr2;
    }
    errMeaCovs.push_back( covAll );
  }
  if ( iErr2 != bf1s.size() )
    throw std::runtime_error( "unused error sources in mergeBF1PerErrSrc" );
  return BlueFish1Obs( bf1.combName(), bf1.obsNames()[0], bf1.meaNames(), bf1.meaVal(),
                       bf1.errNames(), errMeaCovs );
}

//-----------------------------------------------------------------------------

BlueFish1Obs
InfoMinimizerImpl::minimizeBF1ByOffDiagPerErrSrcMD( const InfoMinimizer* im,
                                                    std::ostream& tStr,
                                                    std::vector<InfoMinimizer*>& subMinimizers )
{
  const BlueFish1Obs& bfNomCor = im->bf();
  std::vector<BlueFish1Obs> bf1s = InfoMinimizerImpl::splitBF1PerErrSrc( bfNomCor );
  for ( size_t iErr=0; iErr<bf1s.size(); ++iErr )
  {
    // Minimize by off diagonal element in each error source
    const size_t aObs1 = 0; // there is a single observable in this BF
    subMinimizers.push_back( new InfoMinimizer( bf1s[iErr], bf1s[iErr].blueInf()(aObs1,aObs1), InfoMinimizer::ByOffDiagElemMD ) );
    InfoMinimizer& imByOD = *(subMinimizers[subMinimizers.size()-1]);
    BlueFish1Obs bfMinByOD;
    try
    {
      bfMinByOD = imByOD.minimizeBF1( tStr );
      TextReporter::printBlueFish( bfMinByOD, tStr );
      //if ( opts.cov != InfoAnalyzer::COV_NONE )
      //  TextReporter::printMeaCovs( bfMinByOD, tStr, latex, partial );
      bf1s[iErr] = bfMinByOD;
    }
    catch( BlueFish::CovarianceNotPosDef& )
    {
      tStr << "ERROR! Minimization ByOffDiagPerErrSrc failed for "
           << bf1s[iErr].errName(0) << " (covariance not pos def)" << std::endl;
      throw;
    }
    catch( std::exception& e )
    {
      tStr << "ERROR! Minimization ByOffDiagPerErrSrc failed for "
           << bf1s[iErr].errName(0) << " (" << e.what() << ")" << std::endl;
      throw;
    }
    catch( ... )
    {
      tStr << "ERROR! Minimization ByOffDiagPerErrSrc failed for "
           << bf1s[iErr].errName(0) << " (unknown cause)" << std::endl;
      throw;
    }
  }
  return InfoMinimizerImpl::mergeBF1PerErrSrc( bfNomCor, bf1s );
}

//-----------------------------------------------------------------------------

void InfoMinimizer::normInfoAndFirstDerivatives( const std::vector<Number>& ddNew, // size: ALL errors
                                                 Number& nInfoNew,
                                                 std::vector<Number>& d1NInfoNew ) const // size: ALL errors
{
  if ( this->infoNorm() == 0 )
    throw std::runtime_error( "InfoMinimizer is not initialized in normInfoAndFirstDerivatives" );
  d1NInfoNew.clear();
  size_t nPar = this->nPar();
  // Scale factor on each error source (compute derivatives using matrices)
  if ( this->type() == ByErrorSourceMD )
  {
    const BlueFish1Obs& bfRef = this->bf();
    BlueFish1Obs bfNew = bfRef.rescaleCorrsByErrorSourceBF1( ddNew );
    if ( bfNew.nObs() != 1 )
      throw std::runtime_error( "nObs != 1 in normInfoAndFirstDerivatives" );
    const size_t aObs1 = 0; // there is a single observable in this BF
    nInfoNew = bfNew.blueInf()(aObs1,aObs1)/this->infoNorm(); // Normalize by the given factor
    // Use matrix derivatives
    const std::vector<Number>& d1Infos = bfNew.blueD1NormInfByErrorSourceBF1( bfRef );
    if ( d1Infos.size() != nPar )
      throw std::runtime_error( "d1Infos wrong size" );
    for ( size_t iPar=0; iPar<nPar; ++iPar )
    {
      d1NInfoNew.push_back( d1Infos[iPar] ); // already normalized
    }
  }
  // Scale factor on off diagonal elements
  else if ( this->type() == ByOffDiagElemMD )
  {
    const BlueFish1Obs& bfRef = this->bf();
    BlueFish1Obs bfNew( bfRef.rescaleCorrsByOffDiagElemBF1( ddNew ) );
    if ( bfNew.nObs() != 1 )
      throw std::runtime_error( "nObs != 1 in normInfoAndFirstDerivatives" );
    const size_t aObs1 = 0; // there is a single observable in this BF
    nInfoNew = bfNew.blueInf()(aObs1,aObs1)/this->infoNorm(); // Normalize by the given factor
    // Use matrix derivatives
    const TriangularMatrix& d1Infos = bfNew.blueD1NormInfByOffDiagElemBF1( bfRef );
    for ( size_t iPar=0; iPar<nPar; ++iPar )
    {
      std::pair<size_t,size_t> ij = offDiagToIJ( bfRef.meaCov(), iPar );
      size_t iMea = ij.first;
      size_t jMea = ij.second;
      d1NInfoNew.push_back( d1Infos(iMea,jMea) ); // already normalized
    }
  }
  // Global scale factor
  else if ( this->type() == ByGlobalFactorMD )
  {
    const BlueFish1Obs& bfRef = this->bf();
    if ( nPar != 1 )
      throw std::runtime_error( "nPar != 1 for ByGlobalFactorMD in normInfoAndFirstDerivatives" );
    Number par = ddNew[0];
    BlueFish1Obs bfNew = bfRef.rescaleCorrsByGlobalFactorBF1( par );
    if ( bfNew.nObs() != 1 )
      throw std::runtime_error( "nObs != 1 in normInfoAndFirstDerivatives" );
    const size_t aObs1 = 0; // there is a single observable in this BF
    nInfoNew = bfNew.blueInf()(aObs1,aObs1)/this->infoNorm(); // Normalize by the given factor
    // Use matrix derivatives
    d1NInfoNew.push_back( bfNew.blueD1NormInfByGlobalFactorBF1( bfRef ) ); // already normalized
    // Cross-check against older approximate calculation from first principles
    par += BlueFish1Obs::Delta/10; // new value + delta
    if ( par >= 1 ) return; // do not fail at boundary
    double nInfoNewP = bfRef.rescaleCorrsByGlobalFactorBF1( par ).blueInf()(aObs1,aObs1) / this->infoNorm();
    par -= 2*BlueFish1Obs::Delta/10; // new value - delta
    double nInfoNewM = bfRef.rescaleCorrsByGlobalFactorBF1( par ).blueInf()(aObs1,aObs1) / this->infoNorm();
    if ( !equals<double>( d1NInfoNew[0],
                          ( nInfoNewP - nInfoNewM ) / (2*BlueFish1Obs::Delta/10),
                          2E-4, "2E-4" ) ) // 1.5E-4 found in lhc2012bJESonly
      throw std::runtime_error( "d1InfoByGlobalFactor mismatch?" );
  }
  // UNKNOWN/UNSUPPORTED TYPE
  else
    throw std::runtime_error( "Temporary? Type not yet handled in normInfoAndFirstDerivatives" );
}

//-----------------------------------------------------------------------------

bool InfoMinimizer::minimizeUsingROOT( Number& runMinVal,
                                       std::vector<Number>& runSfs,
                                       std::vector<Number>& runDSfs,
                                       unsigned& runNCalls,
                                       const std::set<size_t>& parsTV,
                                       const bool downFromNomCor,
                                       const std::vector<Number>& runSfsStartOriginal,
                                       std::ostream& tStr,
                                       const bool debugROOT ) const
{
  std::vector<Number> runSfsStart = runSfsStartOriginal;
  if ( InfoMinimizerImpl::s_imForROOT != 0 )
    throw std::runtime_error( "Static InfoMinimizer is already being used" );
  InfoMinimizerImpl::s_imForROOT = this;
  InfoMinimizerImpl::s_streamForNormInfoTM = &tStr;
  InfoMinimizerImpl::s_infoMax = -1;
  char out[80];
  size_t nPar = this->nPar();
  // PREPARE INFORMATION MINIMIZATION (USING ROOT)
  // See http://root.cern.ch/root/html/tutorials/fit/NumericalMinimization.C.html
  const std::string rootMinName = "Minuit2";
  const std::string rootAlgName = "Migrad"; // default (same as "")
  //const std::string rootAlgName = "Simplex"; // ~same as Migrad?
  //const std::string rootAlgName = "Combined"; // ~same as Migrad?
  //const std::string rootAlgName = "Scan"; // VERY BAD (#calls=1???)
  //const std::string rootAlgName = "Fumili"; // DOES NOT WORK
  //const std::string rootMinName = "GSLMultiMin";
  //const std::string rootAlgName = "ConjugateFR"; // CRASH???
  //const std::string rootAlgName = "ConjugatePR"; // CRASH???
  //const std::string rootMinName = "GSLMultiFit";
  //const std::string rootAlgName = ""; // CRASH???
  tStr << "MINIMIZE USING ROOT " << rootMinName << std::endl;
  std::auto_ptr<ROOT::Math::Minimizer> rootMin;
  rootMin.reset( ROOT::Math::Factory::CreateMinimizer( rootMinName, rootAlgName ) );
  //rootAlgName = rootMin->Options().MinimizerAlgorithm(); // always returns Migrad (BUG?)
  rootMin->SetMaxFunctionCalls(1000000); // for Minuit/Minuit2 in ROOT
  rootMin->SetMaxIterations(10000);      // for GSL in ROOT
  //rootMin->SetTolerance(0.0000001);    // BLUEFIN 01.00.00 (no minimum in top byOD)
  rootMin->SetTolerance(0.000001);       // BLUEFIN 01.00.01 (minimum OK in top byOD)
  if ( debugROOT >= 1 ) rootMin->SetPrintLevel(1);
  else rootMin->SetPrintLevel(0);
  // INFORMATION MINIMIZATION (USING ROOT)
  // [Using a non-static private member function (eg &InfoMinimizer::normInfo) fails in ROOT]
  ROOT::Math::Functor rootFunc( &InfoMinimizerImpl::normInfoTM, this->nPar() ); // include fixed and variable
  rootMin->SetFunction( rootFunc );
  //if ( varyAllParameters() ) tStr << "__INFO: ALL PARAMETERS WILL BE VARIED (hardcoded override)__" << std::endl;
  for ( size_t iPar=0; iPar<nPar; ++iPar )
  {
    if ( parsTV.find(iPar) != parsTV.end() )
    {
      Number low = 0;
      Number up = 1;
      if ( downFromNomCor )
      {
        low = 0;
        up = runSfs[iPar];
      }
      else
      {
        low = runSfs[iPar];
        up = 1;
      }
      Number& start = runSfsStart[iPar];
      // Sanity checks
      if ( start < low )
      {
        tStr << "ERROR! StartValue=" << start << " < LowerBound=" << low << std::endl;
        throw std::runtime_error( "StartValue < LowerBound" );
      }
      else if ( up < start )
      {
        tStr << "ERROR! UpperBound=" << up << " < StartValue=" << start << std::endl;
        throw std::runtime_error( "UpperBound < StartValue" );
      }
      else if ( low == up ) // fix the parameter (else ROOT fails)
      {
        rootMin->SetFixedVariable( iPar, this->parName(iPar), start );
      }
      else if ( low > up )
      {
        tStr << "ERROR! LowerBound=" << low << " > UpperBound=" << up << std::endl;
        throw std::runtime_error( "LowerBound > UpperBound" );
      }
      else
      {
        if ( start == up )
        {
          start = low + 0.990 * ( up - low );
          tStr << "WARNING! UpperBound=" << up << " == StartValue=" << up
               << " : redefine StartValue=" << start << std::endl;
        }
        else if ( start == low )
        {
          start = low + 0.010 * ( up - low );
          tStr << "WARNING! LowerBound=" << low << " == StartValue=" << low
               << " : redefine StartValue=" << start << std::endl;
        }
        const Number step = 0.001 * ( up - low );
        rootMin->SetLimitedVariable( iPar, this->parName(iPar), start, step, low, up );
      }
    }
    else
    {
      if ( runSfsStart[iPar] != runSfs[iPar] )
      {
        tStr << "ERROR! StartValue=" << runSfsStart[iPar] << " != Bound=" << runSfs[iPar]
             << " for fixed parameter" << std::endl;
        throw std::runtime_error( "StartValue != Bound for fixed parameter" );
      }
      rootMin->SetFixedVariable( iPar, this->parName(iPar), runSfsStart[iPar] );
    }
  }
  // The running values of the edm and fit status
  Number runEdm = -1;
  bool runStatus = false;
  // Do the minimization
  try
  {
    unsigned done = 0;
    do
    {
      // Printout before minimization
      if ( done==0 )
      {
        if ( m_type == ByErrorSourceMD  )
          tStr << "Error sources (%=FIXED)";
        else if ( m_type == ByOffDiagElemMD )
          tStr << "Off-Diag Elem (%=FIXED)";
        else
          tStr << "Parameters    (%=FIXED)";
        for ( size_t iPar=0; iPar<nPar; ++iPar )
        {
          bool parWasVaried = parsTV.find(iPar)!=parsTV.end();
          snprintf( out, 6, "%5s", ( (parWasVaried?" ":"%") + this->parID(iPar) ).c_str() );
          tStr << (iPar==0?" ":", ") << out;
        }
        tStr << std::endl;
        std::auto_ptr<double> ddStart( new double[nPar] );
        for ( size_t iPar=0; iPar<nPar; ++iPar ) ddStart.get()[iPar] = runSfsStart[iPar];
        snprintf( out, 12, "%11.8f", InfoMinimizerImpl::normInfoTM( ddStart.get() ) );
        tStr << "START: f=" << out << " at";
        for ( size_t iPar=0; iPar<nPar; ++iPar )
        {
          snprintf( out, 6, "%.5f", (double)runSfsStart[iPar] );
          tStr << (iPar==0?" ":", ") << out;
        }
        tStr << std::endl;
      }
      // Printout after minimization
      else
      {
        snprintf( out, 12, "%11.8f", rootMin->MinValue() );
        tStr << "END:   f=" << out << " at";
        for ( size_t iPar=0; iPar<nPar; ++iPar )
        {
          snprintf( out, 6, "%.5f", (double)runSfs[iPar] );
          tStr << (iPar==0?" ":", ") << out;
        }
        tStr << std::endl;
      }
      // Do the minimization
      if ( done == 0 )
      {
        if ( tStr != std::cout )
        {
          // See http://stackoverflow.com/questions/4832603
          //std::cerr << "TEST initial std::cerr" << std::endl;
          char tmpName[] = "/tmp/fileXXXXXX";
          int newCerr = ::mkstemp( tmpName );
          if ( newCerr < 0 )
            throw std::runtime_error( "Failed to open tmp file for writing in minimizeUsingROOT" );
          //std::cerr << "TEST REDIRECT:  " << tmpName << std::endl;
          ::fflush( ::stderr );
          int oldCerr = ::dup( 2 );
          if ( ::dup2( newCerr, 2 ) < 0 )
            throw std::runtime_error( "Failed to redirect stderr in minimizeUsingROOT" );
          ::close( newCerr );
          //std::cerr << "TEST redirected std::cerr" << std::endl;
          runStatus = rootMin->Minimize();
          ::fflush( ::stderr );
          if ( ::dup2( oldCerr, 2 ) < 0 )
            throw std::runtime_error( "Failed to go back to stderr in minimizeUsingROOT" );
          ::close( oldCerr );
          //std::cerr << "TEST back to std::cerr" << std::endl;
          std::ifstream ifStream( tmpName );
          if ( !ifStream.is_open() )
            throw std::runtime_error( "Failed to open tmp file for reading in minimizeUsingROOT" );
          for ( std::string line; std::getline( ifStream, line ); )
            tStr << line << std::endl;
          ifStream.close();
        }
        else
        {
          runStatus = rootMin->Minimize();
        }
        //if ( !runStatus ) runStatus = rootMin->Minimize(); // try again a second time
        //if ( !runStatus ) runStatus = rootMin->Minimize(); // try again a third time
        runMinVal = rootMin->MinValue();
        for ( size_t iPar=0; iPar<nPar; ++iPar )
        {
          runSfs[iPar] = rootMin->X()[iPar];
          runDSfs[iPar] = rootMin->Errors()[iPar];
        }
        runEdm = rootMin->Edm();
        if ( ::isnan( runEdm ) )
          throw std::runtime_error( "Minimization failed - edm is NAN" );
        runNCalls = rootMin->NCalls();
      }
    }
    while( done++ < 1 );
    tStr << "Minimization completed";
    snprintf( out, 10, "%9.7f", (double)runMinVal );
    tStr << ": minValue=" << out;
    snprintf( out, 10, "%9.7f", (double)runEdm );
    tStr << ", edm=" << out;
    snprintf( out, 10, "%-9d", runNCalls);
    tStr << ", #calls=" << out;
    tStr << std::endl;
    tStr << (runStatus?"":"ERROR! ") << "ROOT minimizer " << rootMinName
         << (rootAlgName!=""?" ("+rootAlgName+") ":" ")
         << (runStatus?"found":"did not find") << " a minimum" << std::endl;
    tStr << std::endl;
    // Update the errors
    for ( size_t iPar=0; iPar<nPar; ++iPar )
      if ( parsTV.find(iPar) == parsTV.end() )
        runDSfs[iPar]=-2; // parameter fixed
  }
  catch( ... )
  {
    InfoMinimizerImpl::s_imForROOT = 0;
    InfoMinimizerImpl::s_streamForNormInfoTM = 0;
    InfoMinimizerImpl::s_infoMax = -1;
    throw;
  }
  InfoMinimizerImpl::s_imForROOT = 0;
  InfoMinimizerImpl::s_streamForNormInfoTM = 0;
  InfoMinimizerImpl::s_infoMax = -1;
  return runStatus;
}

//-----------------------------------------------------------------------------

const std::vector<Number>&
InfoMinimizer::d1NInfosNomCor() const
{
  if ( m_d1NInfosNomCor.size() == 0 )
  {
    const size_t nPar = this->nPar();
    // Derivative analysis at nominal correlations ("@1")
    std::vector<Number> parNomCor; // size == nPar
    for ( size_t iPar=0; iPar<nPar; ++iPar ) parNomCor.push_back( 1 ); // full nominal correlations
    Number nInfoNomCor;
    this->normInfoAndFirstDerivatives( parNomCor, nInfoNomCor, m_d1NInfosNomCor );
    if ( !equals( 1, nInfoNomCor, true ) ) // this is true for all types (1 == nominal)!
      throw std::runtime_error( "nInfoNomCor mismatch" );
    if ( m_d1NInfosNomCor.size() != nPar )
      throw std::runtime_error( "d1NInfosNomCor unexpected size" );
  }
  return m_d1NInfosNomCor;
}

//-----------------------------------------------------------------------------

const std::vector<Number>&
InfoMinimizer::d1NInfosZerCor() const
{
  if ( m_d1NInfosZerCor.size() == 0 )
  {
    const size_t nPar = this->nPar();
    const BlueFish1Obs& bfNomCor = bf();
    size_t aObs1 = 0; // all BFs here have only one obs
    const Number infoNomCor = bfNomCor.blueInf()(aObs1,aObs1); // nominal correlations
    const BlueFish1Obs bfZerCor = bfNomCor.keepCorrsForSource(-1); // zero correlations
    const Number infoZerCor = bfZerCor.blueInf()(aObs1,aObs1); // zero correlations
    // Derivative analysis at zero correlations ("@0")
    std::vector<Number> parZerCor; // size == nPar
    for ( size_t iPar=0; iPar<nPar; ++iPar ) parZerCor.push_back( 0 ); // no correlations
    Number nInfoZerCor;
    this->normInfoAndFirstDerivatives( parZerCor, nInfoZerCor, m_d1NInfosZerCor );
    if ( !equals( infoZerCor, infoNomCor*nInfoZerCor, true ) ) // this is true for all types (0 == noSoc)!
      throw std::runtime_error( "nInfoZerCor mismatch" );
    if ( m_d1NInfosZerCor.size() != nPar )
      throw std::runtime_error( "d1NInfosZerCor unexpected size" );
  }
  return m_d1NInfosZerCor;
}

//-----------------------------------------------------------------------------

const std::set<size_t>&
InfoMinimizer::parToVary() const
{
  if ( !m_parToVaryExists )
  {
    const size_t nPar = this->nPar();
    // Derivative analysis at zero correlations ("@0")
    const std::vector<Number>& d1NInfosNomCor = this->d1NInfosNomCor();
    // Derivative analysis at nominal correlations ("@1")
    const std::vector<Number>& d1NInfosZerCor = this->d1NInfosZerCor();
    // LOOP OVER PARAMETERS ONE BY ONE AND PERFORM DERIVATIVE ANALYSIS
    for ( size_t iPar=0; iPar<nPar; ++iPar )
    {
      // Sanity checks (intuitive, no formal demonstration)
      if ( d1NInfosZerCor[iPar] == 0 && d1NInfosNomCor[iPar] != 0 )
        throw std::runtime_error( "d1NInfoZerCor==0 but d1NInfoNomCor!=0 in analyseDerivatives" );
      // Check which parameters should be varied
      if ( InfoMinimizer::varyAllParameters() )
      {
        if ( d1NInfosZerCor[iPar] == 0 && d1NInfosNomCor[iPar] == 0 ) {} // FIXED (info insensitive - uncorrelated error, eg stat)
        else
        {
          m_parToVary.insert( iPar ); // VARY (vary all parameters!)
        }
      }
      else
      {
        if ( d1NInfosZerCor[iPar] > 0 ) {} // FIXED (d/dX@0>0: NEGATIVE CORRELATIONS?)
        else if ( d1NInfosZerCor[iPar] == 0 ) {} // FIXED (info insensitive - uncorrelated error, eg stat)
        else if ( d1NInfosNomCor[iPar] > varParD1NInfoNomCorThreshold() ) // no need to normalize (sfs==1 here)
        {
          m_parToVary.insert( iPar ); // VARY (above threshold)
        }
        else {} // FIXED (below threshold)
      }
    }
    m_parToVaryExists = true;
  }
  return m_parToVary;
}

//-----------------------------------------------------------------------------

void InfoMinimizer::printMinimization( const bool preOnly,
                                       std::ostream& ostr,
                                       bool latex ) const
{
  TextReporter::printMinimization( *this, preOnly, ostr, latex );
}

//-----------------------------------------------------------------------------

const BlueFish1Obs&
InfoMinimizer::minimizeBF1( std::ostream& tStr ) const
{
  FcnType type = this->type();
  // Perform the minimization if not already attempted
  // Special handling for ByOffDiagPerErrSrcMD minimizations
  if ( m_minBf.nObs() == 0 && !m_minimizationFailed &&
       type == ByOffDiagPerErrSrcMD )
  {
    try
    {
      m_minBf = InfoMinimizerImpl::minimizeBF1ByOffDiagPerErrSrcMD( this, tStr, m_subMinimizers );
    }
    catch ( ... )
    {
      m_minimizationFailed = true;
      throw;
    }
  }
  // Perform the minimization if not already attempted
  else if ( m_minBf.nObs() == 0 && !m_minimizationFailed )
  {
    const BlueFish1Obs& bfNomCor = this->bf();
    const size_t nPar = this->nPar();
    // Analyse derivatives and compute the parameters that will be varied
    const std::set<size_t>& parToVary = this->parToVary();
    // Dump pre-minimization derivative analysis
    bool preOnly = true;
    this->printMinimization( preOnly, tStr );
    // No parameters to vary - SKIP MINIMIZATION
    if ( parToVary.size() == 0 )
    {
      tStr << "No parameters to vary. Minimization will be skipped" << std::endl << std::endl;
      m_minBf = bfNomCor;
      for ( size_t iPar=0; iPar<nPar; ++iPar ) m_parMin.push_back( 1 );
      for ( size_t iPar=0; iPar<nPar; ++iPar ) m_dparMin.push_back( 0 );
      m_d1NInfosMinCor = this->d1NInfosNomCor();
    }
    // There are parameters to vary - PERFORM MINIMIZATION
    else
    {
      // Stripped down and polished version of earlier prototypes
      // - Minimize only down from nominal correlations (not up from zero correlations)
      // - Minimize only with ROOT (no 'adHoc')
      // - Minimize only once (no loop over minimization retrials)
      bool downFromNomCor = true;
      // The running values of the scale factors and errors, function value and #calls
      // These vectors have size nPar (but some of these parameters may be fixed)
      Number minVal = 1;
      unsigned nCalls = 0;
      std::vector<Number> par; // this is the upper bound for downFromNomCor=true
      for ( size_t iPar=0; iPar<nPar; ++iPar ) par.push_back( 1 );
      std::vector<Number> dpar;
      for ( size_t iPar=0; iPar<nPar; ++iPar ) dpar.push_back( 1 );
      // The (const) starting values for the minimization
      std::vector<Number> parStart;
      for ( size_t iPar=0; iPar<nPar; ++iPar ) parStart.push_back( 1 ); // hardcoded for fixed parameters
      // Use onionized covariance as initial starting point for ByOffDiagElemMD
      if ( type == ByOffDiagElemMD )
      {
        BlueFish1Obs bfOnionAV = bfNomCor.onionizeCorrsByErrorSourceBF1( false, 0 ); // AV onionization
        for ( size_t iPar=0; iPar<nPar; ++iPar )
        {
          if ( parToVary.find(iPar) != parToVary.end() )
          {
            std::pair<size_t,size_t> ij = offDiagToIJ( bfNomCor.meaCov(), iPar );
            if ( bfNomCor.meaCov()( ij.first, ij.second ) > 0 )
              parStart[iPar] = bfOnionAV.meaCov()( ij.first, ij.second ) / bfNomCor.meaCov()( ij.first, ij.second );
            else
              parStart[iPar] = 0;
          }
        }
      }
      // Use 1 as initial starting point for ByGlobalFactor and ByErrSrc
      else
      {
        for ( size_t iPar=0; iPar<nPar; ++iPar )
        {
          if ( parToVary.find(iPar) != parToVary.end() ) parStart[iPar] = 1;
        }
      }
      // Minimize
      try
      {
        m_wasMinimumFound = this->minimizeUsingROOT( minVal, par, dpar, nCalls, parToVary, downFromNomCor, parStart, tStr );
      }
      catch ( ... )
      {
        m_minimizationFailed = true;
        throw;
      }
      m_parMin = par;
      m_dparMin = dpar;
      // Minimization _seems_ to have been successful but try/catch the rest anyway
      // (e.g. edm=nan with non-positive-negative covariances has been observed here)
      try
      {
        // Compute the derivatives at the minimum
        Number nInfoMinCor;
        std::vector<Number> d1NInfosMinCor;
        this->normInfoAndFirstDerivatives( par, nInfoMinCor, m_d1NInfosMinCor );
        // Create the new BlueFish1Obs
        if ( type == ByErrorSourceMD )
          m_minBf = bfNomCor.rescaleCorrsByErrorSourceBF1( par );
        else if ( type == ByOffDiagElemMD )
          m_minBf = bfNomCor.rescaleCorrsByOffDiagElemBF1( par );
        else if ( type == ByGlobalFactorMD && par.size() == 1 )
          m_minBf = bfNomCor.rescaleCorrsByGlobalFactorBF1( par[0] );
        else
        {
          // No path to these statements (internal error - PANIC)
          m_minimizationFailed = true;
          throw std::runtime_error( "INTERNAL ERROR! Unknown type to rescale in minimizeBF1?" );
        }
        // Dump derivatives and results after the minimization
        preOnly = false;
        this->printMinimization( preOnly, tStr );
      }
      catch( std::exception& e )
      {
        throw std::runtime_error( "Exception caught in post-minimization step within minimizeBF1 - "
                                  + std::string(e.what()) );
      }
      catch( ... )
      {
        throw std::runtime_error( "Unknown exception caught in post-minimization step within minimizeBF1" );
      }
    }
  }
  // Hook to test printout when minimizationByOD fails
  if ( type == ByOffDiagElemMD && ::getenv( "BLUEFIN_TEST_MINIMIZATION_FAILED" ) )
    m_minimizationFailed = true;
  // Did the minimization fail?
  if ( m_minimizationFailed )
    throw std::runtime_error( "Minimization failed (in minimizeBF1)" );
  // Return the results
  return m_minBf;
}

//-----------------------------------------------------------------------------

bool
InfoMinimizer::wasMinimumFound() const
{
  // Did the minimization fail?
  if ( m_minimizationFailed )
    throw std::runtime_error( "Minimization failed (in wasMinimumFound)" );
  // Perform the minimization if not already attempted
  else if ( m_minBf.nObs() == 0 )
    minimizeBF1();
  // Return the results
  return m_wasMinimumFound;
}

//-----------------------------------------------------------------------------

const std::vector<Number>&
InfoMinimizer::parMin() const
{
  // Did the minimization fail?
  if ( m_minimizationFailed )
    throw std::runtime_error( "Minimization failed (in parMin)" );
  // Perform the minimization if not already attempted
  else if ( m_minBf.nObs() == 0 )
    minimizeBF1();
  // Return the results
  return m_parMin;
}

//-----------------------------------------------------------------------------

const std::vector<Number>&
InfoMinimizer::dparMin() const
{
  // Did the minimization fail?
  if ( m_minimizationFailed )
    throw std::runtime_error( "Minimization failed (in dparMin)" );
  // Perform the minimization if not already attempted
  else if ( m_minBf.nObs() == 0 )
    minimizeBF1();
  // Return the results
  return m_dparMin;
}

//-----------------------------------------------------------------------------

const std::vector<Number>&
InfoMinimizer::d1NInfosMinCor() const
{
  // Did the minimization fail?
  if ( m_minimizationFailed )
    throw std::runtime_error( "Minimization failed (in d1NInfosMinCor)" );
  // Perform the minimization if not already attempted
  else if ( m_minBf.nObs() == 0 )
    minimizeBF1();
  // Return the results
  return m_d1NInfosMinCor;
}

//-----------------------------------------------------------------------------

const std::vector<InfoMinimizer*>&
InfoMinimizer::subMinimizers() const
{
  // Did the minimization fail?
  if ( m_minimizationFailed )
    throw std::runtime_error( "Minimization failed (in subMinimizers)" );
  // Perform the minimization if not already attempted
  else if ( m_minBf.nObs() == 0 )
    minimizeBF1();
  // Return the sub minimizers
  return m_subMinimizers;
}

//-----------------------------------------------------------------------------
