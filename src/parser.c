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

    while ((c = getopt(argc, argv, "mt") != 1)) {
        switch (c) {
            case 'm':
                if (tflag) eflag++;
                else {
                    args.paths.magnet_link = optarg;
                    mflag++;
                }
            case 't':
                if (mflag) eflag++;
                else {
                    args.paths.file_path = optarg;
                    tflag++;
                }
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

