/******************************************************************************
 * $Id: vapor.c 31 2012-08-02 13:17:08Z mikeleany $
 * Project: Vapor Chess
 * Purpose: Program entry point.
 * 
 * Copyright 2012 by Michael Leany
 * All rights reserved
 */

#include "vapor.h"
#include "io.h"
#include "uci.h"
#include "version.h"
#include "mgtest.h"
#include "init.h"

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>


/******************************************************************************
 * int findCommand(const char *Command);
 * PARAMETERS
 *    Command -- the command to find
 * DESCRIPTION
 *    Finds the command in a list and returns the appropriate command code.
 * RETURN VALUE
 *    Returns the command code for the given command keyword.
 */
int findCommand(const char *Command) {
  static const char *CmdList[] = {
    NULL,
    "perft",
    "vcount",
    "mgtest",
    NULL
  };

  for (int i = 1; CmdList[i]; i++) {
    if (strcmp(Command, CmdList[i]) == 0) {
      return i;
    }
  }

  return 0; // not a valid command
}
#define PERFTEST  1
#define VCOUNT    2
#define MGTEST    3

/******************************************************************************
 * int main(int ArgC, char **ArgV);
 * PARAMETERS
 *    ArgC -- the number of strings contained in ArgV.
 *    ArgV -- array of string arguments passed to the program.
 * DESCRIPTION
 *    The program's entry point.
 * RETURN VALUE
 *    Returns 0 upon success or 1 upon failure.
 */
int main(int ArgC, char **ArgV)
{
  static int ShowPrompt = 0;
  static const struct option LongOptions[] = {
    { "prompt", no_argument, &ShowPrompt, 1 },
    { "noprompt", no_argument, &ShowPrompt, 0 },
    { "help", optional_argument, NULL, 'h' },
    { "uci", no_argument, NULL, 'u' },
    { "version", no_argument, NULL, 'v' },
    { "xboard", no_argument, NULL, 'x' },
    { "fen", required_argument, NULL, 'f' },
    { "depth", required_argument, NULL, 'd' },
    { "epdfile", required_argument, NULL, 'e' },
    { 0, 0, 0, 0 }
  };

  int Depth = 0;
  char *EPDFile = NULL;
  char *Fen = NULL;

  int Opt = 0;
  int LongIndex = -1;
  int CmdCode = 0;
  const char *Command = NULL;
  const char *Prog = "vapor";

  if (ArgC > 0) {
    Prog = ArgV[0];
  }

  if (ArgC > 1 && ArgV[1][0] != '-') {
    Command = ArgV[1];
    CmdCode = findCommand(Command);
    if (!CmdCode) {
      fflush(stdout);
      fprintf(stderr, "%s: invalid command '%s'\n", Prog, Command);
      return 1;
    }
    optind = 2;
  }

  Opt = getopt_long_only(ArgC, ArgV, "", LongOptions, &LongIndex);
  while (Opt != -1)
  {
    switch (Opt)
    {
      case 0: // variable assignment was done
        break;
      case 'h': // help
        printf("Help not yet available\n");
        return 0;
      case 'u': // uci
        if (CmdCode) {
          fflush(stdout);
          fprintf(stderr, "%s: '--%s' option not valid with '%s' command\n",
              Prog, LongOptions[LongIndex].name, Command);
          return 1;
        }
        break;
      case 'v': // version
        if (CmdCode) {
          fflush(stdout);
          fprintf(stderr, "%s: '--%s' option not valid with '%s' command\n",
              Prog, LongOptions[LongIndex].name, Command);
          return 1;
        }
        printVersion();
        return 0;
      case 'f': // fen
        if (CmdCode != PERFTEST && CmdCode != VCOUNT) {
          fflush(stdout);
          fprintf(stderr, "%s: '--%s' option not valid in this context\n",
              Prog, LongOptions[LongIndex].name);
          return 1;
        }
        Fen = optarg;
        break;
      case 'd': // depth
        if (CmdCode != PERFTEST && CmdCode != VCOUNT) {
          fflush(stdout);
          fprintf(stderr, "%s: '--%s' option not valid in this context\n",
              Prog, LongOptions[LongIndex].name);
          return 1;
        }
        Depth = atoi(optarg);
        if (Depth < 1) {
          fflush(stdout);
          fprintf(stderr, "%s: depth must be a positive integer\n", Prog);
          return 1;
        }
        break;
      case 'e': // epdfile
        if (CmdCode != MGTEST) {
          fflush(stdout);
          fprintf(stderr, "%s: '--%s' option not valid in this context\n",
              Prog, LongOptions[LongIndex].name);
          return 1;
        }
        EPDFile = optarg;
        break;
      case 'x':
        fflush(stdout);
        fprintf(stderr,
            "%s: This is a UCI engine.\n"
            "\tDownload polyglot if you need xboard support.\n", Prog);
        return 1;
      case '?': // unknown option
        // TODO: display usage information
        return 1;
      default: // option not implemented
        if (LongIndex >= 0)
        {
          fflush(stdout);
          fprintf(stderr, "%s: oops, '--%s' option not implemented\n",
              Prog, LongOptions[LongIndex].name);
        }
        else
        {
          fflush(stdout);
          fprintf(stderr, "%s: oops, option not implemented: -%c\n",
              Prog, Opt);
        }
        return 1;
    }

    LongIndex = -1;
    Opt = getopt_long_only(ArgC, ArgV, "", LongOptions, &LongIndex);
  }

  if (CmdCode) {
    init();
    switch (CmdCode) {
      case PERFTEST:
        if (Depth < 1) {
          fflush(stdout);
          fprintf(stderr, "%s: depth not specified\n", Command);
          return 1;
        }
        return perftest(Fen, Depth);

      case VCOUNT:
        if (Depth < 1) {
          fflush(stdout);
          fprintf(stderr, "%s: depth not specified\n", Command);
          return 1;
        }
        return printVariations(Fen, Depth);

      case MGTEST:
        if (!EPDFile) {
          fflush(stdout);
          fprintf(stderr, "%s: no epd file specified\n", Command);
          return 1;
        }
        return mgtest(EPDFile);

      default:
        return 1;
    }
  }
  return ucimain();
}

/* end of file */
