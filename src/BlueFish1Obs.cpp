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
#include <algorithm>
#include <cstdio>
#include "boost_bind_headers.h"
#include "BlueFin/BlueFish1Obs.h"
#include "BlueFin/TextReporter.h"

// Namespace
using namespace bluefin;

//-----------------------------------------------------------------------------

BlueFish1Obs::~BlueFish1Obs()
{
}

//-----------------------------------------------------------------------------

BlueFish1Obs::BlueFish1Obs( const std::string& combName,
                            const std::string& obsName,
                            const std::vector<std::string>& meaNames,
                            const Vector& meaVal,
                            const std::vector<std::string>& errNames,
                            const std::vector<SymmetricMatrix>& errMeaCovs,
                            bool debug )
  : BlueFish( combName, std::vector<std::string>( 1, obsName ), meaNames,
              std::vector<size_t>( meaNames.size(), 0 ), meaVal, errNames, errMeaCovs, debug )
  , m_blueDInfosRef()
  , m_blueD1NormInfByOffDiagElemByErrorSourceBF1()
  , m_blueD1NormInfByOffDiagElemBF1()
  , m_blueD1NormInfByErrorSourceBF1()
  , m_blueD1NormInfByGlobalFactorBF1()
  , m_blueFiw()
  , m_blueFiwTot()
  , m_blueMiw()
  , m_blueMiwTot()
  , m_blueRiw()
  , m_blueRiwTot()
{
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in BlueFish1Obs ctor" );
}

//-----------------------------------------------------------------------------

BlueFish1Obs::BlueFish1Obs( const BlueFish& bf )
  : BlueFish( bf )
  , m_blueDInfosRef()
  , m_blueD1NormInfByOffDiagElemByErrorSourceBF1()
  , m_blueD1NormInfByOffDiagElemBF1()
  , m_blueD1NormInfByErrorSourceBF1()
  , m_blueD1NormInfByGlobalFactorBF1()
  , m_blueFiw()
  , m_blueFiwTot()
  , m_blueMiw()
  , m_blueMiwTot()
  , m_blueRiw()
  , m_blueRiwTot()
{
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in BlueFish1Obs ctor" );
}

//-----------------------------------------------------------------------------

BlueFish1Obs::BlueFish1Obs()
  : BlueFish()
  , m_blueDInfosRef()
  , m_blueD1NormInfByOffDiagElemByErrorSourceBF1()
  , m_blueD1NormInfByOffDiagElemBF1()
  , m_blueD1NormInfByErrorSourceBF1()
  , m_blueD1NormInfByGlobalFactorBF1()
  , m_blueFiw()
  , m_blueFiwTot()
  , m_blueMiw()
  , m_blueMiwTot()
  , m_blueRiw()
  , m_blueRiwTot()
{
}

//-----------------------------------------------------------------------------

BlueFish1Obs::BlueFish1Obs( const BlueFish1Obs& rhs )
  : BlueFish( rhs )
  , m_blueDInfosRef( rhs.m_blueDInfosRef.get() == 0 ? 0 : new BlueFish1Obs( *(rhs.m_blueDInfosRef.get()) ) )
  , m_blueD1NormInfByOffDiagElemByErrorSourceBF1( rhs.m_blueD1NormInfByOffDiagElemByErrorSourceBF1 )
  , m_blueD1NormInfByOffDiagElemBF1( rhs.m_blueD1NormInfByOffDiagElemBF1 )
  , m_blueD1NormInfByErrorSourceBF1( rhs.m_blueD1NormInfByErrorSourceBF1 )
  , m_blueD1NormInfByGlobalFactorBF1( rhs.m_blueD1NormInfByGlobalFactorBF1.get() != 0 ?
                                      new Number( *(rhs.m_blueD1NormInfByGlobalFactorBF1) ) : 0 )
  , m_blueFiw( rhs.m_blueFiw )
  , m_blueFiwTot( rhs.m_blueFiwTot )
  , m_blueMiw( rhs.m_blueMiw )
  , m_blueMiwTot( rhs.m_blueMiwTot )
  , m_blueRiw( rhs.m_blueRiw )
  , m_blueRiwTot( rhs.m_blueRiwTot )
{
}

//-----------------------------------------------------------------------------

BlueFish1Obs&
BlueFish1Obs::operator=( const BlueFish1Obs& rhs )
{
  BlueFish::operator=( rhs );
  m_blueDInfosRef.reset( rhs.m_blueDInfosRef.get() == 0 ? 0 : new BlueFish1Obs( *(rhs.m_blueDInfosRef.get()) ) );
  m_blueD1NormInfByOffDiagElemByErrorSourceBF1 = rhs.m_blueD1NormInfByOffDiagElemByErrorSourceBF1;
  m_blueD1NormInfByOffDiagElemBF1 = rhs.m_blueD1NormInfByOffDiagElemBF1;
  m_blueD1NormInfByErrorSourceBF1 = rhs.m_blueD1NormInfByErrorSourceBF1;
  m_blueD1NormInfByGlobalFactorBF1.reset( rhs.m_blueD1NormInfByGlobalFactorBF1.get() != 0 ?
                                          new Number( *(rhs.m_blueD1NormInfByGlobalFactorBF1) ) : 0 );
  m_blueFiw = rhs.m_blueFiw;
  m_blueFiwTot = rhs.m_blueFiwTot;
  m_blueMiw = rhs.m_blueMiw;
  m_blueMiwTot = rhs.m_blueMiwTot;
  m_blueRiw = rhs.m_blueRiw;
  m_blueRiwTot = rhs.m_blueRiwTot;
  return *this;
}

