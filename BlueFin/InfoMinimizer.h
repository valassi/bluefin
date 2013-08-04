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
#ifndef BLUEFIN_INFOMINIMIZER_H
#define BLUEFIN_INFOMINIMIZER_H 1

// Include files
#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "BlueFin/BlueFish1Obs.h"

namespace bluefin
{
  /** @class InfoMinimizer InfoMiminizer.h
   *
   *  Class describing a BlueFish1Obs transformation for info minimization.
   *  The class also holds static data and methods for ROOT minimization.
   *
   *  @author Andrea Valassi
   *  @date   2012-2013
   */
  class InfoMinimizer
  {

  public:

    /// The supported functional dependencies
    enum FcnType
      {
        ByGlobalFactorMD=1,     // by global factor (matrix derivatives)
        ByErrorSourceMD=2,      // by error source (matrix derivatives)
        ByOffDiagElemMD=3,      // by off diagonal elements (matrix derivatives)
        ByOffDiagPerErrSrcMD=4  // by off diag per error src (matrix derivatives)
      };

    // Should all parameters be varied?
    static bool varyAllParameters();

    // Threshold for varying parameters (minimum "d1NInfoNomCor" 1/I*dI/dR at 1)
    static Number varParD1NInfoNomCorThreshold()
    {
      return 0.001;
    }

    /// Get the type name for a given type
    static const std::string typeName( FcnType type )
    {
      switch ( type )
      {
      case ByErrorSourceMD:
        return "ByErrorSource";
      case ByOffDiagElemMD:
        return "ByOffDiagElem";
        /*
      case ByOffDiagPerErrSrcMD:
        return "ByOffDiagPerErrSrc";
        */
      case ByGlobalFactorMD:
        return "ByGlobalFactor";
      default:
        throw std::runtime_error( "unsupported type in typeName" );
      }
    }

    /// Get the number of parameters of the problem.
    static size_t nPar( const BlueFish1Obs& bf, FcnType type )
    {
      switch ( type )
      {
      case ByErrorSourceMD:
        return bf.nErr();
      case ByOffDiagElemMD:
        return nOffDiag( bf.meaCov() );
        /*
      case ByOffDiagPerErrSrcMD:
        return bf.nErr() * nOffDiag( bf.meaCov() );
        */
      case ByGlobalFactorMD:
        return 1;
      default:
        throw std::runtime_error( "unsupported type in nPar" );
      }
    }

    /// Get the parameter names of the problem.
    static const std::string parName( size_t iPar, const BlueFish1Obs& bf, FcnType type )
    {
      switch ( type )
      {
      case ByErrorSourceMD:
        return bf.errNames()[iPar];
      case ByOffDiagElemMD:
        {
          std::pair<size_t,size_t> ij = offDiagToIJ( bf.meaCov(), iPar );
          return bf.meaName(ij.first) + "/" + bf.meaName(ij.second);
        }
        /*
      case ByOffDiagPerErrSrcMD:
        {
          std::pair<size_t,size_t> ij = offDiagToIJ( bf.meaCov(), iPar );
          return bf.meaName(ij.first) + "/" + bf.meaName(ij.second)
            + "@" + bf.errNames()[iPar];
        }
        */
      case ByGlobalFactorMD:
        return "GlobalScaleFact";
      default:
        throw std::runtime_error( "unsupported type in parName" );
      }
    }

    //// Destructor
    virtual ~InfoMinimizer();

    /// Set the BlueFish1Obs instance whose information needs to be minimized.
    /// Set the normalization factor for the information function to minimize.
    /// Throw if this is not a single-observable BlueFish1Obs.
    InfoMinimizer( const BlueFish1Obs& bf, Number infoNorm, FcnType type );

    /// Get the BlueFish1Obs instance whose information needs to be minimized.
    const BlueFish1Obs& bf() const
    {
      if ( m_infoNorm == 0 )
        throw std::runtime_error( "InfoMinimizer is not initialized" );
      return m_bf;
    }

    /// Get the normalization factor for the information function to minimize.
    const Number& infoNorm() const
    {
      if ( m_infoNorm == 0 )
        throw std::runtime_error( "InfoMinimizer is not initialized" );
      return m_infoNorm;
    }

    /// Get the type of this InfoMinimizer
    const FcnType& type() const
    {
      if ( m_infoNorm == 0 )
        throw std::runtime_error( "InfoMinimizer is not initialized" );
      return m_type;
    }

    /// Get the number of parameters of the problem.
    size_t nPar() const
    {
      return nPar( m_bf, m_type );
    }

    /// Get the parameter names of the problem.
    const std::string parName( size_t iPar ) const
    {
      return parName( iPar, m_bf, m_type );
    }

    /// Get the parameter IDs (4 char max) of the problem.
    const std::string parID( size_t iPar ) const
    {
      size_t nPar = this->nPar();
      size_t lenMax = 0;
      for ( size_t jPar=0; jPar<nPar; jPar++ )
        lenMax = std::max( lenMax, this->parName( jPar ).size() );
      if ( lenMax <= 4 ) return this->parName( iPar );
      std::stringstream id;
      if ( nPar<=100 ) id << "#" << iPar;
      else if ( nPar<=1000 ) id << iPar;
      else throw std::runtime_error( "more than 999 parameters in parID?" );
      return id.str();
    }

    /// ========================================================================
    /// === Return results (computed on demand) of pre-minimization analysis ===
    /// ========================================================================

