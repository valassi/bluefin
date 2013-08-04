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
// Include files
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include "BlueFin/InputParser.h"

//-----------------------------------------------------------------------------

namespace bluefin
{
  namespace InputParserImpl
  {
    void errorInLine( const std::string& line, size_t iLine )
    {
      std::cerr << "ERROR! Invalid format for line #" << iLine << std::endl;
      std::cerr << line << std::endl;
    }
  }
}

//-----------------------------------------------------------------------------

namespace bluefin
{
  namespace InputParserImpl
  {
    void checkField( const std::string& line,
                     size_t iLine,
                     const std::vector<std::string>& fields,
                     size_t iField,
                     const std::string& token )
    {
      if ( fields[iField] != token )
      {
        errorInLine( line, iLine );
        std::stringstream msg;
        msg << "Invalid format (field #" << iField << " is not '" << token << "')";
        throw std::runtime_error( msg.str() );
      }
    }
  }
}

//-----------------------------------------------------------------------------

namespace bluefin
{
  namespace InputParserImpl
  {
    void checkNFields( const std::string& line,
                       size_t iLine,
                       const std::vector<std::string>& fields,
                       size_t nFields )
    {
      if ( fields.size() != nFields )
      {
        errorInLine( line, iLine );
        std::stringstream msg;
        msg << "Invalid format (#fields is not " << nFields << ")";
        throw std::runtime_error( msg.str() );
      }
    }
  }
}

//-----------------------------------------------------------------------------

using namespace bluefin;