//-----------------------------------------------------------------------------

BlueFish1Obs
BlueFish1Obs::onionizeCorrsByErrorSourceBF1( bool rc, std::ostream* ptStr ) const
{
  if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish1Obs is empty" );
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in onionizeCorrsByErrorSourceBF1" );
  if ( this->hasNegativeMeaCorr() )
    throw std::runtime_error( "Negative correlations found in onionizeCorrsByErrorSourceBF1" );
  std::vector<SymmetricMatrix> errMeaCovs;
  std::vector<size_t> meaByErr; // list of iMea ordered by decr error
  unsigned dscf = TextReporter::getDownscaleFactor( *this ); // try to guess a scale factor for optimal printout
  // Loop over error sources
  for ( size_t iErr=0; iErr<nErr(); ++iErr )
  {
    const SymmetricMatrix& cov = m_errMeaCovs[iErr];
    // Order measurements by decreasing error
    // See http://stackoverflow.com/questions/279854
    if ( ( rc && meaByErr.size() == 0 ) // do it once for RC algo
         || !rc ) // do it each time for AV algo
    {
      if ( !rc ) meaByErr.clear();
      std::vector<std::pair<size_t, Number> > pairs;
      for ( size_t iMea=0; iMea<nMea(); ++iMea )
      {
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
        if ( !rc ) // AV: use the partial covariance
          pairs.push_back( std::make_pair<size_t, Number>(iMea,cov(iMea,iMea)) );
        else // RC: use the full covariance
          pairs.push_back( std::make_pair<size_t, Number>(iMea,m_meaCov(iMea,iMea)) );
#else
        if ( !rc ) // AV: use the partial covariance
          pairs.push_back( std::make_pair(iMea,cov(iMea,iMea)) );
        else // RC: use the full covariance
          pairs.push_back( std::make_pair(iMea,m_meaCov(iMea,iMea)) );
#endif
      }
      std::sort( pairs.begin(), pairs.end(),
                 boost::bind( &std::pair<size_t, Number>::second, _1) >
                 boost::bind( &std::pair<size_t, Number>::second, _2) );
      for ( std::vector<std::pair<size_t, Number> >::const_iterator
              pair = pairs.begin(); pair != pairs.end(); pair++ )
        meaByErr.push_back( pair->first );
      if ( meaByErr.size() != nMea() )
        throw std::runtime_error( "nMea/meaByErr mismatch???" );
    }
    // Onionize the covariance for this error source
    // Loop over measurements from those with smallest non-0 errors
    SymmetricMatrix covOnion = ZeroMatrix( nMea() );
    for ( size_t kMeaBE=0; kMeaBE<nMea(); kMeaBE++ )
    {
      size_t kMea = meaByErr[kMeaBE];
      Number err2 = cov(kMea,kMea);
      covOnion(kMea,kMea) = err2;
      for ( size_t iMeaBE=0; iMeaBE<kMeaBE; iMeaBE++ )
      {
        size_t iMea = meaByErr[iMeaBE];
        if ( cov(iMea,kMea) != cov(kMea,iMea) )
          throw std::runtime_error( "Covariance matrix is not symmetric?" );
        if ( err2 == 0 && cov(iMea,kMea) != 0 )
          throw std::runtime_error( "non-0 off diagonal for 0 diagonal?" );
        else if ( cov(iMea,kMea) > err2 )
        {
          covOnion(iMea,kMea) = err2;
          covOnion(kMea,iMea) = err2;
        }
        else
        {
          covOnion(iMea,kMea) = cov(iMea,kMea);
          covOnion(kMea,iMea) = cov(iMea,kMea);
        }
      }
    }
    if ( ptStr )
    {
      *ptStr << "Onionize source #" << iErr
             << " (" << errNames()[iErr] << ")" << std::endl;
      TextReporter::printMatrix( "CovOld", cov, this->meaNames(), dscf, *ptStr );
      TextReporter::printMatrix( "CovNew", covOnion, this->meaNames(), dscf, *ptStr );
      *ptStr << std::endl;
    }
    errMeaCovs.push_back( covOnion );
  }
  return changeMeaCovSources( errMeaCovs );
}

//-----------------------------------------------------------------------------

