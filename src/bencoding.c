/* Implementations of (mostly parsing the) bencoding types from the BitTorrent spec.
 *
 * NOTE:
 * The design of these parsers includes two stages: validation and parsing. The motivation
 * behind separating these two phases is to abstract out any 'effects', i.e. IO and 
 * exception handling, from the actual parsing logic. Validating the inputs beforehand
 * provides benefits to both performance and the ergonomics of exception handling.
 * Due to this, any function calling the parsers should ensure they are taking valid 
 * input (this can be done with the provided validator methods).
 * */

#include "headers/bencoding.h"

static int ipow(int b, int n) {
    if (n == 0) return 1;
    if (n == 1) return b;
    int sum = b;
    for (int i = 2; i <= n; i++)
        sum *= b;
    return sum;
}

// parses a `ByteString` from a raw C string.
// this method is suffixed with _unchecked to signify that it doesn't perform any 
// validation on its parameter.
ByteString parse_bstring_unchecked(char* input) {
    ByteString new;

    // any string that needs more than 8 digits to represent its **LENGTH**
    // is a SIN and i dont care to support it
    char raw_num[9]; // 1 extra for null terminator
    unsigned char d = 0; // keep track of digits using char because it's the smallest number
    char c;
    while ((c = *input) != ':')  {
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
