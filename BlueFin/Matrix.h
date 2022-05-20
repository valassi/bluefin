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
#ifndef BLUEFIN_MATRIX_H
#define BLUEFIN_MATRIX_H 1

// Include files
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include "boost_ublas_matrix_headers.h"
#include "BlueFin/Vector.h"

namespace bluefin
{

  /// Type definitions (basic matrices)
  typedef boost::numeric::ublas::matrix<Number> Matrix;
  typedef boost::numeric::ublas::identity_matrix<Number> IdentityMatrix;
  typedef boost::numeric::ublas::zero_matrix<Number> ZeroMatrix;

  /// Type definitions (symmetric matrices)
  //typedef boost::numeric::ublas::symmetric_matrix<Number,boost::numeric::ublas::lower> SymmetricMatrix; // SLOWER
  typedef Matrix SymmetricMatrix; // FASTER

  /// Type definitions (triangular matrices)
  typedef boost::numeric::ublas::triangular_matrix<Number,boost::numeric::ublas::lower> TriangularMatrix;

  /// Matrix inversion (see http://www.crystalclearsoftware.com/cgi-bin/boost_wiki/wiki.pl?LU_Matrix_Inversion)
  /// Uses lu_factorize and lu_substitute in uBLAS to invert a matrix
  inline bool Matrix_invert( const Matrix& in, Matrix& out )
  {
    // Create a working copy of the input
    Matrix A( in );
    // Create a permutation matrix for the LU-factorization
    boost::numeric::ublas::permutation_matrix<std::size_t> pm( A.size1() );
    // Perform LU-factorization
    int res = lu_factorize( A, pm );
    if ( res != 0 ) return false;
    // Create identity matrix of "inverse"
    out.resize( in.size1(), in.size2(), false );
    out.assign( boost::numeric::ublas::identity_matrix<Number>( A.size1() ) );
    // Back substitute to get the inverse
    lu_substitute( A, pm, out );
    return true;
  }

  /// Symmetric matrix inversion
  inline bool SymmetricMatrix_invert( const SymmetricMatrix& sin, SymmetricMatrix& sout )
  {
    Matrix out;
    if ( !Matrix_invert( sin, out ) ) return false;
    try
    {
      sout = out; // throws "external logic" if out is not symmetric
    }
    catch( std::exception& e )
    {
      std::cout << "ERROR! Assigning Matrix to SymmetricMatrix failed while inverting: " << e.what() << std::endl;
      throw std::runtime_error( "non-diagonal matrix in nOffDiag" );
    }
    return true;
  }

  /// Matrix determinant (see http://www.anderswallin.net/2010/05/matrix-determinant-with-boostublas)
  inline Number determinant( const Matrix& in )
  {
    // Create a working copy of the input
    Matrix A( in );
    // Create a permutation matrix for the LU-factorization
    boost::numeric::ublas::permutation_matrix<std::size_t> pm( A.size1() );
    // Perform LU-factorization
    int res = lu_factorize( A, pm );
    if ( res != 0 ) return 0; // matrix is singular (determinant=0)
    // Compute the determinant sign
    int pm_sign = 1;
    for ( size_t i=0; i<pm.size(); ++i )
      if ( i != pm(i) )
        pm_sign *= -1; // swap_rows would swap a pair of rows, so change sign
    // Compute the determinant
    double det = 1.0;
    for ( size_t i=0; i<A.size1(); i++ )
      det *= A(i,i); // multiply by elements on diagonal
    det = det * pm_sign;
    return det;
  }

  /// Get/set error stream for the positive definiteness check
  inline std::ostream*& errorStreamForPositiveDefiniteCheck()
  {
    static std::ostream* str = 0;
    return str;
  }