BlueFish1Obs
BlueFish1Obs::rescaleCorrsByErrorSourceBF1( const std::vector<Number>& errCorrSFs ) const
{
  if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish1Obs is empty" );
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in rescaleCorrsByErrorSourceBF1" );
  if ( errCorrSFs.size() != this->nErr() )
    throw std::runtime_error( "nErr mismatch in rescaleCorrsByErrorSourceBF1" );
  // Debug printout
  if ( false )
  {
    std::cout << "Scale factors:";
    for ( size_t iErr=0; iErr<nErr(); ++iErr ) std::cout << " " << errCorrSFs[iErr];
    std::cout << std::endl;
  }
  // Loop over error sources and compute non-diagonal elements in a triangle
  std::vector<SymmetricMatrix> errMeaCovs;
  for ( size_t iErr=0; iErr<nErr(); ++iErr )
  {
    SymmetricMatrix cov = m_errMeaCovs[iErr];
    const Number sf = errCorrSFs[iErr];
    if ( sf < 0 )
    {
      std::cout << "ERROR! Invalid scale factor " << sf << "<0" << std::endl;
      throw std::runtime_error( "scale factor not in [0,1] in rescaleCorrsByErrorSourceBF1" );
    }
    if ( 1E-17 < (sf-1) ) // protect against sf = 1 + fewE-18
    {
      std::cout << "ERROR! Invalid scale factor " << sf << ">1 (sf-1=" << sf-1 << ")" << std::endl;
      throw std::runtime_error( "scale factor not in [0,1] in rescaleCorrsByErrorSourceBF1" );
    }
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
      for ( size_t jMea=0; jMea<iMea; ++jMea )
      {
        if ( cov(iMea,jMea) != cov(jMea,iMea) )
          throw std::runtime_error( "error source non symmetric in rescaleCorrsByErrorSourceBF1" );
        cov(iMea,jMea) = cov(iMea,jMea) * sf;
        cov(jMea,iMea) = cov(iMea,jMea);
      }
    errMeaCovs.push_back( cov );
  }
  BlueFish1Obs bf = changeMeaCovSources( errMeaCovs );
  // Debug printouts
  if ( false )
  {
    std::cout << "Rescale by";
    for ( size_t iErr=0; iErr<nErr(); ++iErr ) std::cout << " " << errCorrSFs[iErr];
    std::cout << std::endl;
    unsigned dscf = TextReporter::getDownscaleFactor( *this ); // try to guess a scale factor for optimal printout
    TextReporter::printMatrix( "CovOld", this->meaCov(), this->meaNames(), dscf );
    TextReporter::printMatrix( "CovNew", bf.meaCov(), this->meaNames(), dscf );
  }
  return bf;
}

//-----------------------------------------------------------------------------

Number
BlueFish1Obs::infoRescaledCorrsByErrorSourceBF1( const std::vector<Number>& errCorrSFs ) const
{
  if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish1Obs is empty" );
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in infoRescaledCorrsByErrorSourceBF1" );
#if false
  /// --- Use rescaleCorrsByErrorSourceBF1 instead of the original alternative implementation
  size_t aObs1 = 0; // there is a single observable in this BF
  return this->rescaleCorrsByErrorSourceBF1( errCorrSFs ).blueInf()(aObs1,aObs1);
#else
  /// --- The original alternative implementation is faster (2.1s instead of 3.7s for top)
  if ( errCorrSFs.size() != this->nErr() )
    throw std::runtime_error( "nErr mismatch in infoRescaledCorrsByErrorSourceBF1" );
  SymmetricMatrix meaCov = ZeroMatrix( this->nMea() );
  // Debug printout
  if ( false )
  {
    std::cout << "Scale factors:";
    for ( size_t iErr=0; iErr<nErr(); ++iErr ) std::cout << " " << errCorrSFs[iErr];
    std::cout << std::endl;
  }
  // Loop over error sources and compute non-diagonal elements in a triangle
  for ( size_t iErr=0; iErr<nErr(); ++iErr )
  {
    const SymmetricMatrix& cov = m_errMeaCovs[iErr]; // Assume symmetric (DO NOT CHECK to speed this up)
    const Number sf = errCorrSFs[iErr];
    if ( sf < 0-2*Delta || 1 < sf ) // relax lower limit to -2*delta to compute second derivatives
    {
      std::cout << "ERROR! Invalid scale factor " << sf << std::endl;
      throw std::runtime_error( "scale factor not in [0,1] in infoRescaledCorrsByErrorSourceBF1" );
    }
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
      for ( size_t jMea=0; jMea<iMea; ++jMea )
        meaCov( iMea, jMea ) += cov( iMea, jMea ) * sf;
  }
  //std::cout << "HALF " << meaCov << std::endl;
  // Fill in the diagonal elements and the other triangle to make the covariance symmetric
  for ( size_t iMea=0; iMea<nMea(); ++iMea )
  {
    meaCov( iMea, iMea ) = this->meaCov()( iMea, iMea );
    for ( size_t jMea=0; jMea<iMea; ++jMea )
      meaCov( jMea, iMea ) = meaCov( iMea, jMea );
  }
  //std::cout << "FULL " << meaCov << std::endl;
  // Debug printouts
  if ( false )
  {
    std::cout << "Rescale by";
    for ( size_t iErr=0; iErr<nErr(); ++iErr ) std::cout << " " << errCorrSFs[iErr];
    std::cout << std::endl;
    unsigned dscf = TextReporter::getDownscaleFactor( *this ); // try to guess a scale factor for optimal printout
    TextReporter::printMatrix( "CovOld", this->meaCov(), this->meaNames(), dscf );
    TextReporter::printMatrix( "CovNew", meaCov, this->meaNames(), dscf );
  }
  // Invert the covariance and compute the 1 x 1 information matrix
  SymmetricMatrix meaCovInv;
  if ( !SymmetricMatrix_invert( meaCov, meaCovInv ) )
    throw std::runtime_error( "cannot invert covariance in infoRescaledCorrsByErrorSourceBF1" );
  Number info = 0;
  for ( size_t iMea=0; iMea<nMea(); ++iMea )
  {
    info += meaCovInv( iMea, iMea );
    for ( size_t jMea=0; jMea<iMea; ++jMea )
      info += 2 * meaCovInv( iMea, jMea );
  }
#endif
  return info;
}

