//============================================================================
//
// Copyright 2012-2022 CERN
//
// This file is part of the BlueFin software. BlueFin is distributed under
// the terms of the GNU General Public License version 3 (GPL Version 3),
// copied verbatim in file "COPYING". In applying this license, CERN does not
// waive the privileges and immunities granted to it by virtue of its status
// as an Intergovernmental Organization or submit itself to any jurisdiction.
//
// Original author of the software:
//   * Andrea Valassi (CERN) 2012-2022
//
// Based on ideas (http://arxiv.org/abs/1307.4003) by:
//   * Andrea Valassi (CERN)
//   * Roberto Chierici (Univ. Lyon)
//
//============================================================================
// Include files
#include "boost_bind_headers.h"
#include "BlueFin/BlueFish.h"

// Namespace
using namespace bluefin;

//-----------------------------------------------------------------------------

BlueFish::~BlueFish()
{
}

//-----------------------------------------------------------------------------

BlueFish::BlueFish()
  : m_combName()
  , m_obsNames()
  , m_meaNames()
  , m_obsForMea()
  , m_meaForObs()
  , m_uMap()
  , m_meaVal()
  , m_errNames()
  , m_errMeaCovs()
  , m_meaCov()
  , m_errMeaCovsOffDiag()
  , m_meaCovOffDiag()
    /*
  , m_errMeaSetsPosVar()
  , m_errMeaCovsPosVar()
    */
  , m_meaCor()
  , m_meaCovInv()
  , m_uMapTrByMeaCovInv()
  , m_blueInf()
  , m_blueCov()
  , m_blueCor()
  , m_blueCvw()
  , m_blueCvwTot()
  , m_errBlueCovs()
  , m_blueVal()
  , m_blueCh2()
  , m_blueApproxMeaCh2NoCorr()
  , m_blueApproxCh2NoCorr()
  , m_blueApproxObsCh2NoDiffObsCorr()
  , m_hasNegativeMeaCorr( false )
{
}

//-----------------------------------------------------------------------------

