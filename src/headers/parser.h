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

#ifndef __CTOR_PARSER__
#define __CTOR_PARSER__

typedef struct {
    int using_file; // 1 if using a .torrent file, 0 if magnet link
    char* path; // a path to the torrent file to load, or a magnet link
} Args;

Args parse_args(int argc, char** argv);

#endif

