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

#ifndef _CTOR_PARSER
#define _CTOR_PARSER

typedef union {
    char* file_path;
    char* magnet_link;
} Paths;

typedef struct {
    int using_file; // 1 if using a .torrent file, 0 if magnet link
    Paths paths; // a path to the torrent file to load
} Args;

Args parse_args(int argc, char** argv);

#endif