BlueFish::BlueFish( const std::string& combName,
                    const std::vector<std::string>& obsNames,
                    const std::vector<std::string>& meaNames,
                    const std::vector<size_t>& obsForMea,
                    const Vector& meaVal,
                    const std::vector<std::string>& errNames,
                    const std::vector<SymmetricMatrix>& errMeaCovs,
                    bool debug )
  : m_combName( combName )
  , m_obsNames( obsNames )
  , m_meaNames( meaNames )
  , m_obsForMea( obsForMea )
  , m_meaForObs()
  , m_uMap( ZeroMatrix( nMea(), nObs() ) )
  , m_meaVal( meaVal )
  , m_errNames( errNames )
  , m_errMeaCovs( errMeaCovs )
  , m_meaCov()
  , m_errMeaCovsOffDiag()
  , m_meaCovOffDiag()
    /*
  , m_errMeaSetsPosVar()
  , m_errMeaCovsPosVar()
    */
  , m_meaCor()
  , m_meaCovInv()
  , m_uMapTrByMeaCovInv()
  , m_blueInf()
  , m_blueCov()
  , m_blueCor()
  , m_blueCvw()
  , m_blueCvwTot()
  , m_errBlueCovs()
  , m_blueVal()
  , m_blueCh2( 0 )
  , m_blueApproxMeaCh2NoCorr()
  , m_blueApproxCh2NoCorr( 0 )
  , m_blueApproxObsCh2NoDiffObsCorr()
  , m_hasNegativeMeaCorr( false )
{
  // =============================
  // === Validate the input    ===
  // =============================
  if ( nMea() == 0 )
    throw std::runtime_error( "#measurement names == 0" );
  if ( nObs() == 0 )
    throw std::runtime_error( "#observable names == 0" );
  // Create the UMAP matrix and the mapping vectors
  if ( obsForMea.size() != nMea() )
    throw std::runtime_error( "Size of mapping vector != #mea" );
  for ( size_t aObs=0; aObs<nObs(); ++aObs )
    m_meaForObs.push_back( std::vector<size_t>() );
  for ( size_t iMea=0; iMea<nMea(); ++iMea )
  {
    if ( obsForMea[iMea] >= nObs() )
      throw std::runtime_error( "One mapping vector value >= #obs" );
    m_uMap( iMea, obsForMea[iMea] ) = 1;
    m_meaForObs[obsForMea[iMea]].push_back(iMea);
  }
  //std::cout << "UMAP: " << m_uMap << std::endl;
  // Check the measurements
  if ( m_meaVal.size() != nMea() )
    throw std::runtime_error( "#measurement values != #measurement names" );
  // Check the error contributions
  if ( nErr() == 0 )
    throw std::runtime_error( "#error source names == 0" );
  if ( m_errMeaCovs.size() != nErr() )
    throw std::runtime_error( "#error covs mea != #error source names" );
  for ( size_t iErr=0; iErr<nErr(); ++iErr )
    if ( m_errMeaCovs[iErr].size1() != nMea() ||
         m_errMeaCovs[iErr].size2() != nMea() )
      throw std::runtime_error( "One covariance matrix is not #mea by #mea" );
  // Forbid non-alphanumeric characters other than ' ' or '-' in the combination name
  for ( int i=0; combName.c_str()[i]; i++ )
    if ( !isalnum( combName.c_str()[i] ) &&
         combName.c_str()[i] != ' ' &&
         combName.c_str()[i] != '-' )
      throw std::runtime_error( "Invalid combination name '" + combName
                                + "' contains non-alphanumeric characters other than ' ')" );
  // Forbid non-alphanumeric characters other than ' ' in all other names
  for ( size_t iMea=0; iMea<nMea(); ++iMea )
  {
    const std::string& meaName = m_meaNames[iMea];
    for ( int i=0; meaName.c_str()[i]; i++ )
      if ( !isalnum( meaName.c_str()[i] ) &&
           meaName.c_str()[i] != ' ' )
        throw std::runtime_error( "Invalid measurement name '" + meaName
                                  + "' contains non-alphanumeric characters other than ' ')" );
  }
  for ( size_t iObs=0; iObs<nObs(); ++iObs )
  {
    const std::string& obsName = m_obsNames[iObs];
    for ( int i=0; obsName.c_str()[i]; i++ )
      if ( !isalnum( obsName.c_str()[i] ) &&
           obsName.c_str()[i] != ' ' )
        throw std::runtime_error( "Invalid observable name '" + obsName
                                  + "' contains non-alphanumeric characters other than ' ')" );
  }
  for ( size_t iErr=0; iErr<nErr(); ++iErr )
  {
    const std::string& errName = m_errNames[iErr];
    for ( int i=0; errName.c_str()[i]; i++ )
      if ( !isalnum( errName.c_str()[i] ) &&
           errName.c_str()[i] != ' ' )
        throw std::runtime_error( "Invalid error source name '" + errName
                                  + "' contains non-alphanumeric characters other than ' ')" );
  }
  // =============================
  // === Now Compute the BLUEs ===
  // =============================
  // Compute the full input covariance matrix
  // Also check that all input covariances are symmetric!
  // Also compute the off-diagonal covariances for derivative calculations
  // Also check if there are any negative correlations
  m_meaCov = ZeroMatrix( nMea() );
  m_meaCovOffDiag = ZeroMatrix( nMea() );
  for ( size_t iErr=0; iErr<nErr(); ++iErr )
  {
    SymmetricMatrix errMeaCovOffDiag = ZeroMatrix( nMea() );
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
      for ( size_t jMea=0; jMea<iMea; ++jMea )
      {
        if ( m_errMeaCovs[iErr](iMea,jMea) != m_errMeaCovs[iErr](jMea,iMea) )
          throw std::runtime_error( "Covariance matrix is not symmetric?" );
        if ( m_errMeaCovs[iErr](iMea,jMea) < 0 )
          m_hasNegativeMeaCorr = true;
        errMeaCovOffDiag(iMea,jMea) = m_errMeaCovs[iErr](iMea,jMea);
        errMeaCovOffDiag(jMea,iMea) = m_errMeaCovs[iErr](iMea,jMea);
      }
    m_errMeaCovsOffDiag.push_back( errMeaCovOffDiag );
    m_meaCov += m_errMeaCovs[iErr];
    m_meaCovOffDiag += m_errMeaCovsOffDiag[iErr];
  }
  /*
  // For each error source find the measurements with variance>0
  // Compute their reduced covariance and check that it is positive definite
  for ( size_t iErr=0; iErr<nErr(); ++iErr )
  {
    std::set<size_t> errMeaSetPosVar;
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
    {
      if ( m_errMeaCovs[iErr](iMea,iMea) < 0 )
        throw std::runtime_error( "Variance<0 in error source " + errName(iErr) );
      if ( m_errMeaCovs[iErr](iMea,iMea) > 0 )
        errMeaSetPosVar.insert( iMea );
    }
    m_errMeaSetsPosVar.push_back( errMeaSetPosVar );
    SymmetricMatrix errMeaCovPosVar = ZeroMatrix( errMeaSetPosVar.size() );
    for ( size_t iMea=0, iMea2=0; iMea<nMea(); ++iMea )
    {
      if ( errMeaSetPosVar.find(iMea) != errMeaSetPosVar.end() )
      {
        errMeaCovPosVar(iMea2,iMea2) = m_errMeaCovs[iErr](iMea,iMea);
        for ( size_t jMea=0, jMea2=0; jMea<iMea; ++jMea )
        {
          if ( errMeaSetPosVar.find(jMea) != errMeaSetPosVar.end() )
          {
            errMeaCovPosVar(iMea2,jMea2) = m_errMeaCovs[iErr](iMea,jMea);
            errMeaCovPosVar(jMea2,iMea2) = m_errMeaCovs[iErr](iMea,jMea);
            ++jMea2;
          }
        }
        ++iMea2;
      }
    }
    if ( !isPositiveDefinite( errMeaCovPosVar, "covariance for error "+errName(iErr) ) )
    {
      //std::ostream* str = errorStreamForPositiveDefiniteCheck();
      //if ( str )
      //  *str << "WARNING! Reduced covariance matrix for error source " << errName(iErr)
      //       << " is not positive definite " << errMeaCovPosVar << std::endl;
      // TODO: throw an exception if any eigenvalue is < 0?
    }
    m_errMeaCovsPosVar.push_back( errMeaCovPosVar );
  }
  */
  // Check that the full covariance matrix is positive definite
  // (TODO: check that partial covariance matrices have no negative eigenvalues)
  if ( !isPositiveDefinite( m_meaCov, "total covariance" ) )
  {
    std::stringstream msg;
    msg << "Total covariance matrix is not positive definite " << m_meaCov;
    throw CovarianceNotPosDef( msg.str() );
  }
  // Compute the full input correlation matrix
  m_meaCor = ZeroMatrix( nMea(), nMea() );
  for ( size_t iMea=0; iMea<nMea(); ++iMea )
    for ( size_t jMea=0; jMea<nMea(); ++jMea )
      m_meaCor(iMea,jMea) = m_meaCov(iMea,jMea)
        / sqrt( m_meaCov(iMea,iMea) )
        / sqrt( m_meaCov(jMea,jMea) );
  // Invert the full nMea by nMea input covariance matrix
  if ( debug ) std::cout << "MEAS cov " << m_meaCov << std::endl;
  if ( !SymmetricMatrix_invert( m_meaCov, m_meaCovInv ) )
  {
    std::cout << "ERROR! Cannot invert " << m_meaCov << std::endl;
    throw std::runtime_error( "Covariance matrix is singular" );
  }
  if ( debug ) std::cout << "MEAS cov inv " << m_meaCovInv << std::endl;
  // Compute the nObs by nObs information matrix
  m_uMapTrByMeaCovInv = prod( trans( m_uMap ), m_meaCovInv );
  m_blueInf = prod( m_uMapTrByMeaCovInv, m_uMap );
  if ( m_blueInf.size1() != nObs() || m_blueInf.size2() != nObs() )
    throw std::runtime_error( "Information matrix is not #obs by #obs ??" );
  if ( debug ) std::cout << "BLUE inf " << m_blueInf << std::endl;
  // Invert the nObs by nObs information matrix
  if ( !SymmetricMatrix_invert( m_blueInf, m_blueCov ) )
  {
    std::cout << "ERROR! Cannot invert " << m_blueInf << std::endl;
    throw std::runtime_error( "Information matrix is singular ??" );
  }
  if ( debug ) std::cout << "BLUE cov " << m_blueCov << std::endl;
  // Compute the nObs by nObs BLUE correlation matrix
  m_blueCor = ZeroMatrix( nObs(), nObs() );
  for ( size_t aObs=0; aObs<nObs(); ++aObs )
    for ( size_t bObs=0; bObs<nObs(); ++bObs )
      m_blueCor(aObs,bObs) = m_blueCov(aObs,bObs)
        / sqrt( m_blueCov(aObs,aObs) )
        / sqrt( m_blueCov(bObs,bObs) );
  // Compute the nObs by nMea 'central value weights' matrix
  m_blueCvw = prod( m_blueCov, m_uMapTrByMeaCovInv );
  if ( m_blueCvw.size1() != nObs() || m_blueCvw.size2() != nMea() )
    throw std::runtime_error( "CVW matrix is not #obs by #mea ??" );
  // Compute the nObs by nObs 'total CVW' matrix
  // THIS SHOULD BE EQUAL TO THE IDENTITY MATRIX!
  m_blueCvwTot = prod ( m_blueCvw, m_uMap );
  if ( debug ) std::cout << "BLUE cvw tot " << m_blueCvwTot << std::endl;
  IdentityMatrix identityNObs( nObs() );
  if ( debug ) std::cout << "Identity " << identityNObs << std::endl;
  if( !equals( m_blueCvwTot, identityNObs ) )
  {
    std::cout << m_blueCvwTot << " != " << identityNObs << std::endl;
    throw std::runtime_error( "CVW weights are not normalized?" );
  }
  // Compute the nObs by nObs BLUE covariances (one per error source)
  std::vector<SymmetricMatrix>::const_iterator iCov;
  for ( iCov = m_errMeaCovs.begin(); iCov != m_errMeaCovs.end(); ++iCov )
  {
    Matrix cov_by_cvw = prod( *iCov, trans( m_blueCvw ) );
    m_errBlueCovs.push_back( prod( m_blueCvw, cov_by_cvw ) );
  }
  if ( m_errBlueCovs.size() != nErr() )
    throw std::runtime_error( "#error covs blue != #error source names" );
  // Compute the nObs by nObs sum of all nErr BLUE covariances
  // THIS SHOULD BE EQUAL TO THE INVERSE OF THE INFORMATION MATRIX!
  SymmetricMatrix blueCov = ZeroMatrix( nObs() );
  for ( iCov = m_errBlueCovs.begin(); iCov != m_errBlueCovs.end(); ++iCov )
    blueCov += *iCov;
  if ( debug ) std::cout << "BLUE cov? " << blueCov << std::endl;
  if( !equals( blueCov, m_blueCov, true, true ) ) // relax for XSE
  {
    std::cout << blueCov << " != " << m_blueCov << std::endl;
    throw std::runtime_error( "BLUE covariance != information^-1 ?" );
  }
  // Compute the nObs BLUEs
  m_blueVal = prod( m_blueCvw, m_meaVal ); // as an nObs vector
  Vector blues = prod( m_uMap, m_blueVal ); // as an nMea vector
  // Compute the 1 global chi^2 taking into account all correlations
  Vector diffs = m_meaVal - blues;
  m_blueCh2 = inner_prod( diffs, prod( m_meaCovInv, diffs ) );
  // Compute the nMea approximate chi^2's and their sum neglecting all corr
  m_blueApproxMeaCh2NoCorr = ZeroVector( nMea() );
  for ( size_t iMea=0; iMea<nMea(); ++iMea )
  {
    m_blueApproxMeaCh2NoCorr(iMea) = diffs(iMea) * diffs(iMea) / m_meaCov(iMea,iMea);
    m_blueApproxCh2NoCorr += m_blueApproxMeaCh2NoCorr(iMea);
  }
  // Compute the nObs approximate chi^2's neglecting diff obs correlations
  m_blueApproxObsCh2NoDiffObsCorr = ZeroVector( nObs() );
  for ( size_t aObs=0; aObs<nObs(); aObs++ )
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
      if ( m_obsForMea[iMea] == aObs )
        for ( size_t jMea=0; jMea<nMea(); ++jMea )
          if ( m_obsForMea[jMea] == aObs )
            m_blueApproxObsCh2NoDiffObsCorr(aObs) += diffs(iMea) * diffs(jMea) * m_meaCovInv(iMea,jMea);
}

