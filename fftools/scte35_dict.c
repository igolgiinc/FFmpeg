#include "scte35_dict.h"
#include "cmdutils.h"

size_t hash_function(int64_t key, size_t buckets) {
    return key % buckets;
}

// dictionary has 64 bit integer for key and a void* for value
SCTE35Dictionary* init_dictionary(size_t buckets, size_t valueSize) {
    SCTE35Dictionary* new_dict = (SCTE35Dictionary*)malloc(sizeof(SCTE35Dictionary));
    new_dict->entries = (DictionaryEntry**)calloc(buckets, sizeof(DictionaryEntry*));
    new_dict->buckets = buckets;
    new_dict->valueSize = valueSize;
    new_dict->count = 0;

    return new_dict;
}

// dictionary uses chaining (linked lists) for hash collision resolution
void insert(SCTE35Dictionary* dict, int64_t key, void* value) {
    dict->count++;
    // dictionary reszies when number of elements exceeds 75% capacity
    if (dict->count > dict->buckets * 0.75) {
        resize_dict(dict);
    }

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

// frees allocation of one specific entry in dictionary (based on key)
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
    dict->count--;
}

// frees allocation of entire dictionary
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

// resizes dictionary to two times the original capacity
void resize_dict(SCTE35Dictionary* dict) {
    int i;
    size_t num_buckets = dict->buckets;
    num_buckets *= 2;
    DictionaryEntry **entries = (DictionaryEntry**)calloc(num_buckets, sizeof(DictionaryEntry*));
    for (i = 0; i < dict->buckets; i++) {
        DictionaryEntry *cur_entry = dict->entries[i];
        while (cur_entry) {
            size_t new_hash = hash_function(cur_entry->key, num_buckets);
            DictionaryEntry *temp_entry = cur_entry->next;
            cur_entry->next = entries[new_hash];
            entries[new_hash] = cur_entry;
            cur_entry = temp_entry;	    
        }
    }
    free(dict->entries);
    dict->entries = NULL;
    dict->entries = entries;
    dict->buckets = num_buckets;
}

// entries in array are 64 bit integers
DynamicIntArray* init_array(size_t capacity) {
    DynamicIntArray* arr = (DynamicIntArray*)malloc(sizeof(DynamicIntArray));
    arr->values = (int64_t*)malloc(sizeof(int64_t) * capacity);
    arr->cur_index = -1;
    arr->cur_capacity = capacity;

    return arr;
}

// resizes array to two times the original capacity
void resize_array(DynamicIntArray* arr) {
    arr->cur_capacity *= 2;
    arr->values = (int64_t*)realloc(arr->values, sizeof(int64_t) * arr->cur_capacity);
}

void array_insert(DynamicIntArray* arr, int64_t value) {
    int64_t next_index = arr->cur_index + 1;
    // array resizes once current capacity is reached
    if (next_index >= arr->cur_capacity)
        resize_array(arr);
    
    arr->values[next_index] = value;
    arr->cur_index = next_index;
}

void free_array(DynamicIntArray* arr) {
    free(arr->values);
    free(arr);
}

// obtains the packet numbers for the PCR timestamp before and after the target packet number
void getting_pcr_packet_nums(DynamicIntArray* arr, int64_t target, int64_t* before, int64_t* after) {
    int64_t l, r, mid;
    l = 0;
    r = arr->cur_index;

    if (arr->cur_index != -1) {
        while (l <= r) {
            mid = (l + r) / 2;
            int64_t mid_value = arr->values[mid];
            if (mid_value < target) {
                *before = mid_value;
                l = mid + 1;
            } else if (mid_value > target) {
                *after = mid_value;
                r = mid - 1;
            } else {
                *before = mid_value;
                *after = mid_value;
                break;
            }
	    }
    }

}

