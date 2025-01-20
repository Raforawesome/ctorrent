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

// Calculates integer powers. This function exists because we're only using powers
// to get places of 10 and therefore don't need the entire overhead of floating point math.
static int ipow(int b, int n) {
    if (n == 0) return 1;
    if (n == 1) return b;
    int sum = b;
    for (int i = 2; i <= n; i++)
        sum *= b;
    return sum;
}

static long long llpow(long long b, int n) {
    if (n == 0) return 1;
    if (n == 1) return b;
    long long sum = b;
    for (int i = 2; i <= n; i++)
        sum *= b;
    return sum;
}

// [s]tring [to] [i]nteger
int stoi(char* s, int n) {
    int num = 0;
    byte neg = 0;
    if (*s == '-') {
        neg++;
        s++;
    }
    for (int i = 0; i < n; i++) {
        num += ipow(10, n - i - 1) * (s[i] - 48);
    }
    return neg ? -num : num;
}

// [s]tring [to] [l]ong [l]ong  
long long stoll(char* s, int n) {
    long long num = 0;
    byte neg = 0;
    if (*s == '-') {
        neg++;
        s++;
    }
    for (int i = 0; i < n; i++) {
        num += llpow(10, n - i - 1) * (s[i] - 48);
    }
    return num;
    return neg ? -num : num;
}

// parses a `ByteString` from a raw C string.
// this method is suffixed with _unchecked to signify that it doesn't perform any 
// validation on its parameter.
ByteString parse_bstring_unchecked(char* input) {
    ByteString new;

    // any string that needs more than 16 digits to represent its **LENGTH**
    // is a SIN and i dont care to support it
    char raw_num[16];
    byte d = 0; // keep track of digits using char because it's the smallest number
    char c;
    while ((c = *input) != ':')  {
        raw_num[d++] = c; // add it to our number string (no checking needed)
        input++;
    }
    int len = stoi(raw_num, d);
    char new_str[len]; // allocate new string
    for (int i = 0; i < len; i++)
        new_str[i] = *(++input); // skip over ':'
    new_str[len] = '\0'; // null-terminate string

    new.repr = new_str;
    new.length = len;
    return new;
}

byte validate_bstring(char* input) {
    byte valid = 1, sep = 0;
    char c;
    while (valid && (c = *input) != '\0') {
        if (c == ':') sep++;
        else {
            if (!sep) {
                if (c < '0' || c > '9') valid = 0;
            }
        }
    }
    return valid && sep == 1;
}

BInt parse_bint_unchecked(char* input) {
    char raw_num[18]; // max signed 64 bit int (2^63 - 1 = 18 digits)
    int d = 0;
    char c;
    while ((c = *(++input)) != 'e') // skip first letter
        raw_num[d++] = c;
    return stoll(raw_num, d);
}

byte validate_bint(char* input) {
    char c = *input;
    byte valid = c == 'i';
    while (valid && (c = *(++input)) >= '0' && c <= '9') continue;
    return valid && c == 'e';
}
