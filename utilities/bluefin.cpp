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
// Include files
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include "BlueFin/BlueFish.h"
#include "BlueFin/InfoAnalyzer.h"
#include "BlueFin/InputParser.h"
#include "BlueFin/VersionInfo.h"

// External binary data from BlueFinLogo
extern char _binary_BlueFinLogo_jpg_start;
extern char _binary_BlueFinLogo_jpg_end;

// External binary data from BlueFinReport
extern char _binary_BlueFinReport_tex_start;
extern char _binary_BlueFinReport_tex_end;

//-----------------------------------------------------------------------------

namespace bluefin
{
  void usage( const char* argv0, std::ostream& out = std::cout )
  {
    out << "Usage: " << argv0 << " [options] [<indir>/]<infile>.bfin\n"
        << "  Available options:\n"
        << "    -h           - print help message and exit\n"
        << "    -q           - quiet latex (omit latex logs from bluefin logs)\n"
        << "    -o <outdir>  - store/overwrite <infile>.pdf and <infile>_bluefin.log in <outdir>\n"
        << "                   [default: use the input directory <indir>]\n"
        << "    -t <texdir>  - store/overwrite tex and other temporary files in <texdir>\n"
        << "                   [default: use a temporary directory]\n"
        << "    -c (0..2)    - covariance printout: 0=none, 1=full, 2=all (full and partial)\n"
        << "                   [default: 2=all]\n"
        << "    -M (0..4)    - minimizations: 0=none, 1+=ByGlobFac, 2+=ByErrSrc, 3+=ByOffDiag, 4+=ByOffDiagPerErrSrc\n"
        << "                   [default: 3=all but the experimental ByOffDiagPerErrSrc]\n"
      ;
  }
}

//-----------------------------------------------------------------------------

