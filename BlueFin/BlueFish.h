//============================================================================
//
// Copyright 2012, 2013 CERN
//
// This file is part of the BlueFin software. BlueFin is distributed under
// the terms of the GNU General Public License version 3 (GPL Version 3),
// copied verbatim in file "COPYING". In applying this license, CERN does not
// waive the privileges and immunities granted to it by virtue of its status
// as an Intergovernmental Organization or submit itself to any jurisdiction.
//
// Original author of the software:
//   * Andrea Valassi (CERN) 2012, 2013
//
// Based on ideas (http://arxiv.org/abs/1307.4003) by:
//   * Andrea Valassi (CERN)
//   * Roberto Chierici (Univ. Lyon)
//
//============================================================================
#ifndef BLUEFIN_BLUEFISH_H
#define BLUEFIN_BLUEFISH_H 1

// Include files
//#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "BlueFin/Matrix.h"

namespace bluefin
{
  /** @class BlueFish BlueFish.h
   *
   *  One BlueFish instance represents one generic BLUE combination problem.
   *  Any number of measurements may exist of each of several observables.
   *
   *  This is the main class of the BlueFin software
   *  (Best Linear Unbiased Estimate Fisher INformation analySis).
   *
   *  @author Andrea Valassi
   *  @date   2012-2013
   */
  class BlueFish
  {

  public:

    /// Destructor
    virtual ~BlueFish();

    /// Constructor. Set all inputs and compute the output.
    /// Throws CovarianceNotPosDef if a covariance is not positive definite.
    BlueFish( const std::string& combName,
              const std::vector<std::string>& obsNames,
              const std::vector<std::string>& meaNames,
              const std::vector<size_t>& obsForMea,
              const Vector& meaVal,
              const std::vector<std::string>& errNames,
              const std::vector<SymmetricMatrix>& errMeaCovs,
              bool debug=false );

    /// Copy constructor.
    BlueFish( const BlueFish& rhs );

    /// Assignment operator
    BlueFish& operator=( const BlueFish& rhs );

    /// Return the number of measurements
    size_t nMea() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_meaNames.size();
    }

    /// Return the number of observables
    size_t nObs() const
    {
      // This is the only method that does not throw if the BlueFish is empty
      return m_obsNames.size();
    }

