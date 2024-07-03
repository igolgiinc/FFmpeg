#ifndef _SCTE35_DICT_
#define _SCTE35_DICT

#include "cmdutils.h"
#include "config.h"
#include "stdio.h"

typedef struct DictionaryEntry {
    int64_t key;
    void *value;
    struct DictionaryEntry *next;
} DictionaryEntry;

typedef struct SCTE35Dictionary {
    DictionaryEntry **entries;
    size_t buckets;
    size_t valueSize;
    size_t count;
} SCTE35Dictionary;

size_t hash_function(int64_t key, size_t buckets);
SCTE35Dictionary* init_dictionary(size_t buckets, size_t valueSize);
void insert(SCTE35Dictionary *dict, int64_t key, void* value);
void* find(SCTE35Dictionary *dict, int64_t key);
void free_entry(SCTE35Dictionary* dict, int64_t key);
void free_dict(SCTE35Dictionary* dict);
void resize_dict(SCTE35Dictionary *dict);

#endif
