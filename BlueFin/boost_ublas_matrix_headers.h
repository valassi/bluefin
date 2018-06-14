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
#ifndef BLUEFIN_BOOSTUBLASMATRIXHEADERS_H
#define BLUEFIN_BOOSTUBLASMATRIXHEADERS_H 1

// Disable warnings triggered by Boost headers
#if defined __GNUC__
#pragma GCC system_header
#endif

// Include files
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/symmetric.hpp>
#include <boost/numeric/ublas/triangular.hpp>

#endif // BLUEFIN_BOOSTUBLASMATRIXHEADERS_H
