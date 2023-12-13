/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       main.c
 Description:  main stub for a command line executable version of EPANET
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/07/2018
 ******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <epanet2.h>
#include <epanet2_2.h>
#include <types.h>
#include <funcs.h>

void  writeConsole(char *s)
{
    fprintf(stdout, "\r%s", s);
    fflush(stdout);
}

static Project __defaultProject;
static Project *_defaultProject = &__defaultProject;

int outputJSON(Project *, const char *);

// Functions for creating and removing default temporary files
static void createtmpfiles()
{
    getTmpName(_defaultProject->TmpHydFname);
    getTmpName(_defaultProject->TmpOutFname);
    getTmpName(_defaultProject->TmpStatFname);
}

static void removetmpfiles()
{
    remove(_defaultProject->TmpHydFname);
    remove(_defaultProject->TmpOutFname);
    remove(_defaultProject->TmpStatFname);
}



int createJSON(const char *f1, const char *f2, const char *f3, const char *f4) {
  
/*--------------------------------------------------------------
 **  Input:   argc    = number of command line arguments
 **           *argv[] = array of command line arguments
 **  Output:  none
 **  Purpose: main program stub for command line EPANET
 **
 **  Command line for stand-alone operation is:
 **    progname f1  f2  f3
 **  where progname = name of executable this code was compiled to,
 **  f1 = name of input file,
 **  f2 = name of report file
 **  f3 = name of binary output file (optional).
 **--------------------------------------------------------------
 */

  char errmsg[256] = "";
  int  errcode = 0;
  int  warncode = 0;    
  int  version;
  int  major;
  int  minor;
  int  patch;
  
  // Get version number and display in Major.Minor.Patch format
  ENgetversion(&version);
  major = version/10000;
  minor = (version%10000)/100;
  patch = version%100;
  printf("\n... Running EPANET Version %d.%d.%d\n", major, minor, patch);
  
  Project *theProjectPtr = _defaultProject;
  
  // Run the project and record any warning
  createtmpfiles();
  ERRCODE(EN_open(theProjectPtr, f1, f2, f3));        
  if (errcode < 100) warncode = errcode;
  removetmpfiles();
  
  if (warncode) errcode = MAX(errcode, warncode);
  
  // Check for errors/warnings and report accordingly
  if (errcode != 0) {
    if (errcode < 100) {
      printf("\n... EPANET ran with warnings - check the Status Report.\n");
      return errcode;
    } else {
      ENgeterror(errcode, errmsg, 255);
      printf("\n... EPANET failed with %s.\n", errmsg);
      return 100;
    }
  }
  
  if (f4 != 0) {
    outputJSON(theProjectPtr, f4);
  }
}
