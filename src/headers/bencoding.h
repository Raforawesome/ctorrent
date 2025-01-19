/* C implementation of all the data types under 'Bencoding' in the BitTorrent spec.
 * See: https://wiki.theory.org/BitTorrentSpecification#Bencoding
 * */

#ifndef __CTOR_BENCODING__
#define __CTOR_BENCODING__

/* Byte String:
 * Byte strings are encoded like: <length in ASCII>:<string>
 * Example: "4:spam" represents the string "spam"
 * Note: No terminators
 *
 * field `raw` stores the raw input, and the "output", aka the `repr` field,
 * is simply just a pointer to the same string but advanced ahead by 2 bytes
 * to reference the string itself.
 * */
typedef struct {
    char* raw;
    char* repr;
    unsigned int length;
} ByteString;

ByteString parse_bstring(char* input);

/* Integer (Bencoding-specific)
 * Integers in this spec are encoded as 'i{n}e', where `n` represents a number. The
 * integer is delimited by the constant chars 'i' and 'e'. The representation (`repr` field)
 * is set to long long as the spec mandates these must be parsed as 64-bit integers.
 * Example: 'i42e' represents the integer (long long) 42.
 * */
typedef struct {
    char* raw;
    long long repr;
} BInt;

BInt parse_bint(char* input);

/* 
 * */
typedef struct {} BList;

/* 
 * */
typedef struct {} BDict;

#endif

