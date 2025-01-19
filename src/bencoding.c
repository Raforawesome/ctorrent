#include "headers/bencoding.h"
#include <stdio.h>
#include <stdlib.h>

static int ipow(int b, int n) {
    if (n == 0) return 1;
    if (n == 1) return b;
    int sum = b;
    for (int i = 2; i <= n; i++)
        sum *= b;
    return sum;
}

// parses a `ByteString` from a raw C string.
ByteString parse_bstring(char* input) {
    ByteString new;

    // any string that needs more than 8 digits to represent its **LENGTH**
    // is a SIN and i dont care to support it
    char raw_num[9]; // 1 extra for null terminator
    unsigned char d = 0; // keep track of digits using char because it's the smallest number
    char c;
    while ((c = *input) != ':')  {
        if (c == '\0') { // if string ends before ':'
            fputs("Malformed string to parse_bstring!", stderr);
            exit(1); // replace with actual exception handling later
        }
        if (c >= '0' && c <= '9') // if c is a digit
            raw_num[d++] = c; // add it to our number string
        input++;
    }
    raw_num[d--] = '\0'; // TODO: remove?

    int len = 0;
    int i = 0;
    for (; d >= 0; d--) {
        len += ipow(10, d) * raw_num[i]; // use exp to get correct tens place
        i++;
    }

    new.repr = ++input;
    new.length = len;
    return new;

}

BInt parse_bint(char* input);