//-----------------------------------------------------------------------------

BlueFish1Obs
BlueFish1Obs::rescaleCorrsByOffDiagElemBF1( const std::vector<Number>& sfs ) const
{
  if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish1Obs is empty" );
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in rescaleCorrsByOffDiagElemBF1" );
  size_t nPar = nOffDiag( this->meaCov() );
  if ( sfs.size() != nPar )
    throw std::runtime_error( "nPar mismatch in rescaleCorrsByOffDiagElemBF1" );
  // Debug printout
  if ( false )
  {
    std::cout << "Scale factors:";
    for ( size_t iPar=0; iPar<nPar; ++iPar ) std::cout << " " << sfs[iPar];
    std::cout << std::endl;
  }
  // Sanity checks
  for ( size_t iPar=0; iPar<nPar; ++iPar )
  {
    const Number sf = sfs[iPar];
    if ( sf < 0 )
    {
      std::cout << "ERROR! Invalid scale factor " << sf << "<0" << std::endl;
      throw std::runtime_error( "scale factor not in [0,1] in rescaleCorrsByOffDiagElemBF1" );
    }
    if ( 1E-17 < (sf-1) ) // protect against sf = 1 + fewE-18
    {
      std::cout << "ERROR! Invalid scale factor " << sf << ">1 (sf-1=" << sf-1 << ")" << std::endl;
      throw std::runtime_error( "scale factor not in [0,1] in rescaleCorrsByOffDiagElemBF1" );
    }
  }
  // Loop over error sources and compute non-diagonal elements
  std::vector<SymmetricMatrix> errMeaCovs;
  for ( size_t iErr=0; iErr<this->nErr(); ++iErr )
  {
    SymmetricMatrix cov = m_errMeaCovs[iErr];
    size_t odCheck = 0;
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
      for ( size_t jMea=0; jMea<iMea; ++jMea )
      {
        if ( cov(iMea,jMea) != cov(jMea,iMea) )
          throw std::runtime_error( "error source non symmetric in rescaleCorrsByOffDiagElemBF1" );
        size_t iPar = offDiagFromIJ( cov, iMea, jMea );
        if ( iPar != odCheck++ )
          throw std::runtime_error( "od mapping error in rescaleCorrsByOffDiagElemBF1" );
        Number sf = sfs[iPar];
        cov(iMea,jMea) = cov(iMea,jMea) * sf;
        cov(jMea,iMea) = cov(iMea,jMea);
      }
    errMeaCovs.push_back( cov );
  }
  BlueFish1Obs bf = changeMeaCovSources( errMeaCovs );
  // Debug printouts
  if ( false )
  {
    std::cout << "Rescale by";
    for ( size_t iPar=0; iPar<nPar; ++iPar ) std::cout << " " << sfs[iPar];
    std::cout << std::endl;
    unsigned dscf = TextReporter::getDownscaleFactor( *this ); // try to guess a scale factor for optimal printout
    TextReporter::printMatrix( "CovOld", this->meaCov(), this->meaNames(), dscf );
    TextReporter::printMatrix( "CovNew", bf.meaCov(), this->meaNames(), dscf );
  }
  return bf;
}

//-----------------------------------------------------------------------------

