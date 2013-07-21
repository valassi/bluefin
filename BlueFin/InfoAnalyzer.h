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
#ifndef BLUEFIN_INFOANALYZER_H
#define BLUEFIN_INFOANALYZER_H 1

// Include files
#include <iostream>
#include "BlueFin/BlueFish.h"

namespace bluefin
{
  /** @class InfoAnalyzer InfoAnalyzer.h
   *
   *  InfoAnalyzer is a static class providing tools
   *  to analyse Fisher information in BLUE combinations.
   *
   *  @author Andrea Valassi
   *  @date   2012-2013
   */
  class InfoAnalyzer
  {

  public:

    /// Analysis and printout options (covariance)
    enum CovPrintoutOpts { COV_NONE=0, COV_TOTAL, COV_TOTAL_AND_PARTIAL };
        
    /// Analysis and printout options (all)
    typedef struct
    {
      CovPrintoutOpts cov;
    } PrintoutOpts;

    /// Analyse the BLUE combination and print the results to a text stream.
    /// If a file name is given, also print the results there in latex format.
    static void printInfoAnalysis( const BlueFish& bf,
                                   std::ostream& tStr = std::cout, // text stream
                                   const std::string& outputLatexFile = "",
                                   const PrintoutOpts& = (PrintoutOpts){COV_TOTAL_AND_PARTIAL} );

  private:

    /// Default constructor.
    InfoAnalyzer();

    /// Destructor.
    ~InfoAnalyzer();

    /// Copy constructor.
    InfoAnalyzer( const InfoAnalyzer& rhs );

    /// Assignment operator
    InfoAnalyzer& operator=( const InfoAnalyzer& rhs );

  };
}

#endif // BLUEFIN_INFOANALYZER_H
