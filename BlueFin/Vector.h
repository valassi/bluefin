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
#ifndef BLUEFIN_VECTOR_H
#define BLUEFIN_VECTOR_H 1

// Include files
#include <cmath>
#include <boost/numeric/ublas/vector.hpp>

namespace bluefin
{
  /// Vector and Matrix precision
  //typedef float Number;
  //typedef double Number;
  typedef long double Number;

  /// Type definitions
  typedef boost::numeric::ublas::vector<Number> Vector;
  typedef boost::numeric::ublas::zero_vector<Number> ZeroVector;

  /// Number equality
  template<typename T>
  inline bool equals( const T& n1, const T& n2, const T& tol, const std::string& stol,
                      const T& thr=0, const std::string& sthr="", bool quiet=false )
  {
    // Try default equality first
    if ( n1 == n2 ) return true;
    // Either number is nan
    if ( std::isnan(n1) || std::isnan(n2) )
    {
      if ( !quiet )
      {
        std::cout << std::endl;
        std::cout << n1 << " != " << n2 << " (NaN detected)" << std::endl;
      }
      return false;
    }
    // One number is 0 and the other is > tolerance and > threshold
    else if ( n1 == 0 )
    {
      if ( std::abs(n2) > tol && std::abs(n2) > thr )
      {
        if ( !quiet )
        {
          std::cout << std::endl;
          std::cout << n1 << " != " << n2 << std::endl;
        }
        return false;
      }
      else return true;
    }
    else if ( n2 == 0 )
    {
      if ( std::abs(n1) > tol && std::abs(n1) > thr )
      {
        if ( !quiet )
        {
          std::cout << std::endl;
          std::cout << n1 << " != " << n2 << std::endl;
        }
        return false;
      }
      else return true;
    }
    // Both numbers are non 0 and their relative difference is > tolerance
    // and one or both numbers are above threshold
    else if ( std::abs(n1) > thr || std::abs(n2) > thr )
    {
      Number maxabs = std::max( std::abs(n1), std::abs(n2) );
      if ( std::abs(n1-n2) / maxabs > tol )
      {
        if ( !quiet )
        {
          std::cout << std::endl;
          std::cout << n1 << " != " << n2
                    << " (difference=" << n1-n2 << ")" << std::endl;
          std::cout << "Relative difference " << (n1-n2) / maxabs
                    << " is > " << stol;
          if ( thr>0 ) std::cout << " and numbers are not both < " << sthr;
          std::cout << std::endl;
        }
        return false;
      }
      else return true;
    }
    else return true;
  }

  /// Number equality
  inline bool equals( const Number& n1, const Number& n2, bool relax=false, bool quiet=false )
  {
    static Number tol1 = 5E-17;
    static std::string stol1 = "5E-17";
    static Number tol2 = 5E-13;
    static std::string stol2 = "5E-13";
    static bool first = true;
    if ( first )
    {
      first = false;
      if ( sizeof(Number) == sizeof(double) )
      {
        tol1 = 5E-15;
        stol1 = "5E-15";
        tol2 = 5E-13;
        stol2 = "5E-13";
      }
      else if ( sizeof(Number) == sizeof(float) )
      {
        tol1 = 5E-6;
        stol1 = "5E-6";
        tol2 = 5E-4;
        stol2 = "5E-4";
      }
    }
    Number tol = ( relax ? tol2 : tol1 );
    std::string stol = ( relax ? stol2 : stol1 );
    return equals( n1, n2, tol, stol, (Number)0, "", quiet );
  }

  /// Vector equality
  inline bool equals( const Vector& v1, const Vector& v2, bool approx=true )
  {
    if ( v1.size() != v2.size() ) return false;
    for ( size_t i=0; i<v1.size(); i++ )
      if ( approx )
      {
        if ( ! equals( v1(i), v2(i) ) ) return false;
      }
      else // strict comparison
      {
        if ( v1(i) != v2(i) ) return false;
      }
    return true;
  }

}

#endif // BLUEFIN_VECTOR_H
