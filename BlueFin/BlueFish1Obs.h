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
#ifndef BLUEFIN_BLUEFISH1OBS_H
#define BLUEFIN_BLUEFISH1OBS_H 1

// Include files
#include <iostream>
#include <map>
#include <vector>
#include "BlueFin/BlueFish.h"

namespace bluefin
{
  /** @class BlueFish1Obs BlueFish1Obs.h
   *
   *  One BlueFish1Obs instance represents a BLUE combination problem
   *  for any number of measurements of one single observable.
   *
   *  @author Andrea Valassi
   *  @date   2012-2013
   */
  class BlueFish1Obs : public BlueFish
  {

  public:

    /// Destructor
    virtual ~BlueFish1Obs();

    /// Constructor. Set all inputs and compute the output.
    /// Throws CovarianceNotPosDef if a covariance is not positive definite.
    BlueFish1Obs( const std::string& combName,
                  const std::string& obsName,
                  const std::vector<std::string>& meaNames,
                  const Vector& meaVal,
                  const std::vector<std::string>& errNames,
                  const std::vector<SymmetricMatrix>& errMeaCovs,
                  bool debug = false );

    /// Constructor from a BlueFish.
    /// Throws if this is not a single-observable combination.
    BlueFish1Obs( const BlueFish& bf );

    /// Default constructor. Creates an empty invalid BlueFish1Obs.
    BlueFish1Obs();

    /// Copy constructor.
    BlueFish1Obs( const BlueFish1Obs& rhs );

    /// Assignment operator
    BlueFish1Obs& operator=( const BlueFish1Obs& rhs );

    /// "Onionize" correlations by error source in single-obs combination
    /// You can try out two recipes:
    /// 1. Onionization a la AV: within each error source,
    /// limit the off diagonal elements of the partial covariance_ij
    /// to the smallest of the _partial_ variances of measurements i and j.
    /// 2. Alternative onionization a la RC: within each error source,
    /// limit the off diagonal elements of the partial covariance_ij
    /// to the smallest of the _full_ variances of measurements i and j.
    /// [Alternative: onionize the summed covariance with all sources summed]
    /// [Alternative: onionize the full covariance including diff obs meas]
    /// Recompute the output and return it in a new BlueFish1Obs instance.
    /// Throws if this is not a single-observable combination
    /// Throws if there are any negative correlations
    /// [More traditional recipes for onionizing blue fish are described for instance
    /// by Goldoni et al: http://www.cucinaveneziana.net/2009/01/09/la-sarde-in-saor]
    BlueFish1Obs onionizeCorrsByErrorSourceBF1( bool rc, std::ostream* ptStr = 0 ) const;

    /// Rescale corrs for all error sources in a single-obs combination.
    /// Recompute the output and return it in a new BlueFish1Obs instance.
    /// Throws if this is not a single-observable combination
    BlueFish1Obs rescaleCorrsByErrorSourceBF1( const std::vector<Number>& errCorrSFs ) const;

    /// Compute information of a single-obs combination after rescaling corrs for all error sources.
    /// [One could also use rescaleByErrorSourceBF1 but this is 1.5x faster].
    /// Throws if this is not a single-observable combination.
    Number infoRescaledCorrsByErrorSourceBF1( const std::vector<Number>& errCorrSFs ) const;

    /// Rescale off-diagonal elements in a single-obs combination.
    /// Recompute the output and return it in a new BlueFish1Obs instance.
    /// Throws if this is not a single-observable combination
    BlueFish1Obs rescaleCorrsByOffDiagElemBF1( const std::vector<Number>& offDiagSFs ) const;

    /// Rescale all corrs in a single-obs combination.
    /// Recompute the output and return it in a new BlueFish1Obs instance.
    /// Throws if this is not a single-observable combination
    BlueFish1Obs rescaleCorrsByGlobalFactorBF1( Number sf ) const;

    /// Remove measurements with CVW <= 0 from a single-observable combination.
    /// The process is iterative: remove the most negative CVW until all CVW are > 0.
    /// [NB The output has a different number of measurements!]
    /// Throws if this is not a single-observable combination.
    BlueFish1Obs removeMeasWithNegativeCvwInBF1( std::vector< std::pair<BlueFish1Obs,size_t> >& removed ) const;

    /// Check if this BF is compatible to a reference.
    /// The reference must have larger or the same off-diag covariances.
    /// Throws if this is not a single-observable combination.
    /// Throws if either BF has negative correlations.
    bool isSmallerBF1( const BlueFish1Obs& ref ) const;

    /// Return the normalised BLUE info first derivatives (computed on demand).
    /// Derivatives are for every error source wrt each off-diagonal pair.
    /// Set a reference BlueFish1Obs to compute derivatives.
    /// The reference must have larger or the same off-diag covariances.
    /// Throws if a _different_ reference has already been set.
    /// Throws if this or the reference are not single-observable combinations.
    const std::vector<TriangularMatrix>& blueD1NormInfByOffDiagElemByErrorSourceBF1( const BlueFish1Obs& ref ) const;

    /// Return the normalised BLUE info first derivatives (computed on demand).
    /// Derivatives are wrt each off-diagonal pair, summed on error sources.
    /// Set a reference BlueFish1Obs to compute derivatives.
    /// The reference must have larger or the same off-diag covariances.
    /// Throws if a _different_ reference has already been set.
    /// Throws if this or the reference are not single-observable combinations.
    const TriangularMatrix& blueD1NormInfByOffDiagElemBF1( const BlueFish1Obs& ref ) const;