//-----------------------------------------------------------------------------

BlueFish::BlueFish( const BlueFish& rhs )
  : m_combName( rhs.m_combName )
  , m_obsNames( rhs.m_obsNames )
  , m_meaNames( rhs.m_meaNames )
  , m_obsForMea( rhs.m_obsForMea )
  , m_meaForObs( rhs.m_meaForObs )
  , m_uMap( rhs.m_uMap )
  , m_meaVal( rhs.m_meaVal )
  , m_errNames( rhs.m_errNames )
  , m_errMeaCovs( rhs.m_errMeaCovs )
  , m_meaCov( rhs.m_meaCov )
  , m_errMeaCovsOffDiag( rhs.m_errMeaCovsOffDiag )
  , m_meaCovOffDiag( rhs.m_meaCovOffDiag )
    /*
  , m_errMeaSetsPosVar( rhs.m_errMeaSetsPosVar )
  , m_errMeaCovsPosVar( rhs.m_errMeaCovsPosVar )
    */
  , m_meaCor( rhs.m_meaCor )
  , m_meaCovInv( rhs.m_meaCovInv )
  , m_uMapTrByMeaCovInv( rhs.m_uMapTrByMeaCovInv )
  , m_blueInf( rhs.m_blueInf )
  , m_blueCov( rhs.m_blueCov )
  , m_blueCor( rhs.m_blueCor )
  , m_blueCvw( rhs.m_blueCvw )
  , m_blueCvwTot( rhs.m_blueCvwTot )
  , m_errBlueCovs( rhs.m_errBlueCovs )
  , m_blueVal( rhs.m_blueVal )
  , m_blueCh2( rhs.m_blueCh2 )
  , m_blueApproxMeaCh2NoCorr( rhs.m_blueApproxMeaCh2NoCorr )
  , m_blueApproxCh2NoCorr( rhs.m_blueApproxCh2NoCorr )
  , m_blueApproxObsCh2NoDiffObsCorr( rhs.m_blueApproxObsCh2NoDiffObsCorr )
  , m_hasNegativeMeaCorr( rhs.m_hasNegativeMeaCorr )
{
}