    /// Return the normalised info derivatives at nominal correlations (computed on demand).
    virtual const std::vector<Number>& d1NInfosNomCor() const; // size: ALL parameters

    /// Return the normalised info derivatives at zero correlations (computed on demand).
    virtual const std::vector<Number>& d1NInfosZerCor() const; // size: ALL parameters

    /// Return the set of parameters that will be or have been varied during the minimization (computed on demand).
    const std::set<size_t>& parToVary() const; // size: VARIABLE parameters

    /// ========================================================================
    /// === Return results (computed on demand) of parameter minimization    ===
    /// ========================================================================

    /// Minimize correlations in single-obs combination.
    /// Recompute the output and return it in a new BlueFish1Obs instance.
    /// Throws if this is not a single-observable combination
    const BlueFish1Obs& minimizeBF1( std::ostream& tStr = std::cout ) const; // text stream

    /// Return true if a minimum was found.
    bool wasMinimumFound() const;

    /// Return the parameters at minimum-info correlations (computed on demand).
    const std::vector<Number>& parMin() const; // size: ALL parameters

    /// Return the errors on parameters at minimum-info correlations (computed on demand).
    const std::vector<Number>& dparMin() const; // size: ALL parameters

    /// Return the normalised info derivatives at minimum-info correlations (computed on demand).
    const std::vector<Number>& d1NInfosMinCor() const; // size: ALL parameters

    /// Return all sub-minimizers (only for ByOffDiagPerErrSrcMD)
    const std::vector<InfoMinimizer*>& subMinimizers() const;

  protected:

    /// Minimize using ROOT. Throws if minimization failed with an error.
    /// The input runSfs for one par is used as upper(lower) bound in downFromFrom1(upFrom0).
    /// The input runSfsStart is used as starting point; it must be == runSfs if the par is fixed.
    /// Return value: true if a minimum was found, false otherwise.
    bool minimizeUsingROOT( Number& runMinVal,
                            std::vector<Number>& runSfs, // size: ALL par
                            std::vector<Number>& runDSfs, // size: ALL par
                            unsigned& runNCalls,
                            const std::set<size_t>& parsTV, // size: VARIED par
                            const bool downFromNomCor, // alternative: upFromZerCor...
                            const std::vector<Number>& runSfsStart, // size: ALL par
                            std::ostream& tStr = std::cout, // text stream
                            const bool debugROOT = false ) const; // debug ROOT (std::cout only)

    /// Compute the function and all first derivatives.
    /// Input: vector of N scale factors in [0,1]: 0=no correlation, 1=full nominal correlation.
    /// Output: normalised info and vectors of 1st derivatives
    /// with respect to the N scale factors, at the given values.
    /// Info and derivatives are normalized to the original s_bf information.
    void normInfoAndFirstDerivatives( const std::vector<Number>& ddNew, // size: ALL parameters
                                      Number& nInfoNew,
                                      std::vector<Number>& d1NInfoNew ) const; // size: ALL parameters

  private:

    /// Print the results of the pre-minimization derivative analysis.
    /// Print also the results of the minimization unless not required.
    virtual void printMinimization( const bool preOnly = false,
                                    std::ostream& ostr = std::cout,
                                    bool latex = false ) const;

    /// Default constructor is private
    InfoMinimizer();

    /// Copy constructor is private
    InfoMinimizer( const InfoMinimizer& rhs );

    /// Assignment is private
    InfoMinimizer& operator=( const InfoMinimizer& rhs );

  private:

    /// The BlueFish1Obs instance whose information needs to be minimized.
    /// This must be a single-observable BlueFish1Obs.
    BlueFish1Obs m_bf;

    /// The normalization factor for the information function.
    Number m_infoNorm;

    /// The functional dependency to be studied
    FcnType m_type;

  protected:

    /// ========================================================================
    /// === Data members set (on demand) during pre-minimization analysis    ===
    /// ========================================================================

    /// The normalised info derivatives at nominal correlations
    mutable std::vector<Number> m_d1NInfosNomCor; // size: ALL parameters

    /// The normalised info derivatives at zero correlations
    mutable std::vector<Number> m_d1NInfosZerCor; // size: ALL parameters

    /// Has the set of variable parameters been computed?
    mutable bool m_parToVaryExists;

    /// The set of parameters that will be or have been varied during the minimization
    mutable std::set<size_t> m_parToVary; // size: VARIABLE parameters

    /// ========================================================================
    /// === Data members set (on demand) during parameter minimization       ===
    /// ========================================================================

    /// Was minimization attempted and did it fail?
    mutable bool m_minimizationFailed;

    /// Was minimization attempted and was a minimum found?
    mutable bool m_wasMinimumFound;

    /// The parameters at minimum-info correlations.
    mutable std::vector<Number> m_parMin; // size: ALL parameters

    /// The errors on parameters at minimum-info correlations.
    mutable std::vector<Number> m_dparMin; // size: ALL parameters

    /// The normalised info derivatives at minimum-info correlations.
    mutable std::vector<Number> m_d1NInfosMinCor; // size: ALL parameters

    /// The BLUE at minimum-info correlations
    mutable BlueFish1Obs m_minBf;

    /// The sub-minimizers (only for ByOffDiagPerErrSrcMD)
    mutable std::vector<InfoMinimizer*> m_subMinimizers;

  };
}

#endif // BLUEFIN_INFOMINIMIZER_H