BlueFish1Obs
BlueFish1Obs::rescaleCorrsByGlobalFactorBF1( Number sf ) const
{
  if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish1Obs is empty" );
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in rescaleCorrsByGlobalFactorBF1" );
  if ( sf < 0-2*Delta ) // relax lower limit to -2*delta to compute second derivatives
  {
    std::cout << "ERROR! Invalid scale factor " << sf << "<0" << std::endl;
    throw std::runtime_error( "scale factor not in [0,1] in rescaleCorrsByGlobalFactorBF1" );
  }
  if ( 1E-17 < (sf-1) ) // protect against sf = 1 + fewE-18
  {
    std::cout << "ERROR! Invalid scale factor " << sf << ">1 (sf-1=" << sf-1 << ")" << std::endl;
    throw std::runtime_error( "scale factor not in [0,1] in rescaleCorrsByGlobalFactorBF1" );
  }
  // Loop over error sources and compute non-diagonal elements in a triangle
  std::vector<SymmetricMatrix> errMeaCovs;
  for ( size_t iErr=0; iErr<nErr(); ++iErr )
  {
    SymmetricMatrix cov = m_errMeaCovs[iErr];
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
      for ( size_t jMea=0; jMea<iMea; ++jMea )
      {
        if ( cov(iMea,jMea) != cov(jMea,iMea) )
          throw std::runtime_error( "error source non symmetric in rescaleCorrsByGlobalFactorBF1" );
        cov(iMea,jMea) = cov(iMea,jMea) * sf;
        cov(jMea,iMea) = cov(iMea,jMea);
      }
    errMeaCovs.push_back( cov );
  }
  BlueFish1Obs bf = changeMeaCovSources( errMeaCovs );
  return bf;
}

//-----------------------------------------------------------------------------

BlueFish1Obs
BlueFish1Obs::removeOneMeasInBF1( size_t kMeaRemove ) const
{
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in removeOneMeasInBF1" );
  size_t aObs1 = 0; // there is a single observable in this BF
  std::vector<std::string> meaNames;
  if ( this->nMea() == 1 ) // cannot remove the only measurement!
    throw std::runtime_error( "nMea == 1 in removeOneMeasInBF1" );
  size_t nMeaKeep = this->nMea()-1;
  Vector meaVal( nMeaKeep );
  for ( size_t iMea=0, iMeaKeep=0; iMea<this->nMea(); iMea++ )
  {
    if ( iMea != kMeaRemove )
    {
      meaNames.push_back( this->meaNames()[iMea] );
      meaVal( iMeaKeep++ ) = this->meaVal()[iMea];
    }
  }
  if ( meaNames.size() != nMeaKeep )
    throw std::runtime_error( "Unexpected #mea in removeOneMeasInBF1" );
  std::vector<SymmetricMatrix> errMeaCovs;
  for ( std::vector<SymmetricMatrix>::const_iterator iCov =
          this->meaCovSources().begin(); iCov != this->meaCovSources().end(); ++iCov )
  {
    SymmetricMatrix cov = ZeroMatrix( nMeaKeep );
    for ( size_t iMea=0, iMeaKeep=0; iMea<this->nMea(); iMea++ )
    {
      if ( iMea != kMeaRemove )
      {
        for ( size_t jMea=0, jMeaKeep=0; jMea<this->nMea(); jMea++ )
        {
          if ( jMea != kMeaRemove )
            cov( iMeaKeep, jMeaKeep++ ) = (*iCov)( iMea, jMea );
        }
        iMeaKeep++;
      }
    }
    errMeaCovs.push_back( cov );
  }
  return BlueFish1Obs( this->combName(), this->obsNames()[aObs1], meaNames, meaVal, this->errNames(), errMeaCovs );
}

//-----------------------------------------------------------------------------

BlueFish1Obs
BlueFish1Obs::removeMeasWithNegativeCvwInBF1( std::vector< std::pair<BlueFish1Obs,size_t> >& removed ) const
{
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in removeMeasWithNegativeCvwInBF1" );
  size_t aObs1 = 0; // there is a single observable in this BF
  /// The process is iterative: remove the most negative CVW until all CVW are positive.
  BlueFish1Obs bfNew = *this;
  removed.clear();
  while( true )
  {
    size_t kMeaMinCvw = bfNew.nMea();
    Number minCvw = 1;
    for ( size_t iMea=0; iMea<bfNew.nMea(); iMea++ )
    {
      if ( bfNew.blueCvw()(aObs1,iMea) <= minCvw )
      {
        kMeaMinCvw = iMea;
        minCvw = bfNew.blueCvw()(aObs1,iMea);
      }
    }
    if ( minCvw <= 0 )
    {
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
      removed.push_back( std::make_pair<BlueFish1Obs,size_t>( bfNew, kMeaMinCvw ) );
#else
      removed.push_back( std::make_pair( bfNew, kMeaMinCvw ) );
#endif
      bfNew = bfNew.removeOneMeasInBF1( kMeaMinCvw );
    }
    else break;
  }
  return bfNew;
}

//-----------------------------------------------------------------------------

