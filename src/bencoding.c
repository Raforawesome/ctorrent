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
#include <stdlib.h>

static int last_read = 0; // the ultimate sin

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
    // is a *SIN* and i dont care to support it
    char raw_num[16];
    unsigned long long bytes_read = 0; // we support up to 16 digits
    byte d = 0; // keep track of digits using char because it's the smallest number
    char c;
    while ((c = *input) != ':')  {
        raw_num[d++] = c; // add it to our number string (no checking needed)
        input++;
        bytes_read++;
    }
    int len = stoi(raw_num, d);
    char new_str[len]; // allocate new string
    for (int i = 0; i < len; i++)
        new_str[i] = *(++input); // skip over ':'
    new_str[len] = '\0'; // null-terminate string
    bytes_read += len;
    bytes_read++; // account for ':'

    new.repr = new_str;
    new.length = len;
    last_read = bytes_read;
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
    last_read = d + 2;
    return stoll(raw_num, d);
}

byte validate_bint(char* input) { // returns total bytes read
    char c = *input;
    byte valid = c == 'i';
    while (valid && (c = *(++input)) >= '0' && c <= '9') valid++;
    if (c == 'e' && valid++) // add one for ending delimiter
        return valid;
    else
        return -1;
} 

BDict parse_bdict_unchecked(char* input); // declare beforehand to use in list parser

static inline void blist_add_node(BList* list, BListNode* node) {
    if (list->head == NULL) {
        list->head = node;
    } else {
        list->head->next = node;
    }
    list->length++;
}

BList parse_blist_unchecked(char* input) {
    BList list;
    list.length = 0;
    list.head = NULL;
    unsigned long long bytes_read = 0;
    input++; // skip the 'l'
    
    char c;
    while ((c = *input) != 'e') {
        BListNode* node = malloc(sizeof *node);
        node->next = NULL;
        if (c == 'i') { // case: bencoded integer
            long long* node_data = malloc(sizeof *node_data);
            *node_data = parse_bint_unchecked(input);
            node->data = (void*) node_data;
            node->type = BInt_t;
        } else if (c == 'l') { // recursively parse list
            BList* node_data = malloc(sizeof *node_data);
            *node_data = parse_blist_unchecked(input);
            node->data = (void*) node_data;
            node->type = BList_t;
        } else if (c == 'd') { // parse dictionary
            BDict* node_data = malloc(sizeof *node_data);
            *node_data = parse_bdict_unchecked(input);
            node->data = (void*) node_data;
            node->type = BDict_t;
        } else { // must be a string, no one would call an unchecked function with invalid data right?
            ByteString* node_data = malloc(sizeof *node_data);
            *node_data = parse_bstring_unchecked(input);
            node->data = (void*) node_data;
            node->type = BString_t;
        }
        blist_add_node(&list, node);
        input += last_read;
        bytes_read += last_read;
    }
    
    last_read = bytes_read;
    return list;
}

static inline void bdict_add_node(BDict* dict, BDictNode* node) {
    if (dict->head == NULL) {
        dict->head = node;
    } else {
        dict->head->next = node;
    }
    dict->length++;
}

BDict parse_bdict_unchecked(char *input) {
    BDict dict;
    dict.length = 0;
    dict.head = NULL;
    unsigned long long bytes_read = 0;
    input++; // skip the 'd'

    char c;
    while ((c = *input) != 'e') {
        BDictNode* node = malloc(sizeof *node);
        node->next = NULL;
        if (c == 'i') { // case: bencoded integer
            long long* node_data = malloc(sizeof *node_data);
            *node_data = parse_bint_unchecked(input);
            node->key = (void*) node_data;
            node->key_type = BInt_t;
        } else if (c == 'l') { // recursively parse list
            BList* node_data = malloc(sizeof *node_data);
            *node_data = parse_blist_unchecked(input);
            node->key = (void*) node_data;
            node->key_type = BList_t;
        } else if (c == 'd') { // parse dictionary
            BDict* node_data = malloc(sizeof *node_data);
            *node_data = parse_bdict_unchecked(input);
            node->key = (void*) node_data;
            node->key_type = BDict_t;
        } else { // must be a string, no one would call an unchecked function with invalid data right?
            ByteString* node_data = malloc(sizeof *node_data);
            *node_data = parse_bstring_unchecked(input);
            node->key = (void*) node_data;
            node->key_type = BString_t;
        }
        input += last_read;
        bytes_read += last_read;
        c = *input; // shouldn't ever be 'e', dict keys come in pairs
        if (c == 'i') { // case: bencoded integer
            long long* node_data = malloc(sizeof *node_data);
            *node_data = parse_bint_unchecked(input);
            node->value = (void*) node_data;
            node->value_type = BInt_t;
        } else if (c == 'l') { // recursively parse list
            BList* node_data = malloc(sizeof *node_data);
            *node_data = parse_blist_unchecked(input);
            node->value = (void*) node_data;
            node->value_type = BList_t;
        } else if (c == 'd') { // parse dictionary
            BDict* node_data = malloc(sizeof *node_data);
            *node_data = parse_bdict_unchecked(input);
            node->value = (void*) node_data;
            node->value_type = BDict_t;
        } else { // must be a string, no one would call an unchecked function with invalid data right?
            ByteString* node_data = malloc(sizeof *node_data);
            *node_data = parse_bstring_unchecked(input);
            node->value = (void*) node_data;
            node->value_type = BString_t;
        }
        input += last_read;
        bytes_read += last_read;
        bdict_add_node(&dict, node);
    }

    last_read = bytes_read;
    return dict;
}
