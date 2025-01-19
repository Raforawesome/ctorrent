/* A simple argument parser in C, using getopt to decode
 * arguments into a struct.
 * 
 * Why not:
 * - use `getopt` in main? 
 *   	Parsing ahead of time is cleaner and allows us to validate
 *   the command ahead of time.
 *
 * - use the much nicer `argp` parser?
 *   	It's not cross-platform. A C program should at least be able
 *   to run on all POSIX systems (argp doesn't work on Mac)
 * */

#include "headers/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Args parse_args(int argc, char** argv) {
    Args args;
    int mflag = 0, tflag = 0, eflag = 0;
    int c;

    while (((c = getopt(argc, argv, ":m:t:")) != -1) && !eflag) {
        switch (c) {
            case 'm': // flag -m
                if (tflag) eflag++; // if we also defined a file, this is an error
                else {
                    args.path = optarg;
                    args.using_file = 0;
                    mflag++;
                }
                break;
            case 't': // flag -t
                if (mflag) eflag++; // if we also defined magnet, this is an error
                else {
                    args.path = optarg;
                    args.using_file = 1;
                    tflag++;
                }
                break;
            case ':': // missing argument
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                break;
            case '?': // unrecognized option
                fprintf(stderr, "Option -%c not recognized.\n", optopt);
                break;
                
        }
    }

    if ((mflag || tflag) && !eflag) {
        return args;
    }
    else {
        puts("You must specify either a torrent file with -t, or a magnet link with -m.");
        exit(1);
    }
}

