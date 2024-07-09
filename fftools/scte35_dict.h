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

typedef struct DynamicIntArray {
    int64_t *values;
    int64_t cur_index;
    int64_t cur_capacity;
} DynamicIntArray;

size_t hash_function(int64_t key, size_t buckets);
SCTE35Dictionary* init_dictionary(size_t buckets, size_t valueSize);
void insert(SCTE35Dictionary *dict, int64_t key, void* value);
void* find(SCTE35Dictionary *dict, int64_t key);
void free_entry(SCTE35Dictionary* dict, int64_t key);
void free_dict(SCTE35Dictionary* dict);
void resize_dict(SCTE35Dictionary *dict);

DynamicIntArray* init_array(size_t capacity);
void array_insert(DynamicIntArray* arr, int64_t value);
void resize_array(DynamicIntArray* arr);
void free_array(DynamicIntArray* arr);
void getting_pcr_packet_nums(DynamicIntArray* arr, int64_t target, int64_t* before, int64_t* after);

#endif