int main( int argc, char** argv )
{
  using namespace bluefin;
  try
  {
    // Parse the command line options
    bool quietLatex = false;
    std::string texFileDir;
    std::string outFileDir;
    InfoAnalyzer::CovPrintoutOpts covPrintoutOpts = InfoAnalyzer::COV_TOTAL_AND_PARTIAL;
    InfoAnalyzer::MinimizationOpts minimizationOpts = InfoAnalyzer::MIN_ADD_BYOD;
    {
      int c;
      while ( ( c = ::getopt( argc, argv, "hqo:t:c:M:" ) ) != -1 )
      {
        switch ( c )
        {
        case 'h':
          usage( argv[0], std::cout );
          return 0;
        case 'q':
          quietLatex = true;
          break;
        case 't':
          texFileDir = ::optarg;
          break;
        case 'o':
          outFileDir = ::optarg;
          break;
        case 'c':
          if ( ::strcmp ( ::optarg, "0" ) == 0 )
            covPrintoutOpts = InfoAnalyzer::COV_NONE;
          else if ( ::strcmp ( ::optarg, "1" ) == 0 )
            covPrintoutOpts = InfoAnalyzer::COV_TOTAL;
          else if ( ::strcmp ( ::optarg, "2" ) == 0 )
            covPrintoutOpts = InfoAnalyzer::COV_TOTAL_AND_PARTIAL;
          else
          {
            usage( argv[0], std::cerr );
            return 1;
          }
          break;
        case 'M':
          if ( ::strcmp ( ::optarg, "0" ) == 0 )
            minimizationOpts = InfoAnalyzer::MIN_NONE;
          else if ( ::strcmp ( ::optarg, "1" ) == 0 )
            minimizationOpts = InfoAnalyzer::MIN_ADD_BYGF;
          else if ( ::strcmp ( ::optarg, "2" ) == 0 )
            minimizationOpts = InfoAnalyzer::MIN_ADD_BYES;
          else if ( ::strcmp ( ::optarg, "3" ) == 0 )
            minimizationOpts = InfoAnalyzer::MIN_ADD_BYOD;
          else if ( ::strcmp ( ::optarg, "4" ) == 0 )
            minimizationOpts = InfoAnalyzer::MIN_ADD_BYOE;
          else
          {
            usage( argv[0], std::cerr );
            return 1;
          }
          break;
        default:
          usage( argv[0], std::cerr );
          return 1;
        }
      }
    }
    // There should be exactly one positional argument
    if ( argc - ::optind != 1 )
    {
      //std::cerr << "ERROR! Expect exactly one argument\n" ;
      usage( argv[0], std::cerr );
      return 1;
    }
    std::string inFileName = argv[::optind];
    // Replace "//" by "/" in the input file name
    for ( size_t i = inFileName.find( "//" ); i != std::string::npos; i = inFileName.find( "//" ) )
      inFileName.erase( i, 1 );
    // Check that the input file exists and ends by '.bfin'
    if ( inFileName.rfind( ".bfin" ) != inFileName.size()-5 )
    {
      std::cerr << "ERROR! Input file '" << inFileName << "' does not end by '.bfin'!" << std::endl;
      return 1;
    }
    else
    {
      struct stat sb;
      if ( ::stat( inFileName.c_str(), &sb ) != 0 )
      {
        std::cerr << "ERROR! Input file " << inFileName << " does not exist!" << std::endl;
        return 1;
      }
    }
    std::string inFileBase = inFileName.substr( 0, inFileName.rfind( ".bfin" ) );
    std::string inFileDir = "";
    if ( inFileBase.rfind( "/" ) != std::string::npos )
    {
      inFileDir = inFileBase.substr( 0, inFileBase.rfind( "/" )+1 );
      inFileBase = inFileBase.substr( inFileBase.rfind( "/" )+1 );
    }
    if ( inFileName != inFileDir+inFileBase+".bfin" )
    {
      std::cerr << "INTERNAL ERROR! inFileName mismatch?" << std::endl;
      return 1;
    }
    // Check if the input directory exists
    // We really do this only to compare its stat to the tex and out dirs later
    struct stat inFileDirSb;
    {
      std::string inFileDirTmp = ( inFileDir != "" ? inFileDir : std::string( "./" ) );
      if ( ::stat( inFileDirTmp.c_str(), &inFileDirSb ) != 0 )
      {
        std::cerr << "INTERNAL ERROR! Input directory " << inFileDir
                  << " does not exist?" << std::endl;
        return 1;
      }
      if ( !S_ISDIR( inFileDirSb.st_mode ) )
      {
        std::cerr << "INTERNAL ERROR! Input directory " << inFileDir
                  << " is not a directory?" << std::endl;
        return 1;
      }
    }
    // If no output directory was specified, use the input file directory
    struct stat outFileDirSb;
    if ( outFileDir == "" )
    {
      outFileDir = inFileDir;
      outFileDirSb = inFileDirSb;
    }
    else
    {
      // Replace "//" by "/" in the output directory and ensure a trailing "/"
      for ( size_t i = outFileDir.find( "//" ); i != std::string::npos; i = outFileDir.find( "//" ) )
        outFileDir.erase( i, 1 );
      if ( outFileDir[outFileDir.size()-1] != '/' ) outFileDir += "/";
      // Check if the output directory exists
      if ( ::stat( outFileDir.c_str(), &outFileDirSb ) != 0 )
      {
        std::cerr << "ERROR! Output directory " << outFileDir << " does not exist!" << std::endl;
        return 1;
      }
      if ( !S_ISDIR( outFileDirSb.st_mode ) )
      {
        std::cerr << "ERROR! File " << outFileDir << " is not a directory!" << std::endl;
        return 1;
      }
    }
    const std::string pdfFileNameNoDir = inFileBase+".pdf";
    const std::string pdfFileName = outFileDir+pdfFileNameNoDir;
    const std::string logFileName = outFileDir+inFileBase+"_bluefin.log";
    // If no texdir was specified, create a temporary directory
    if ( texFileDir == "" )
    {
      std::string tmpDir = std::string( "/tmp/" ) + ::getenv("USER") + "/bluefin";
      ::system( ( "mkdir -p " + tmpDir ).c_str() );
      tmpDir += "/XXXXXXXX";
      char* tmpDirC = (char*) ::malloc( tmpDir.size() + 1 );
      ::strcpy( tmpDirC, tmpDir.c_str() );
      if ( ! ::mkdtemp( tmpDirC ) )
      {
        std::cerr << "ERROR! Could not create temporary directory " << tmpDir << std::endl;
        ::free( tmpDirC );
        return 1;
      }
      texFileDir = tmpDirC;
      ::free( tmpDirC );
    }
    // Replace "//" by "/" in the output directory and ensure a trailing "/"
    for ( size_t i = texFileDir.find( "//" ); i != std::string::npos; i = texFileDir.find( "//" ) )
      texFileDir.erase( i, 1 );
    if ( texFileDir[texFileDir.size()-1] != '/' ) texFileDir += "/";
    // Check if the tex directory exists
    struct stat texFileDirSb;
    if ( ::stat( texFileDir.c_str(), &texFileDirSb ) != 0 )
    {
      std::cerr << "ERROR! Tex directory " << texFileDir << " does not exist!" << std::endl;
      return 1;
    }
    if ( !S_ISDIR( texFileDirSb.st_mode ) )
    {
      std::cerr << "ERROR! File " << texFileDir << " is not a directory!" << std::endl;
      return 1;
    }
    const std::string texBodyNameBase = inFileBase+"_body";
    const std::string texBodyNameNoDir = texBodyNameBase+".tex";
    const std::string texBodyName = texFileDir+texBodyNameNoDir;
    const std::string texFileNameNoDir = inFileBase+".tex";
    const std::string texFileName = texFileDir+texFileNameNoDir;
    // Check whether pdflatex is installed on the system
    if ( ::system( "which pdflatex > /dev/null" ) )
    {
      std::cerr << "ERROR! pdflatex is not installed!" << std::endl;
      return 1;
    }
    // Print the input arguments
    const std::string blueFinVersion = BLUEFIN_VERSION;
    std::cout << "==================================================================" << std::endl;
    std::cout << "BLUEFIN version " << blueFinVersion << std::endl;
    std::cout << "==================================================================" << std::endl;
    std::cout << "- input file:      " << inFileName << std::endl;
    std::cout << "- output PDF file: " << pdfFileName << std::endl;
    std::cout << "- output LOG file: " << logFileName << std::endl;
    std::cout << "- output TEX body: " << texBodyName << std::endl;
    std::cout << "- output TEX file: " << texFileName << std::endl;
    // Create BlueFinLogo.jpg in the tex directory
    {
      std::ofstream bStr;
      bStr.open ( ( texFileDir + "/BlueFinLogo.jpg" ).c_str() );
      char* p = &_binary_BlueFinLogo_jpg_start;
      while ( p != &_binary_BlueFinLogo_jpg_end ) bStr << *p++;
      bStr.close();
    }
    // Create BlueFinReport.tex in the tex directory
    {
      std::ofstream bStr;
      bStr.open ( ( texFileDir + "/BlueFinReport.tex" ).c_str() );
      char* p = &_binary_BlueFinReport_tex_start;
      while ( p != &_binary_BlueFinReport_tex_end ) bStr << *p++;
      bStr.close();
    }
    // Copy the input file to the tex directory
    // (unless the tex directory is the input directory)
    if ( texFileDirSb.st_dev != inFileDirSb.st_dev ||
         texFileDirSb.st_ino != inFileDirSb.st_ino )
    {
      std::string cmd = "cp " + inFileName + " " + texFileDir;
      if ( ::system( cmd.c_str() ) )
      {
        std::cerr << cmd << std::endl;
        std::cerr << "ERROR! copying input file failed!" << std::endl;
        return 1;
      }
    }
    // Create a BLUE combination from the input data
    BlueFish bf = InputParser::createBlueFishFromInputData( inFileName );
    std::ofstream tStr; // text stream
    tStr.open ( logFileName.c_str() );
    InfoAnalyzer::PrintoutOpts printoutOpts = { covPrintoutOpts, minimizationOpts };
    InfoAnalyzer::printInfoAnalysis( bf, tStr, texBodyName, printoutOpts );
    tStr.close();
    // Warning message (reserved option)
    std::string extraFooter;
    std::string extraFootMM = "0mm";
    if ( ::getenv( "BFEXTRAFOOTER" ) )
    {
      extraFooter = std::string( ::getenv( "BFEXTRAFOOTER" ) );
      extraFootMM = "4mm";
    }
    // Fine tuning of text width and height
    std::string wAddPt = ""; // not yet determined
    std::string hAddPt = ""; // not yet determined
    do
    {
      // Create a tex wrapper from BlueFinReport.tex
      std::string cmd = "cat " + texFileDir + "BlueFinReport.tex";
      std::string zero = "0";
      cmd += " | sed \"s|BFWADDPT|" + ( wAddPt != "" ? wAddPt : zero ) + "|\"";
      cmd += " | sed \"s|BFHADDPT|" + ( hAddPt != "" ? hAddPt : zero ) + "|\"";
      cmd += " | sed \"s|BFMYFILENAMEIN|" + inFileBase + ".bfin|\"";
      cmd += " | sed \"s|BFMYFILENAMEBODY|" + texBodyNameBase + "|\"";
      cmd += " | sed \"s|BFMYCOMBNAME|" + bf.combName() + "|\"";
      cmd += " | sed \"s|BFVERS|" + blueFinVersion + "|\"";
      cmd += " | sed \"s|BFEXTRAFOOTER|" + extraFooter + "|\"";
      cmd += " | sed \"s|BFEXTRAFOOTMM|" + extraFootMM + "|\"";
      cmd += " > " + texFileName;
      if ( ::system( cmd.c_str() ) )
      {
        std::cerr << cmd << std::endl;
        std::cerr << "ERROR! cat/sed failed!" << std::endl;
        return 1;
      }
      // Execute pdflatex
      const std::string tmpFileNameNoDir = inFileBase+"_tmp.txt";
      const std::string tmpFileName = texFileDir+tmpFileNameNoDir;
      cmd = "cd " + texFileDir + ";";
      cmd += " pdflatex -interaction=nonstopmode " + texFileNameNoDir;
      cmd += " > " + tmpFileNameNoDir;
      if ( ::system( cmd.c_str() ) )
      {
        std::cerr << cmd << std::endl;
        std::cerr << "ERROR! pdflatex (1st pass) failed!" << std::endl;
        return 1;
      }
      // Execute pdflatex again
      cmd = "cd " + texFileDir + ";";
      cmd += " pdflatex -interaction=nonstopmode " + texFileNameNoDir;
      cmd += " >> " + tmpFileNameNoDir;
      if ( ::system( cmd.c_str() ) )
      {
        std::cerr << cmd << std::endl;
        std::cerr << "ERROR! pdflatex (2nd pass) failed!" << std::endl;
        return 1;
      }

      // Check for width and/or height overfull in the tex output
      // 1. Fix the final width first - check for width overfull
      if ( wAddPt == "" ) // do this only the first time!
      {
        cmd = "cat " + tmpFileName + " | egrep '^Overfull \\\\hbox'";
        cmd += " | awk '{h=substr($3,2,length($3)-3); print h}' | sort -g -u | tail -1";
        FILE* pipe = ::popen( cmd.c_str(), "r" );
        if ( pipe )
        {
          wAddPt = "";
          int c;
          while( ( c = fgetc( pipe ) ) != EOF && c != '\n' ) wAddPt += std::string( 1, c );
          ::pclose(pipe);
          if ( wAddPt == "" )
          {
            wAddPt = "0"; // no overfull found - fix the final width
          }
          else
          {
            std::stringstream msg;
            msg << "WARNING! Overfull hbox: increase width by " << wAddPt << "pt";
            std::cout << msg.str() << std::endl;
            cmd = "echo " + msg.str() + " >> " + logFileName;
            if ( ::system( cmd.c_str() ) )
            {
              std::cerr << cmd << std::endl;
              std::cerr << "ERROR! echo/append failed!" << std::endl;
              return 1;
            }
          }
        }
      }
      // 2. If width has been fixed, fix the final height - check for height overfull
      else if ( hAddPt == "" ) // do this only the first time!
      {
        cmd = "cat " + tmpFileName + " | egrep '^Overfull \\\\vbox'";
        cmd += " | awk '{h=substr($3,2,length($3)-3); print h}' | sort -g -u | tail -1";
        FILE* pipe = ::popen( cmd.c_str(), "r" );
        if ( pipe )
        {
          hAddPt = "";
          int c;
          while( ( c = fgetc( pipe ) ) != EOF && c != '\n' ) hAddPt += std::string( 1, c );
          ::pclose(pipe);
          if ( hAddPt == "" )
          {
            hAddPt = "0"; // no overfull found - fix the final height
          }
          else
          {
            std::stringstream msg;
            msg << "WARNING! Overfull vbox: increase height by " << hAddPt << "pt";
            std::cout << msg.str() << std::endl;
            cmd = "echo " + msg.str() + " >> " + logFileName;
            if ( ::system( cmd.c_str() ) )
            {
              std::cerr << cmd << std::endl;
              std::cerr << "ERROR! echo/append failed!" << std::endl;
              return 1;
            }
          }
          // You potentially have both the final width and height now.
          // However it would be nice to have an A4-like ratio between the two!
          const float a4ratio = 296./210.;
          float wAdd = ::atof( wAddPt.c_str() );
          float hAdd = ::atof( hAddPt.c_str() );
          std::stringstream msg;
          //std::cout << "DEBUG1 extras: width=" << wAddPt << ", height=" << hAddPt << std::endl;
          if ( wAddPt == "0" && hAddPt == "0" )
          {
            // Nothing to do!
          }
          else if ( wAdd > hAdd*(a4ratio*0.99) ) // allow 1% too high rather than forcing a new loop
          {
            // We added more width than height
            // We can safely increase the height accordingly and exit the loop
            std::stringstream str;
            str << wAdd/a4ratio;
            hAddPt = str.str();
            msg << "WARNING! Re-gain A4 ratio: increase height by " << hAddPt << "pt instead";
          }
          else
          {
            // We added more height than width
            // We must increase the width accordingly and recompute the height!
            std::stringstream str;
            str << hAdd*a4ratio;
            wAddPt = str.str();
            hAddPt = "";
            msg << "WARNING! Re-gain A4 ratio: increase width by " << wAddPt << "pt instead";
          }
          if ( msg.str() != "" )
          {
            std::cout << msg.str() << std::endl;
            cmd = "echo " + msg.str() + " >> " + logFileName;
            if ( ::system( cmd.c_str() ) )
            {
              std::cerr << cmd << std::endl;
              std::cerr << "ERROR! echo/append failed!" << std::endl;
              return 1;
            }
          }
          //std::cout << "DEBUG2 extras: width=" << wAddPt << ", height=" << hAddPt << std::endl;
        }
      }
      // 3. If both width and height have been fixed, exit the loop
      // (pdflatex has been executed one last time with the final width and height)!
      else
      {
        wAddPt = "";
        hAddPt = "";
      }
      // Append tex output and remove temporary file
      if ( !quietLatex )
        cmd = "cat " + tmpFileName + " >> " + logFileName + "; rm " + tmpFileName;
      else
        cmd = "rm " + tmpFileName;
      if ( ::system( cmd.c_str() ) )
      {
        std::cerr << cmd << std::endl;
        if ( !quietLatex )
          std::cerr << "ERROR! cat/append/rm failed!" << std::endl;
        else
          std::cerr << "ERROR! rm failed!" << std::endl;
        return 1;
      }
    }
    while ( wAddPt != "" || hAddPt != "" ); // both are == "" at the end
    // Copy pdf from the tex to the output directory
    // (unless the output directory is the tex directory)
    if ( outFileDirSb.st_dev != texFileDirSb.st_dev ||
         outFileDirSb.st_ino != texFileDirSb.st_ino )
    {
      std::string cmd = "mv " + texFileDir + pdfFileNameNoDir + " " + pdfFileName;
      if ( ::system( cmd.c_str() ) )
      {
        std::cerr << cmd << std::endl;
        std::cerr << "ERROR! mv pdf failed!" << std::endl;
        return 1;
      }
    }
    // Cleanup - remove BlueFinReport.tex
    std::string cmd = "rm " + texFileDir + "BlueFinReport.tex";
    if ( ::system( cmd.c_str() ) )
    {
      std::cerr << cmd << std::endl;
      std::cerr << "ERROR! rm BlueFinReport.tex failed!" << std::endl;
      return 1;
    }
  }
  catch ( std::exception& e )
  {
    std::cout << "ERROR! Exception caught: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

//-----------------------------------------------------------------------------