    /// Return the name of this combination
    const std::string& combName() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_combName;
    }

    /// Return the names of the nMea measurements
    const std::vector<std::string>& meaNames() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_meaNames;
    }

    /// Return the names of the nMea measurements ('Roman' indices i,j,k...).
    const std::string& meaName( size_t iMea ) const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_meaNames[iMea]; // throws if index invalid
    }

    /// Return the names of the nObs observables
    const std::vector<std::string>& obsNames() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_obsNames;
    }

    /// Return the names of the nObs observables ('Greek' indices a,b,c...).
    const std::string& obsName( size_t aObs ) const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_obsNames[aObs]; // throws if index invalid
    }

    /// Return the observable# for a given measurement#
    size_t obsForMea( size_t iMea ) const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_obsForMea[iMea];
    }

    /// Return the list of measurement#s for a given observable#
    const std::vector<size_t>& meaForObs( size_t aObs ) const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_meaForObs[aObs];
    }

    /// Return the nMea by nObs mapping matrix U
    const Matrix& uMap() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_uMap;
    }

    /// Return the number of error contributions
    size_t nErr() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_errNames.size();
    }

    /// Return the names of the nErr error sources.
    const std::vector<std::string>& errNames() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_errNames;
    }

    /// Return the names of the nErr error source.
    const std::string& errName( size_t iErr ) const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_errNames[iErr]; // throws if index invalid
    }

    /// Return the nMea measured values
    const Vector& meaVal() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_meaVal;
    }

    /// Return the nErr (nMea by nMea) measurement covs (one per error source)
    const std::vector<SymmetricMatrix>& meaCovSources() const // HERE WAS A NASTY BUG (FORGOTTEN &)
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_errMeaCovs;
    }

    /// Return the nMea by nMea measurement full covariance matrix
    const SymmetricMatrix& meaCov() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_meaCov;
    }

    /// Return the nErr (nMea by nMea) measurement off-diagonal covs (one per error source)
    const std::vector<SymmetricMatrix>& meaCovOffDiagSources() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_errMeaCovsOffDiag;
    }

    /// Return the nMea by nMea measurement full off-diagonal covariance matrix
    const SymmetricMatrix& meaCovOffDiag() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_meaCovOffDiag;
    }

    /// Return the nMea by nMea measurement full correlation matrix
    const SymmetricMatrix& meaCor() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_meaCor;
    }

    /// Return the nMea by nMea inverse of the measurement full covariance matrix
    const SymmetricMatrix& meaCovInv() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_meaCovInv;
    }

    /// Return the nObs by nMea product of uMapTr by the inverse measurement covariance
    const Matrix& uMapTrByMeaCovInv() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_uMapTrByMeaCovInv;
    }

    /// Return the nObs by nObs BLUE information matrix
    const SymmetricMatrix& blueInf() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_blueInf;
    }

    /// Return the nObs by nObs BLUE full covariance matrix (== inverse of info)
    const SymmetricMatrix& blueCov() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_blueCov;
    }

    /// Return the nObs by nObs BLUE correlation matrix
    const SymmetricMatrix& blueCor() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_blueCor;
    }

    /// Return the nObs by nMea BLUE central value weight matrix
    const Matrix& blueCvw() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_blueCvw;
    }

    /// Return the nObs by nObs BLUE central value weight total (== identity)
    const SymmetricMatrix& blueCvwTot() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_blueCvwTot;
    }

    /// Return the nErr (nMea by nMea) BLUE covariances (one per error source)
    const std::vector<SymmetricMatrix>& blueCovSources() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_errBlueCovs;
    }

    /// Return the nObs BLUE central values
    const Vector& blueVal() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_blueVal;
    }

    /// Return the BLUE global chi2 with correlations
    const Number& blueCh2() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_blueCh2;
    }

    /// Return the number of degrees of freedom for the BLUE global chi2 with correlations
    size_t blueCh2Ndof() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return nMea() - nObs();
    }

    /// Return the approx chi2's for the nMea measurements without all corr
    /// (FOR BACKWARD COMPATIBILITY TO OLD LEP2 SOFTWARE ONLY)
    const Vector& blueApproxMeaCh2NoCorr() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_blueApproxMeaCh2NoCorr;
    }

    /// Return the approx global chi2 without all correlations
    /// (FOR BACKWARD COMPATIBILITY TO OLD LEP2 SOFTWARE ONLY)
    const Number& blueApproxCh2NoCorr() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_blueApproxCh2NoCorr;
    }

    /// Return the approx chi2's for the nObs combinations without diff obs corr
    /// (FOR BACKWARD COMPATIBILITY TO OLD LEP2 SOFTWARE ONLY)
    const Vector& blueApproxObsCh2NoDiffObsCorr() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_blueApproxObsCh2NoDiffObsCorr;
    }

    /// Are there negative correlations in the full or any partial covariance matrix?
    bool hasNegativeMeaCorr() const
    {
      if ( nObs() == 0 ) throw std::runtime_error( "This BlueFish is empty" );
      return m_hasNegativeMeaCorr;
    }

    /// Change only the measurement covariances for all error sources.
    /// Recompute the output and return it in a new BlueFish instance.
    BlueFish changeMeaCovSources( const std::vector<SymmetricMatrix>& meaCovSources ) const;

    /// Keep correlations only for a given error source (0..nErr-1) or none (-1).
    /// Recompute the output and return it in a new BlueFish instance.
    BlueFish keepCorrsForSource( int iErr ) const;

  public:

    /// The delta for derivatives. Note that a negative delta is used,
    /// hence the lower limit for all variables is 2*delta (not 0!).
    static const Number Delta = 0.0001;

  public:

    class CovarianceNotPosDef: public std::runtime_error
    {
    public:
      virtual ~CovarianceNotPosDef() throw () {}
      CovarianceNotPosDef()
        : std::runtime_error( "Covariance is not positive definite" ) {}
      CovarianceNotPosDef( const std::string& msg ) 
        : std::runtime_error( "Covariance is not positive definite (" + msg + ")" ) {}
    };

  protected:

    /// Default constructor. Creates an empty invalid BlueFish.
    BlueFish();

    /// Comparison operator
    bool operator==( const BlueFish& rhs );

    /// Comparison operator
    bool operator!=( const BlueFish& rhs )
    {
      return ( ! this->operator==( rhs ) );
    }

  protected:

    /// ============================================
    /// === Data members set in validating input ===
    /// ============================================

    /// The name of this combination
    std::string m_combName;

    /// The names of the nObs observables
    std::vector<std::string> m_obsNames;

    /// The names of the nMea measurements
    std::vector<std::string> m_meaNames;

    /// The nMea mapping vector
    std::vector<size_t> m_obsForMea;

    /// The lists of measurement#s for a given observable#
    std::vector< std::vector<size_t> > m_meaForObs;

    /// The nMea by nObs mapping matrix U
    Matrix m_uMap;

    /// The nMea measured values
    Vector m_meaVal;

    /// The names of the nErr error sources
    std::vector<std::string> m_errNames;

    /// The nErr (nMea by nMea) measurement covariances (one per error source)
    std::vector<SymmetricMatrix> m_errMeaCovs;

    /// ============================================
    /// === Data members set in computing BLUEs  ===
    /// ============================================

    /// The nMea by nMea measurement full covariance matrix
    SymmetricMatrix m_meaCov;

    /// The nErr (nMea by nMea) off-diagonal measurement covariances (one per error source)
    /// These are used to compute matrix derivatives for the information splitup
    std::vector<SymmetricMatrix> m_errMeaCovsOffDiag;

    /// The nMea by nMea off-diagonal measurement full covariance matrix
    /// This is used to compute matrix derivatives for the information splitup
    SymmetricMatrix m_meaCovOffDiag;

    /// The nErr sets of (0...nMea) measurements with non-zero variances (one per error source)
    //std::vector<std::set<size_t> > m_errMeaSetsPosVar;

    /// The nErr reduced covariances for measurements with non-zero variances (one per error source)
    //std::vector<SymmetricMatrix> m_errMeaCovsPosVar;

    /// The nMea by nMea measurement full correlation matrix
    SymmetricMatrix m_meaCor;

    /// The nMea by nMea inverse of the measurement full covariance matrix
    SymmetricMatrix m_meaCovInv;

    /// The nObs by nMea product of uMapTr by the inverse measurement covariance
    Matrix m_uMapTrByMeaCovInv;

    /// The nObs by nObs BLUE information matrix
    SymmetricMatrix m_blueInf;

    /// The nObs by nObs BLUE full covariance matrix (== inverse of info)
    SymmetricMatrix m_blueCov;

    /// The nObs by nObs BLUE correlation matrix
    SymmetricMatrix m_blueCor;

    /// The nObs by nMea BLUE central value weight matrix
    Matrix m_blueCvw;

    /// The nObs by nObs BLUE central value weight total (== identity)
    SymmetricMatrix m_blueCvwTot;

    /// The nErr (nMea by nMea) BLUE covariances (one per error source)
    std::vector<SymmetricMatrix> m_errBlueCovs;

    /// The nObs BLUE central values
    Vector m_blueVal;

    /// The BLUE global chi2 with correlations
    Number m_blueCh2;

    /// The approximate chi2's for the nMea measurements without all correlations
    Vector m_blueApproxMeaCh2NoCorr; // formerly 'ch2s'

    /// The approximate global chi2 without all correlations
    Number m_blueApproxCh2NoCorr; // formerly 'ch2f'

    /// The approximate chi2's for the nObs combinations without diff obs corr
    Vector m_blueApproxObsCh2NoDiffObsCorr; // formerly 'ch2e'

    /// Are there negative correlations in the full or any partial covariance matrix?
    bool m_hasNegativeMeaCorr;

  };

}

#endif // BLUEFIN_BLUEFISH_H