//-----------------------------------------------------------------------------

BlueFish&
BlueFish::operator=( const BlueFish& rhs )
{
  m_combName = rhs.m_combName;
  m_obsNames = rhs.m_obsNames;
  m_meaNames = rhs.m_meaNames;
  m_obsForMea = rhs.m_obsForMea;
  m_meaForObs = rhs.m_meaForObs;
  m_uMap = rhs.m_uMap;
  m_meaVal = rhs.m_meaVal;
  m_errNames = rhs.m_errNames;
  m_errMeaCovs = rhs.m_errMeaCovs;
  m_meaCov = rhs.m_meaCov;
  m_errMeaCovsOffDiag = rhs.m_errMeaCovsOffDiag;
  m_meaCovOffDiag = rhs.m_meaCovOffDiag;
  /*
  m_errMeaSetsPosVar = rhs.m_errMeaSetsPosVar;
  m_errMeaCovsPosVar = rhs.m_errMeaCovsPosVar;
  */
  m_meaCor = rhs.m_meaCor;
  m_meaCovInv = rhs.m_meaCovInv;
  m_uMapTrByMeaCovInv = rhs.m_uMapTrByMeaCovInv;
  m_blueInf = rhs.m_blueInf;
  m_blueCov = rhs.m_blueCov;
  m_blueCor = rhs.m_blueCor;
  m_blueCvw = rhs.m_blueCvw;
  m_blueCvwTot = rhs.m_blueCvwTot;
  m_errBlueCovs = rhs.m_errBlueCovs;
  m_blueVal = rhs.m_blueVal;
  m_blueCh2 = rhs.m_blueCh2;
  m_blueApproxMeaCh2NoCorr = rhs.m_blueApproxMeaCh2NoCorr;
  m_blueApproxCh2NoCorr = rhs.m_blueApproxCh2NoCorr;
  m_blueApproxObsCh2NoDiffObsCorr = rhs.m_blueApproxObsCh2NoDiffObsCorr;
  m_hasNegativeMeaCorr = rhs.m_hasNegativeMeaCorr;
  return *this;
}

