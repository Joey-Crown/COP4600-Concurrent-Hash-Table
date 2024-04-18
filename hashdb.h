#ifndef HASHDB_H
#define HASHDB_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct hash_struct
{
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length);
hashRecord* get_record(uint32_t hash, const char* name, uint32_t salary);
void insert_record(hashRecord* head, uint32_t hash, const char* name, uint32_t salary);
void delete_record(hashRecord* head, uint32_t hash);
hashRecord* search_record(hashRecord* head, uint32_t hash);

#endif