bool
BlueFish1Obs::isSmallerBF1( const BlueFish1Obs& ref ) const
{
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in isSmallerBF1" );
  if ( this->hasNegativeMeaCorr() )
    throw std::runtime_error( "Negative correlations (this) found in isSmallerBF1" );
  if ( ref.hasNegativeMeaCorr() )
    throw std::runtime_error( "Negative correlations (ref) found in isSmallerBF1" );
  // Check preliminaries
  if ( m_combName != ref.m_combName ||
       m_obsNames != ref.m_obsNames ||
       m_meaNames != ref.m_meaNames ||
       m_obsForMea != ref.m_obsForMea ||
       m_meaForObs != ref.m_meaForObs ||
       !equals( m_uMap, ref.m_uMap, false ) || // strict equality
       !equals( m_meaVal, ref.m_meaVal, false ) || // strict equality
       m_errNames != ref.m_errNames )
    return false;
  // Check covariance details
  //static bool warning = false; // ONE warning per application!
  for ( size_t iErr=0; iErr<nErr(); ++iErr )
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
    {
      // Check diagonal elements (equality)
      if ( m_errMeaCovs[iErr](iMea,iMea) != ref.m_errMeaCovs[iErr](iMea,iMea) ) return false;
      // Check off-diagonal elements (smaller or equal)
      for ( size_t jMea=0; jMea<iMea; ++jMea )
      {
        if ( m_errMeaCovs[iErr](iMea,jMea) != m_errMeaCovs[iErr](jMea,iMea) )
          throw std::runtime_error( "Covariance is not symmetric?" );
        if ( ref.m_errMeaCovs[iErr](iMea,jMea) != ref.m_errMeaCovs[iErr](jMea,iMea) )
          throw std::runtime_error( "Reference covariance is not symmetric?" );
        if ( m_errMeaCovs[iErr](iMea,jMea) > ref.m_errMeaCovs[iErr](iMea,jMea) )
        {
          if ( m_errMeaCovs[iErr](iMea,jMea) < 0 || ref.m_errMeaCovs[iErr](iMea,jMea) < 0 )
          {
            //if ( !warning )
            //  std::cout << "WARNING! Negative correlations have been found in #" << iErr
            //            << "(" << iMea << "," << jMea << ")" << std::endl;
            //warning = true;
            ////return false; // COMMENT OUT TO ALLOW THIS TO GO THROUGH...
          }
          else
          {
            if ( ref.m_errMeaCovs[iErr](iMea,jMea) == 0 )
            {
              return false;
            }
            else
            {
              Number relDiff = (m_errMeaCovs[iErr](iMea,jMea)-ref.m_errMeaCovs[iErr](iMea,jMea)) /
                ref.m_errMeaCovs[iErr](iMea,jMea);
              if ( relDiff > 2E-16 )
              {
                // Allow relative differences of 2E-16 (2.1E-17 found in xse)
                std::cout << "WARNING! Covariance matrices not compatible: " << m_errMeaCovs[iErr](iMea,jMea)
                          << " > " << ref.m_errMeaCovs[iErr](iMea,jMea)
                          << " (relative difference " << relDiff << ")" << std::endl;
                return false;
              }
            }
          }
        }
      }
    }
  return true;
}

//-----------------------------------------------------------------------------

void
BlueFish1Obs::setDInfosRefBF1( const BlueFish1Obs& ref ) const
{
  if ( this->nObs() != 1 )
    throw std::runtime_error( "this->nObs != 1 in setDInfosRefBF1" );
  if ( m_blueDInfosRef.get() == 0 )
  {
    if ( ref.nObs() != 1 )
      throw std::runtime_error( "ref.nObs != 1 in setDInfosRefBF1" );
    // Check that this BF is compatible with the reference
    if ( !this->isSmallerBF1( ref ) ) // this could dump many warnings...
      throw std::runtime_error( "incompatible reference in setDInfosRefBF1" );
    m_blueDInfosRef.reset( new BlueFish1Obs( ref ) );
    return;
  }
  // If a reference already exists, check that this is the same, else throw
  if ( *m_blueDInfosRef != ref )
    throw std::runtime_error( "a different reference already exists in setDInfosRefBF1" );
}

//-----------------------------------------------------------------------------

const std::vector<TriangularMatrix>&
BlueFish1Obs::blueD1NormInfByOffDiagElemByErrorSourceBF1( const BlueFish1Obs& ref ) const
{
  this->setDInfosRefBF1( ref );
  if ( this->nObs() != 1 )
    throw std::runtime_error( "nObs != 1 in blueD1NormInfByOffDiagElemByErrorSourceBF1" );
  const size_t aObs1 = 0; // there is a single observable in this BF
  const Number refInfo = ref.blueInf()(aObs1,aObs1);
  // Compute on demand the first derivatives
  if ( m_blueD1NormInfByOffDiagElemByErrorSourceBF1.size() == 0 ) // expect nErr() > 0
  {
    for ( size_t iErr=0; iErr<nErr(); ++iErr )
    {
      TriangularMatrix blueD1NormInfByOffDiagElem = ZeroMatrix( nMea() );
      for ( size_t iMea=0; iMea<nMea(); ++iMea )
        for ( size_t jMea=0; jMea<iMea; ++jMea )
        {
          blueD1NormInfByOffDiagElem(iMea,jMea) = -2 *
            m_uMapTrByMeaCovInv(aObs1,iMea) *
            m_uMapTrByMeaCovInv(aObs1,jMea) *
            m_blueDInfosRef->m_errMeaCovsOffDiag[iErr](iMea,jMea); // n_ij from reference!
        }
      m_blueD1NormInfByOffDiagElemByErrorSourceBF1.push_back( blueD1NormInfByOffDiagElem / refInfo );
    }
  }
  return m_blueD1NormInfByOffDiagElemByErrorSourceBF1;
}