//-----------------------------------------------------------------------------

BlueFish
BlueFish::changeMeaCovSources( const std::vector<SymmetricMatrix>& errMeaCovs ) const
{
  if ( nObs() == 0 )
    throw std::runtime_error( "This BlueFish is empty" );
  return BlueFish( m_combName, m_obsNames, m_meaNames, m_obsForMea, m_meaVal, m_errNames, errMeaCovs );
}

//-----------------------------------------------------------------------------

BlueFish
BlueFish::keepCorrsForSource( int iErr ) const
{
  if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
  if ( iErr >= (int)nErr() )
    throw std::runtime_error( "iErr >= nErr in removeCorrsForSource" );
  if ( iErr < -1 )
    throw std::runtime_error( "iErr < -1 in removeCorrsForSource" );
  std::vector<SymmetricMatrix> errMeaCovs;
  for ( size_t jErr=0; jErr<nErr(); ++jErr )
  {
    SymmetricMatrix cov = m_errMeaCovs[jErr];
    for ( size_t iMea=0; iMea<nMea(); ++iMea )
      for ( size_t jMea=0; jMea<iMea; ++jMea )
        if ( (int)jErr != iErr )
        {
          cov( jMea, iMea ) = 0;
          cov( iMea, jMea ) = 0;
        }
    errMeaCovs.push_back( cov );
  }
  return changeMeaCovSources( errMeaCovs );
}

//-----------------------------------------------------------------------------

bool BlueFish::operator==( const BlueFish& rhs )
{
  // Check only the inputs to the constructor
  if ( m_combName != rhs.m_combName ) return false;
  if ( m_obsNames != rhs.m_obsNames ) return false;
  if ( m_meaNames != rhs.m_meaNames ) return false;
  if ( m_obsForMea != rhs.m_obsForMea ) return false;
  bool approx = false; // require strict equality
  if ( ! equals( m_meaVal, rhs.m_meaVal, approx ) ) return false;
  if ( m_errNames != rhs.m_errNames ) return false;
  for ( size_t iErr=0; iErr<nErr(); ++iErr )
    if ( ! equals( m_errMeaCovs[iErr], rhs.m_errMeaCovs[iErr], approx ) ) return false;
  // Do not check any derived output
  return true;
}

//-----------------------------------------------------------------------------

