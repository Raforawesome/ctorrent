#include "headers/parser.h"
#include <stdio.h>

int main(int argc, char** argv) {
    Args args = parse_args(argc, argv);
    if (args.using_file) {
        printf("File input detected with path %s\n", args.path);
    } else {
        printf("Magnet input detected with link %s\n", args.path);
    }
    puts("Success!");
    return 0;
}