//-----------------------------------------------------------------------------

const TriangularMatrix&
BlueFish1Obs::blueD1NormInfByOffDiagElemBF1( const BlueFish1Obs& ref ) const
{
  this->setDInfosRefBF1( ref );
  // Compute on demand the first derivatives summing over sources
  if ( m_blueD1NormInfByOffDiagElemBF1.size1() == 0 ) // expect nMea() > 0
  {
    this->blueD1NormInfByOffDiagElemByErrorSourceBF1( ref ); // compute on demand - already normalized!
    m_blueD1NormInfByOffDiagElemBF1 = ZeroMatrix( nMea() );
    for ( size_t iErr=0; iErr<nErr(); ++iErr )
      m_blueD1NormInfByOffDiagElemBF1 += m_blueD1NormInfByOffDiagElemByErrorSourceBF1[iErr];
  }
  return m_blueD1NormInfByOffDiagElemBF1;
}

//-----------------------------------------------------------------------------

const std::vector<Number>&
BlueFish1Obs::blueD1NormInfByErrorSourceBF1( const BlueFish1Obs& ref ) const
{
  this->setDInfosRefBF1( ref );
  // Compute on demand the first derivatives summing over off diagonal elements
  if ( m_blueD1NormInfByErrorSourceBF1.size() == 0 ) // expect nErr() > 0
  {
    this->blueD1NormInfByOffDiagElemByErrorSourceBF1( ref ); // compute on demand - already normalized!
    for ( size_t iErr=0; iErr<nErr(); ++iErr )
    {
      Number blueD1Inf = 0;
      for ( size_t iMea=0; iMea<nMea(); ++iMea )
        for ( size_t jMea=0; jMea<iMea; ++jMea ) // NB iMea<jMea (AVOID DOUBLE COUNTING)
          blueD1Inf += m_blueD1NormInfByOffDiagElemByErrorSourceBF1[iErr](iMea,jMea);
      m_blueD1NormInfByErrorSourceBF1.push_back( blueD1Inf );
    }
  }
  return m_blueD1NormInfByErrorSourceBF1;
}

//-----------------------------------------------------------------------------

const Number&
BlueFish1Obs::blueD1NormInfByGlobalFactorBF1( const BlueFish1Obs& ref ) const
{
  this->setDInfosRefBF1( ref );
  // Compute on demand the first derivative
  if ( m_blueD1NormInfByGlobalFactorBF1.get() == 0 )
  {
    m_blueD1NormInfByGlobalFactorBF1.reset( new Number() );
    // Compute by summing over error sources
    this->blueD1NormInfByErrorSourceBF1( ref ); // compute on demand - already normalized!
    for ( size_t iErr=0; iErr<nErr(); ++iErr )
      *m_blueD1NormInfByGlobalFactorBF1 += m_blueD1NormInfByErrorSourceBF1[iErr];
    // Cross-check by summing over off-diagonal elements
    this->blueD1NormInfByOffDiagElemBF1( ref ); // compute on demand - already normalized!
    Number blueD1NormInf = 0;
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
      for ( size_t jMea=0; jMea<iMea; ++jMea ) // NB iMea<jMea (AVOID DOUBLE COUNTING)
        blueD1NormInf += m_blueD1NormInfByOffDiagElemBF1(iMea,jMea);
    if ( !equals( *m_blueD1NormInfByGlobalFactorBF1, blueD1NormInf, true ) )
      throw std::runtime_error( "m_blueD1NormInfByGlobalFactorBF1 mismatch in blueD1NormInfByGlobalFactorBF1" );
  }
  return *m_blueD1NormInfByGlobalFactorBF1;
}

//-----------------------------------------------------------------------------

const Matrix&
BlueFish1Obs::blueFiw() const
{
  // Compute the nObs by nMea 'FIW' matrix
  if ( m_blueFiw.size1() == 0 ) // expect nObs() > 0
  {
    m_blueFiw = ZeroMatrix( this->nObs(), this->nMea() );
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
    {
      size_t aObs = this->obsForMea(iMea);
      m_blueFiw(aObs,iMea) = this->blueCov()(aObs,aObs) / this->meaCov()(iMea,iMea);
    }
  }
  return m_blueFiw;
}

//-----------------------------------------------------------------------------

