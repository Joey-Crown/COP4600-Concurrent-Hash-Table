#include "hashdb.h"

uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length) 
{
    size_t i = 0;
    uint32_t hash = 0;
    while (i != length) {
    hash += key[i];
    hash += hash << 10;
    hash ^= hash >> 6;
    i++;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

hashRecord* get_record(uint32_t hash, const char* name, uint32_t salary) {
    hashRecord *new_record = (hashRecord*)malloc(sizeof(hashRecord));
    new_record->hash = hash;
    strcpy(new_record->name, name);
    new_record->salary = salary;
    new_record->next = NULL;
    return new_record;
}

void insert_record(hashRecord* head, uint32_t hash, const char* name, uint32_t salary) 
{
    hashRecord *temp = head;

    if (temp == NULL) {
        temp = get_record(hash, name, salary);
        return;
    }

    // Traverse and update existing node if it exists
    while (temp->next != NULL) {
        if (temp->hash == hash) {
            temp->salary = salary;
            return;
        }
        temp = temp->next;
    }

    // Check last node in list and update or insert
    if (temp->hash == hash) {
        temp->salary = salary;
    } else {
        temp->next = get_record(hash, name, salary);
    }

    return;
}

void delete_record(hashRecord* head, uint32_t hash)
{
    hashRecord *temp = head;
    hashRecord *prev = NULL;
    if (temp->hash == hash) {
        head = temp->next;
        free(temp);
        return;
    }

    while(temp != NULL) {
        if (temp->hash == hash) {
            prev->next = temp->next;
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}

hashRecord* search_record(hashRecord* head, uint32_t hash)
{
    hashRecord *temp = head;
    while (temp != NULL) {
        if (temp->hash == hash) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}