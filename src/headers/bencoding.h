/* C implementation of all the data types under 'Bencoding' in the BitTorrent spec.
 * See: https://wiki.theory.org/BitTorrentSpecification#Bencoding
 *
 * NOTE:
 * The design of the parsers includes two stages: validation and parsing. The motivation
 * behind separating these two phases is to abstract out any 'effects', i.e. IO and 
 * exception handling, from the actual parsing logic. Validating the inputs beforehand
 * provides benefits to both performance and the ergonomics of exception handling.
 * Due to this, any function calling the parsers should ensure they are taking valid 
 * input (this can be done with the provided validator functions).
 * */

#ifndef __CTOR_BENCODING__
#define __CTOR_BENCODING__

// so we don't have to use 2 bytes for small vars
typedef unsigned char byte;

typedef enum {
    BString_t,
    BInt_t,
    BList_t,
    BDict_t,
} BType;

/* Byte String:
 * Byte strings are encoded like: <length in ASCII>:<string>
 * Example: "4:spam" represents the string "spam"
 * Note: No terminators
 *
 * output field `repr` contains a copy of the string that was parsed.
 * */
typedef struct {
    char* repr;
    unsigned int length;
    unsigned long long *bytes_read; // de-allocated after use
} ByteString;

// Parses a ByteString from a raw C string that is assumed to be valid.
ByteString parse_bstring_unchecked(char* input);

// Returns 1 if C string passed is a valid ByteString, 0 if not.
byte validate_bstring(char* input);

/* Integer (Bencoding-specific)
 * Integers in this spec are encoded as 'i{n}e', where `n` represents a number. The
 * integer is delimited by the constant chars 'i' and 'e'. The representation used here
 * is set to long long as the spec mandates these must be parsed as 64-bit integers. 
 * Example: 'i42e' represents the integer (long long) 42.
 *
 * Since there is no need for a whole struct here, this one is a simple type alias for brevity.
 * */
typedef long long BInt;

// Parses a BInt from a raw C string that is assumed to be valid.
BInt parse_bint_unchecked(char* input);

// Returns 1 if C string passed is a valid BInt, 0 if not.
unsigned char validate_bint(char* input);

/* 
 * */
typedef struct __CTOR_BLIST_NODE__ { // need an ugly name to reference in `next` field
    BType type;
    void* data;
    struct __CTOR_BLIST_NODE__ *next;
} BListNode;

typedef struct {
    BListNode* head;
    unsigned int length;
} BList;

BList parse_blist_unchecked(char* input);

unsigned char validate_blist(char* input);

/* 
 * */
// We purposefully choose not to implement a hash table here because for the expected
// size of the dictionaries (according to the .torrent file spec), they're small enough
// that linear-time indexing is probably faster than most hash functions.
typedef struct __CTOR_BDICT_NODE__ { // need an ugly name to reference in `next` field
    BType key_type;
    void* key;
    BType value_type;
    void* value;
    struct __CTOR_BDICT_NODE__ *next;
} BDictNode;

typedef struct {
    BDictNode* head;
    unsigned int length;
} BDict;

BDict parse_bdict_unchecked(char* input);

unsigned char validate_bdict(char* input);

#endif