    /// Return the normalised BLUE info first derivatives (computed on demand).
    /// Derivatives are wrt each error source, summed on off-diagonal pairs.
    /// Set a reference BlueFish1Obs to compute derivatives.
    /// The reference must have larger or the same off-diag covariances.
    /// Throws if a _different_ reference has already been set.
    /// Throws if this or the reference are not single-observable combinations.
    const std::vector<Number>& blueD1NormInfByErrorSourceBF1( const BlueFish1Obs& ref ) const;

    /// Return the normalised BLUE info first derivatives (computed on demand).
    /// Derivative is wrt a global factor: it is checked internally that this
    /// is the sum of derivatives by error source and off-diagonal pairs.
    /// Set a reference BlueFish1Obs to compute derivatives.
    /// The reference must have larger or the same off-diag covariances.
    /// Throws if a _different_ reference has already been set.
    /// Throws if this or the reference are not single-observable combinations.
    const Number& blueD1NormInfByGlobalFactorBF1( const BlueFish1Obs& ref ) const;

    /// Return (computed on demand) the nObs by nMea BLUE intrinsic information weight matrix
    const Matrix& blueFiw() const;

    /// Return (computed on demand) the nObs by nMea BLUE intrinsic information weight tot (!= identity!)
    const SymmetricMatrix& blueFiwTot() const;

    /// Return (computed on demand) the nObs by nMea BLUE marginal information weight matrix
    const Matrix& blueMiw() const;

    /// Return (computed on demand) the nObs by nMea BLUE marginal information weight tot (!= identity!)
    const SymmetricMatrix& blueMiwTot() const;

    /// Return (computed on demand) the nObs by nMea BLUE relative importance weight matrix
    const Matrix& blueRiw() const;

    /// Return (computed on demand) the nObs by nMea BLUE relative importance weight tot (== identity!)
    const SymmetricMatrix& blueRiwTot() const;

  private:

    /// Remove a measurement from a single-observable combination.
    /// [NB The output has a different number of measurements!]
    /// Throws if this is not a single-observable combination.
    BlueFish1Obs removeOneMeasInBF1( size_t iMeas ) const;

  protected:

    /// Set a reference BlueFish1Obs to compute derivatives.
    /// The reference must have larger or the same off-diag covariances.
    /// Throws if a _different_ reference has already been set.
    /// Throws if this is not a single-observable combination.
    void setDInfosRefBF1( const BlueFish1Obs& ref ) const;

  protected:

    /// ========================================================================
    /// === Data members set (on demand) in computing BLUE info derivatives  ===
    /// ========================================================================

    /// The BlueFish1Obs reference for derivative calculation.
    /// This can be set only ONCE by the computeBlueDInfosBF1 method.
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
    mutable std::auto_ptr<BlueFish1Obs> m_blueDInfosRef;
#else
    mutable std::unique_ptr<BlueFish1Obs> m_blueDInfosRef;
#endif

    /// The nErr (nMea by nMea) normalised BLUE info first derivatives (computed on demand).
    /// Derivatives are for every error source wrt each off-diagonal pair.
    /// Normalised derivatives have been divided by the info of the reference BLUE.
    /// The matrices are triangular (nMea*(nMea-1)/2 terms) and assume i<j.
    /// They already include a factor 2 to account for proper normalization.
    mutable std::vector<TriangularMatrix> m_blueD1NormInfByOffDiagElemByErrorSourceBF1;

    /// The (nMea by nMea) normalised BLUE info first derivatives (computed on demand).
    /// Derivatives are wrt each off-diagonal pair, summed on error sources.
    /// Normalised derivatives have been divided by the info of the reference BLUE.
    mutable TriangularMatrix m_blueD1NormInfByOffDiagElemBF1;

    /// The nErr normalised BLUE info first derivatives (computed on demand).
    /// Derivatives are wrt each error source, summed on off-diagonal pairs.
    /// Normalised derivatives have been divided by the info of the reference BLUE.
    mutable std::vector<Number> m_blueD1NormInfByErrorSourceBF1;

    /// The normalised BLUE info first derivative (computed on demand).
    /// Derivative is wrt a global factor.
    /// Normalised derivatives have been divided by the info of the reference BLUE.
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
    mutable std::auto_ptr<Number> m_blueD1NormInfByGlobalFactorBF1;
#else
    mutable std::unique_ptr<Number> m_blueD1NormInfByGlobalFactorBF1;
#endif

    /// ========================================================================
    /// === Data members set (on demand) in computing several "weights"      ===
    /// ========================================================================

    /// The nObs (==1) by nMea BLUE Fisher intrinsic information weight matrix
    mutable Matrix m_blueFiw;

    /// The nObs (==1) by nObs BLUE Fisher intrinsic information weight total (!= identity!)
    mutable SymmetricMatrix m_blueFiwTot;

    /// The nObs (==1) by nMea BLUE Fisher marginal information weight matrix
    mutable Matrix m_blueMiw;

    /// The nObs (==1) by nObs BLUE Fisher marginal information weight total (!= identity!)
    mutable SymmetricMatrix m_blueMiwTot;

    /// The nObs (==1) by nMea BLUE relative importance weight matrix
    mutable Matrix m_blueRiw;

    /// The nObs (==1) by nObs BLUE relative importance weight total (== identity)
    mutable SymmetricMatrix m_blueRiwTot;

  };
}

#endif // BLUEFIN_BLUEFISH1OBS_H