BlueFish InputParser::createBlueFishFromInputData( const std::string& inputFileName )
{
  // Open the file
  std::ifstream ifStream( inputFileName.c_str() );
  if ( !ifStream.is_open() )
  {
    std::stringstream msg;
    msg << "Input data file " << inputFileName << " could not be opened";
    throw std::runtime_error( msg.str() );
  }
  // Define all relevant input variables
  std::string combName;
  size_t nObs = 0;
  size_t nMea = 0;
  size_t nErr = 0;
  std::vector<std::string> obsNames;
  std::vector<std::string> meaNames;
  std::vector<size_t> obsForMea;
  Vector meaVal;
  std::set <std::string> errNamesSet;
  std::vector<std::string> errNames;
  //std::vector<Vector> errMeaVals; // BUG FIX! This converts atof double into long double!
  std::vector<std::vector<double> > errMeaVals;
  std::vector<SymmetricMatrix> errMeaCovs;
  // Read all input lines and set all relevant input variables
  size_t iLine = 0;
  for ( std::string line; std::getline( ifStream, line ); )
  {
    if ( line.find_first_not_of( " " ) != std::string::npos &&
         line.find( "#" ) != 0 )
    {
      // Split line into space-separated fields
      std::vector<std::string> fields;
      std::stringstream lStream( line );
      std::string buffer;
      while ( lStream >> buffer ) fields.push_back( buffer );
      // The next line must have 2 fields: 'TITLE' and the title of the BlueFin
      // combination, which may contain spaces, within double quotes.
      // Example: TITLE "LHC top mass combination 2012"
      if ( iLine == 0 )
      {
        const std::string token = "TITLE";
        InputParserImpl::checkField( line, iLine, fields, 0, token );
        const std::string fullLine = line;
        line = line.substr( token.size() );
        line = line.substr( line.find_first_not_of( " " ) );
        line = line.substr( 0, line.find_last_not_of( " " )+1 );
        if ( line[0] != '\"' || line[line.size()-1] != '\"' )
        {
          InputParserImpl::errorInLine( fullLine, iLine );
          throw std::runtime_error( "Invalid format (title is not within double quotes)" );
        }
        combName = line.substr( 1, line.size()-2 );
        for ( int i=0; combName.c_str()[i]; i++ )
          if ( !isalnum( combName.c_str()[i] ) &&
               combName.c_str()[i] != ' ' &&
               combName.c_str()[i] != '-' )
            throw std::runtime_error( "Invalid format (title contains "
                                      "non-alphanumeric characters other than ' ' or '-')" );
      }
      else if ( iLine == 1 )
      {
        // The next line must have 2 fields: 'NOBS' and the number of observables.
        // Example: NOBS 1
        InputParserImpl::checkField( line, iLine, fields, 0, "NOBS" );
        InputParserImpl::checkNFields( line, iLine, fields, 2 );
        nObs = ::atoi( fields[1].c_str() );
        if ( nObs != ::atof( fields[1].c_str() ) )
        {
          InputParserImpl::errorInLine( line, iLine );
          throw std::runtime_error( "Invalid format (integer field expected)" );
        }
      }
      else if ( iLine == 2 )
      {
        // The next line must have 2 fields: 'NMEA' and the number of measurements.
        // Example: NMEA 7
        InputParserImpl::checkField( line, iLine, fields, 0, "NMEA" );
        InputParserImpl::checkNFields( line, iLine, fields, 2 );
        nMea = ::atoi( fields[1].c_str() );
        if ( nMea != ::atof( fields[1].c_str() ) )
        {
          InputParserImpl::errorInLine( line, iLine );
          throw std::runtime_error( "Invalid format (integer field expected)" );
        }
        meaVal = ZeroVector( nMea );
      }
      else if ( iLine == 3 )
      {
        // The next line must have 2 fields: 'NERR' and the number of error sources.
        // Example: NERR 1
        InputParserImpl::checkField( line, iLine, fields, 0, "NERR" );
        InputParserImpl::checkNFields( line, iLine, fields, 2 );
        nErr = ::atoi( fields[1].c_str() );
        if ( nErr != ::atof( fields[1].c_str() ) )
        {
          InputParserImpl::errorInLine( line, iLine );
          throw std::runtime_error( "Invalid format (integer field expected)" );
        }
      }
      else if ( iLine == 4 )
      {
        // The next line must have NMEA+1 fields in this format:
        // - 'MEANAME' followed by NMEA distinct measurement names
        InputParserImpl::checkField( line, iLine, fields, 0, "MEANAME" );
        InputParserImpl::checkNFields( line, iLine, fields, nMea+1 );
        std::set <std::string> meaNamesSet;
        for ( size_t iMea=0; iMea<nMea; ++iMea )
        {
          const std::string meaName = fields[1+iMea];
          for ( int i=0; meaName.c_str()[i]; i++ )
            if ( !isalnum( meaName.c_str()[i] ) &&
                 meaName.c_str()[i] != ' ' )
            {
              InputParserImpl::errorInLine( line, iLine );
              throw std::runtime_error( "Invalid format (measurement name contains "
                                        "non-alphanumeric characters other than ' ')" );
            }
          if ( meaNamesSet.find( meaName ) != meaNamesSet.end() )
          {
            InputParserImpl::errorInLine( line, iLine );
            throw std::runtime_error( "Invalid format (measurement name appears twice)" );
          }
          meaNamesSet.insert( meaName );
          meaNames.push_back( meaName );
        }
      }
      else if ( iLine == 5 )
      {
        // The next line must have NMEA+1 fields in this format:
        // - 'OBSNAME' followed by NMEA names of the observables measured by the
        //   corresponding measurements (NB: there must have NOBS distinct values)
        InputParserImpl::checkField( line, iLine, fields, 0, "OBSNAME" );
        InputParserImpl::checkNFields( line, iLine, fields, nMea+1 );
        std::map<std::string,size_t> obsNamesMap;
        for ( size_t iMea=0; iMea<nMea; ++iMea )
        {
          const std::string obsName = fields[1+iMea];
          for ( int i=0; obsName.c_str()[i]; i++ )
            if ( !isalnum( obsName.c_str()[i] ) &&
                 obsName.c_str()[i] != ' ' )
            {
              InputParserImpl::errorInLine( line, iLine );
              throw std::runtime_error( "Invalid format (observable name contains "
                                        "non-alphanumeric characters other than ' ')" );
            }
          if ( obsNamesMap.find( obsName ) == obsNamesMap.end() )
          {
            const size_t aObs = obsNamesMap.size(); // compute size _before_ adding one element
            obsNamesMap[obsName] = aObs;
            obsNames.push_back( obsName );
          }
          obsForMea.push_back( obsNamesMap[obsName] );
          //std::cout << "iMea=" << iMea << ", obsForMea=" << obsNamesMap[obsName] << std::endl;
        }
        if ( obsNamesMap.size() != nObs )
        {
          InputParserImpl::errorInLine( line, iLine );
          std::stringstream msg;
          msg << "Invalid format (found " << obsNamesMap.size()
              << " distinct observable names but expected " << nObs << ")";
          throw std::runtime_error( msg.str() );
        }
      }
      else if ( iLine == 6 )
      {
        // The next line must have NMEA+1 fields in this format:
        // - 'MEAVAL' followed by the NMEA measurement central values
        InputParserImpl::checkField( line, iLine, fields, 0, "MEAVAL" );
        InputParserImpl::checkNFields( line, iLine, fields, nMea+1 );
        for ( size_t iMea=0; iMea<nMea; ++iMea )
          meaVal( iMea ) = ::atof( fields[1+iMea].c_str() );
      }
      else if ( iLine > 6 && iLine <= 6+nErr )
      {
        // The next NERR lines must have NMEA+1 fields in this format:
        // - in each line, the error source name followed by the NMEA
        // partial errors for each measurement due to that error source.
        InputParserImpl::checkNFields( line, iLine, fields, nMea+1 );
        const std::string errName = fields[0];
        for ( int i=0; errName.c_str()[i]; i++ )
          if ( !isalnum( errName.c_str()[i] ) &&
               errName.c_str()[i] != ' ' )
          {
            InputParserImpl::errorInLine( line, iLine );
            throw std::runtime_error( "Invalid format (error source name contains "
                                      "non-alphanumeric characters other than ' ')" );
          }
        if ( errNamesSet.find( errName ) != errNamesSet.end() )
        {
          InputParserImpl::errorInLine( line, iLine );
          throw std::runtime_error( "Invalid format (error name appears twice)" );
        }
        errNamesSet.insert( errName );
        errNames.push_back( errName );
        std::vector<double> errMeaVal;
        SymmetricMatrix errMeaCov = ZeroMatrix( nMea, nMea );
        for ( size_t iMea=0; iMea<nMea; ++iMea )
        {
          errMeaVal.push_back( ::atof( fields[1+iMea].c_str() ) );
          errMeaCov(iMea,iMea) = pow( errMeaVal[iMea], 2 );
        }
        errMeaVals.push_back( errMeaVal );
        errMeaCovs.push_back( errMeaCov );
      }
      else if ( iLine == 7+nErr )
      {
        // The next line must have NERR+2 fields in this format:
        // - 'CMEA1' 'CMEA2' (correlations between 2 measurements)
        // followed by NERR error source names in the order above.
        InputParserImpl::checkNFields( line, iLine, fields, nErr+2 );
        InputParserImpl::checkField( line, iLine, fields, 0, "CMEA1" );
        InputParserImpl::checkField( line, iLine, fields, 1, "CMEA2" );
        for ( size_t iErr=0; iErr<nErr; ++iErr )
          InputParserImpl::checkField( line, iLine, fields, iErr+2, errNames[iErr] );
      }
      else if ( iLine > 7+nErr && iLine <= (7+nErr) + (nMea*(nMea-1))/2 )
      {
        // The next NMEA*(NMEA-1)/2 lines must have NERR+2 fields:
        // the names of two distinct measurements followed by the
        // NERR correlations between the partial errors on the two
        // measurements  due to the corresponding error source.
        // Measurements must appear the same order above.
        InputParserImpl::checkNFields( line, iLine, fields, nErr+2 );
        const size_t iPair = iLine - (7+nErr) - 1; // 0 to NMEA*(NMEA-1)/2-1
        size_t iMea1 = 0;
        size_t iMea2 = 1;
        for ( size_t jPair=0; jPair<iPair; )
        {
          //std::cout << "jPair=" << jPair << ",iPair=" << iPair << std::endl;
          if ( ++iMea2 == nMea ) iMea2 = ++iMea1 + 1;
          if ( iMea1 == nMea ) // this should never happen by design
            throw std::runtime_error( "INTERNAL ERROR! iMea1==nMea?" );
          jPair++;
        }
        //std::cout << "OD: " << iMea1 << "," << iMea2 << std::endl;
        InputParserImpl::checkField( line, iLine, fields, 0, meaNames[iMea1] );
        InputParserImpl::checkField( line, iLine, fields, 1, meaNames[iMea2] );
        for ( size_t iErr=0; iErr<nErr; ++iErr )
        {
          const std::vector<double>& errMeaVal = errMeaVals[iErr];
          SymmetricMatrix& errMeaCov = errMeaCovs[iErr];
          double corr = atof( fields[2+iErr].c_str() );
          if ( corr<-1 || 1<corr )
          {
            InputParserImpl::errorInLine( line, iLine );
            throw std::runtime_error( "Invalid format (correlation <-1 or >1)" );
          }
          errMeaCov(iMea1,iMea2) = errMeaVal[iMea1] * errMeaVal[iMea2] * corr;
          errMeaCov(iMea2,iMea1) = errMeaCov(iMea1,iMea2);
        }
      }
      else
      {
        InputParserImpl::errorInLine( line, iLine );
        throw std::runtime_error( "Invalid format (too many lines)" );
      }
      // Increment counter of valid lines
      iLine++;
    }
  }
  // Close the file
  ifStream.close();
  // Create and return the BlueFish
  const bool debug = false;
  return BlueFish( combName, obsNames, meaNames, obsForMea, meaVal, errNames, errMeaCovs, debug );
}

//-----------------------------------------------------------------------------
