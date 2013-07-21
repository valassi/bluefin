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
#ifndef BLUEFIN_TEXTREPORTER_H
#define BLUEFIN_TEXTREPORTER_H 1

// Include files
#include <iostream>
#include "BlueFin/BlueFish1Obs.h"
#include "BlueFin/InfoMinimizer.h"

namespace bluefin
{
  /** @class TextReporter TextReporter.h
   *
   *  TextReporter is a static class providing tools
   *  for BLUE printouts in text format.
   *
   *  @author Andrea Valassi
   *  @date   2012-2013
   */
  class TextReporter
  {

  public:

    /// Determine the appropriate printout scale factor for a BlueFish
    /// (the square of the scale factor is used for covariance matrices)
    static unsigned getDownscaleFactor( const BlueFish& bf );

    /// Print a matrix (the square of the provided scale factor is used for covariance matrices)
    /// Optionally compare it to that of the reference BlueFish with nominal correlations
    static void printMatrix( const std::string& tx7, // 7 char title for text (caption for latex)
                             const Matrix& mat,
                             const std::vector<std::string>& eleNames,
                             unsigned dscf, // downscale factor
                             std::ostream& ostr = std::cout,
                             bool latex = false,
                             const Matrix* matNomCor = 0 );

    /// Print a BlueFish
    static void printBlueFish( const BlueFish& bf,
                               std::ostream& ostr = std::cout,
                               bool latex = false );


    /// Print information derivatives for a BlueFish1Obs
    /// (normalized by the information and correlations of itself).
    static void printNInfoDerivativesBF1( const BlueFish1Obs& bf,
                                          std::ostream& ostr = std::cout,
                                          bool latex = false,
                                          const std::string caption = "" )
    {
      printNInfoDerivativesBF1( bf, bf, ostr, latex, caption );
    }

    /// Print a summary table
    static void printSummaryTableBF1( const BlueFish1Obs& bf1NomCor,
                                      const std::vector<std::pair<std::string,BlueFish1Obs> >& bf1s,
                                      std::ostream& ostr,
                                      bool latex = false );

    /// Print the results of the pre-minimization derivative analysis.
    /// Print also the results of the minimization unless not required.
    static void printMinimization( const InfoMinimizer& im,
                                   const bool preOnly = false,
                                   std::ostream& ostr = std::cout,
                                   bool latex = false );

    /// Print details about the combination with only posiive CVWs
    static void printPosCvwsBF1( const BlueFish1Obs& bf1PosCvws,
                                 const std::vector< std::pair<BlueFish1Obs,size_t> >& bf1PosCvwsRemoved,
                                 std::ostream& ostr,
                                 bool latex = false );

    /// Print covariances between input measurements in a BlueFish
    /// Optionally compare them to those of the reference BlueFish with nominal correlations
    static void printMeaCovs( const BlueFish& bf,
                              std::ostream& ostr = std::cout,
                              bool latex = false,
                              bool partial = true, // total and all partial, or only total?
                              const BlueFish* pBfNomCor = 0 );

    /// Nominal correlation tag
    static const std::string nominalCorrelationTag()
    {
      return "Nominal correlations";
    }

    /// Zero correlation tag
    static const std::string zeroCorrelationTag()
    {
      return "NO correlations";
    }

  private:

    /// Determine the appropriate description for a printout scale factor
    /// (the square of the scale factor is used for covariance matrices)
    static const std::string getDownscaleFactorTxt( const unsigned dscf, bool cov=false );

    /// Print information derivatives for a BlueFish1Obs
    /// (normalized by the information and correlations of a reference BLUE).
    static void printNInfoDerivativesBF1( const BlueFish1Obs& bf,
                                          const BlueFish1Obs& ref,
                                          std::ostream& ostr = std::cout,
                                          bool latex = false,
                                          const std::string caption = "" );

  private:

    /// Default constructor.
    TextReporter();

    /// Destructor.
    ~TextReporter();

    /// Copy constructor.
    TextReporter( const TextReporter& rhs );

    /// Assignment operator
    TextReporter& operator=( const TextReporter& rhs );

  };
}

#endif // BLUEFIN_TEXTREPORTER_H
