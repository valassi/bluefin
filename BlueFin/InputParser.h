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
#ifndef BLUEFIN_INPUTPARSER_H
#define BLUEFIN_INPUTPARSER_H 1

// Include files
#include "BlueFin/BlueFish.h"

namespace bluefin
{
  /** @class InputParser InputParser.h
   *
   *  InputParser is a static class providing tools
   *  to parse BlueFin input data files.
   *
   *  @author Andrea Valassi
   *  @date   2012-2013
   */
  class InputParser
  {

  public:

    /// Create a BlueFish from an input data file (xxx.bfin).
    static BlueFish createBlueFishFromInputData( const std::string& inputFileName );

  private:

    /// Default constructor.
    InputParser();

    /// Destructor.
    ~InputParser();

    /// Copy constructor.
    InputParser( const InputParser& rhs );

    /// Assignment operator
    InputParser& operator=( const InputParser& rhs );

  };
}

#endif // BLUEFIN_INPUTPARSER_H
