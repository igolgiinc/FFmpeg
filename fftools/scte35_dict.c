#include "scte35_dict.h"
#include "cmdutils.h"

size_t hash_function(int64_t key, size_t buckets) {
    return key % buckets;
}

SCTE35Dictionary* init_dictionary(size_t buckets, size_t valueSize) {
    SCTE35Dictionary* new_dict = (SCTE35Dictionary*)malloc(sizeof(SCTE35Dictionary));
    new_dict->entries = (DictionaryEntry**)calloc(buckets, sizeof(DictionaryEntry*));
    new_dict->buckets = buckets;
    new_dict->valueSize = valueSize;

    return new_dict;
}

void insert(SCTE35Dictionary* dict, int64_t key, void* value) {
    size_t hash_index;
    DictionaryEntry *cur_entry = (DictionaryEntry*)malloc(sizeof(DictionaryEntry));
    cur_entry->key = key;
    cur_entry->value = malloc(dict->valueSize);
    memcpy(cur_entry->value, value, dict->valueSize);
    hash_index = hash_function(key, dict->buckets);
    cur_entry->next = dict->entries[hash_index];
    dict->entries[hash_index] = cur_entry;
}

void* find(SCTE35Dictionary* dict, int64_t key) {
    size_t hash_index;
    hash_index = hash_function(key, dict->buckets);
    DictionaryEntry *cur_entry = dict->entries[hash_index];
    while (cur_entry) {
        if (cur_entry->key == key) 
	    return cur_entry->value;
	cur_entry = cur_entry->next;
    }
    return NULL;
}

void free_entry(SCTE35Dictionary* dict, int64_t key) {
    size_t hash_index;
    hash_index = hash_function(key, dict->buckets);
    DictionaryEntry *prev_entry = NULL;
    DictionaryEntry *cur_entry = dict->entries[hash_index];
    while (cur_entry) {
        if (cur_entry->key == key)
	    break;	   
        prev_entry = cur_entry;
        cur_entry = cur_entry->next;
    }

    if (cur_entry) {
        if (prev_entry == NULL)
            dict->entries[hash_index] = cur_entry->next;
        else 
            prev_entry->next = cur_entry->next;

        free(cur_entry->value);
	cur_entry->value = NULL;
        free(cur_entry);
	cur_entry = NULL;
    }
}

void free_dict(SCTE35Dictionary* dict) {
    int i;
    for (i = 0; i < dict->buckets; i++) {
        DictionaryEntry *cur_entry = dict->entries[i];
	while (cur_entry) {
	    DictionaryEntry *temp_entry = cur_entry->next;
	    free(cur_entry->value);
	    cur_entry->value = NULL;
	    free(cur_entry);
	    cur_entry = NULL;
	    cur_entry = temp_entry;
	}
    }
    free(dict->entries);
    dict->entries = NULL;
    free(dict);
    dict = NULL;
}