  /// Check positive definiteness (see http://en.wikipedia.org/wiki/Sylvester%27s_criterion)
  inline bool isPositiveDefinite( const Matrix& A, const std::string& txt = "" )
  {
    if ( A.size1() != A.size2() )
      throw std::runtime_error( "non-symmetric matrix in isPositiveDefinite" );
    for ( size_t i=1; i<=A.size1(); i++ ) // 1..n (not 0..n-1)
    {
      const boost::numeric::ublas::matrix_range<const Matrix>& sub = boost::numeric::ublas::subrange( A, 0, i, 0, i );
      Number det = determinant( sub );
      // Compare the determinant to the product of diagonal elements
      // (i.e. the determinant in the absence of correlations!)
      Number detNoCor = 1;
      for ( size_t j=0; j<=sub.size1()-1; ++j ) detNoCor*=sub(j,j);
      // Determinant is negative - print a warning if below threshold
      if ( det < 0 )
      {
        std::ostream* str = errorStreamForPositiveDefiniteCheck();
        if ( str )
        {
          if ( detNoCor > 0 && det < -1E-8 * detNoCor )
          {
            char out[80];
            snprintf( out, 12, "%11.8f", (double)(det/detNoCor) );
            *str << "WARNING! Negative determinant " << out
                 << " in correlation matrix"
                 << ( txt != "" ? " ("+txt+")" : txt )
                 << std::endl;
          }
        }
        return false;
      }
      // Determinant is zero - do not print any warning
      else if ( det == 0 )
      {
        return false;
      }
      // Determinant is positive but compatible with zero!
      else if ( det < 1E-8 * detNoCor )
      {
        return false;
      }
    }
    return true;
  }

  /// Matrix equality
  inline bool equals( const Matrix& m1, const Matrix& m2, bool approx=true, bool relax=false )
  {
    if ( m1.size1() != m2.size1() ) return false;
    if ( m1.size2() != m2.size2() ) return false;
    for ( size_t i1=0; i1<m1.size1(); i1++ )
      for ( size_t i2=0; i2<m1.size2(); i2++ )
        if ( approx )
        {
          if ( ! equals( m1(i1,i2), m2(i1,i2), relax ) ) return false;
        }
        else // strict comparison
        {
          if ( m1(i1,i2) != m2(i1,i2) ) return false;
        }
    return true;
  }

  /// Off-diagonal matrix elements
  inline size_t nOffDiag( const Matrix& m )
  {
    if ( m.size1() != m.size2() )
      throw std::runtime_error( "non-diagonal matrix in nOffDiag" );
    return m.size1() * (m.size1()-1) / 2;
  }

  /// Off-diagonal matrix elements
  inline size_t offDiagFromIJ( const Matrix& m, size_t i, size_t j )
  {
    if ( m.size1() != m.size2() )
      throw std::runtime_error( "non-diagonal matrix in offDiagFromIJ" );
    if ( i >= m.size1() )
      throw std::runtime_error( "invalid i in offDiagFromIJ" );
    if ( j >= m.size1() )
      throw std::runtime_error( "invalid j in offDiagFromIJ" );
    if ( i==j )
      throw std::runtime_error( "i==j in offDiagFromIJ" );
    size_t od = 0;
    for ( size_t i2=0; i2<m.size1(); i2++ )
      for ( size_t j2=0; j2<i2; j2++ )
        if ( ( i2==i && j2==j ) || ( i2==j && j2==i ) ) return od;
        else od++;
    throw std::runtime_error( "error in offDiagFromIJ?" );
  }

  /// Off-diagonal matrix elements
  inline std::pair<size_t,size_t> offDiagToIJ( const Matrix& m, size_t od )
  {
    if ( od >= nOffDiag(m) )
      throw std::runtime_error( "invalid od in offDiagToIJ" );
    size_t od2 = 0;
    for ( size_t i=0; i<m.size1(); i++ )
      for ( size_t j=0; j<i; j++ )
#if ( ! defined(__GXX_EXPERIMENTAL_CXX0X__) ) && (__cplusplus < 201103L )
        if ( od2 == od ) return std::make_pair<size_t,size_t>( i, j );
#else
        if ( od2 == od ) return std::make_pair( i, j );
#endif
        else od2++;
    throw std::runtime_error( "error in offDiagToIJ?" );
  }

}

#endif // BLUEFIN_MATRIX_H