const SymmetricMatrix&
BlueFish1Obs::blueFiwTot() const
{
  if ( m_blueFiwTot.size1() == 0 ) // expect nObs() > 0
  {
    // Compute the nObs by nObs 'total FIW' matrix
    // THIS SHOULD _NOT_ NECESSARILY BE EQUAL TO THE IDENTITY MATRIX!
    m_blueFiwTot = prod( m_blueFiw, this->uMap() );
    for ( size_t aObs=0; aObs<this->nObs(); aObs++ )
      for ( size_t bObs=0; bObs<this->nObs(); bObs++ )
        if ( aObs==bObs && m_blueFiwTot(aObs,bObs) <= 0 )
        {
          std::cout << m_blueFiwTot << " has diag elem <= 0 ???" << std::endl;
          std::cout << "MeaCov: " << this->meaCov() << std::endl;
          std::cout << "MeaCovInv: " << this->meaCovInv() << std::endl;
          std::cout << "BlueInf: " << this->blueInf() << std::endl;
          throw std::runtime_error( "FIW weights are not normalized?" );
        }
        else if ( aObs!=bObs && m_blueFiwTot(aObs,bObs) != 0 )
        {
          std::cout << m_blueFiwTot << " has non-diag elem != 0" << std::endl;
          throw std::runtime_error( "FIW weights are not normalized?" );
        }
  }
  return m_blueFiwTot;
}

//-----------------------------------------------------------------------------

const Matrix&
BlueFish1Obs::blueMiw() const
{
  if ( m_blueMiw.size1() == 0 ) // expect nObs() > 0
  {
    // Compute the nObs by nMea 'MIW' matrix
    m_blueMiw = ZeroMatrix( this->nObs(), this->nMea() );
    for ( size_t iMea=0; iMea<this->nMea(); ++iMea )
    {
      size_t aObs = this->obsForMea(iMea);
      double mInfo = 1/this->blueCov()(aObs,aObs); // info with N measurements
      if ( this->nMea() > 1 )
        mInfo -= 1/this->removeOneMeasInBF1(iMea).blueCov()(aObs,aObs); // subtract info with N-1 measurements
      m_blueMiw(aObs,iMea) = mInfo * m_blueCov(aObs,aObs);
    }
  }
  return m_blueMiw;
}

//-----------------------------------------------------------------------------

const SymmetricMatrix&
BlueFish1Obs::blueMiwTot() const
{
  if ( m_blueMiwTot.size1() == 0 ) // expect nObs() > 0
  {
    // Compute the nObs by nObs 'total MIW' matrix
    // THIS SHOULD _NOT_ NECESSARILY BE EQUAL TO THE IDENTITY MATRIX!
    m_blueMiwTot = prod( m_blueMiw, this->uMap() );
    for ( size_t aObs=0; aObs<this->nObs(); aObs++ )
      for ( size_t bObs=0; bObs<this->nObs(); bObs++ )
        if ( aObs==bObs && m_blueMiwTot(aObs,bObs) <= 0 )
        {
          std::cout << m_blueMiwTot << " has diag elem <= 0 ???" << std::endl;
          std::cout << "MeaCov: " << this->meaCov() << std::endl;
          std::cout << "MeaCovInv: " << this->meaCovInv() << std::endl;
          std::cout << "BlueInf: " << this->blueInf() << std::endl;
          throw std::runtime_error( "MIW weights are not normalized?" );
        }
        else if ( aObs!=bObs && m_blueMiwTot(aObs,bObs) != 0 )
        {
          std::cout << m_blueMiwTot << " has non-diag elem != 0" << std::endl;
          throw std::runtime_error( "MIW weights are not normalized?" );
        }
  }
  return m_blueMiwTot;
}

//-----------------------------------------------------------------------------

const Matrix&
BlueFish1Obs::blueRiw() const
{
  if ( m_blueRiw.size1() == 0 ) // expect nObs() > 0
  {
    // Compute the nObs by nMea 'RIW' matrix
    Vector absCvwTot = ZeroVector( nObs() );
    m_blueRiw = ZeroMatrix( this->nObs(), this->nMea() );
    for ( size_t iMea=0; iMea<this->nMea(); ++iMea )
    {
      size_t aObs = this->obsForMea(iMea);
      absCvwTot(aObs) += std::abs( this->blueCvw()(aObs,iMea) );
    }
    for ( size_t iMea=0; iMea<this->nMea(); ++iMea )
    {
      size_t aObs = this->obsForMea(iMea);
      m_blueRiw(aObs,iMea) = std::abs( this->blueCvw()(aObs,iMea) ) / absCvwTot(aObs);
    }
  }
  return m_blueRiw;
}

//-----------------------------------------------------------------------------

const SymmetricMatrix&
BlueFish1Obs::blueRiwTot() const
{
  if ( m_blueRiwTot.size1() == 0 ) // expect nObs() > 0
  {
    // Compute the nObs by nObs 'total RIW' matrix
    // THIS SHOULD BE EQUAL TO THE IDENTITY MATRIX!
    m_blueRiwTot = prod ( m_blueRiw, this->uMap() );
    IdentityMatrix identityNObs( nObs() );
    if( !equals( m_blueRiwTot, identityNObs ) )
    {
      std::cout << m_blueRiwTot << " != " << identityNObs << std::endl;
      throw std::runtime_error( "RIW weights are not normalized?" );
    }
  }
  return m_blueRiwTot;
}

//-----------------------------------------------------------------------------

