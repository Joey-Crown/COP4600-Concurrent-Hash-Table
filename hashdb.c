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

void get_record(hashRecord** new_record, uint32_t hash, const char* name, uint32_t salary) {
    *new_record = (hashRecord*) malloc(sizeof(hashRecord));
    (*new_record)->hash = hash;
    strcpy((*new_record)->name, name);
    (*new_record)->salary = salary;
    (*new_record)->next = NULL;
}

void insert_record(hashRecord** head, uint32_t hash, const char* name, uint32_t salary)
{
    if (*head == NULL)
    {
        get_record(head, hash, name, salary);
        return;
    }

    // Inserting as new root
    if ((*head)->hash > hash)
    {
        printf("how many time this happens\n");
        hashRecord **new_head = (hashRecord**)malloc(sizeof(hashRecord*));
        get_record(new_head, hash, name, salary);
        (*new_head)->next = *head;
        *head = *new_head;
        free (new_head);
        return;
    }
    hashRecord *temp = *head;
    hashRecord *prev = *head;
    // Traverse and update existing node if it exists
    while (temp != NULL && temp->hash <= hash) {
        if (temp->hash == hash)
        {
            temp->salary = salary;
            return;
        }
        prev = temp;
        temp = temp->next;
    }

    get_record(&prev->next, hash, name, salary);
    prev->next->next = temp;

    return;
}

void delete_record(hashRecord** head, uint32_t hash)
{
    hashRecord *temp;
    if ((*head)->hash == hash) {
        temp = *head;
        *head = (*head)->next;
        free(temp);
    } else {
        hashRecord *current = *head;
        while(current->next != NULL) {
            if (current->next->hash == hash) {
                temp  = current->next;
                current->next = current->next->next;
                free(temp);
                break;
            } else {
                current = current->next;
            }
        }
    }


}

hashRecord* search_record(hashRecord** head, uint32_t hash)
{
    hashRecord *temp = *head;
    while (temp != NULL) {
        if (temp->hash == hash) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

void free_table(hashRecord* head) {
    hashRecord *temp = head;
    while (temp != NULL) {
        head = temp;
        temp = temp->next;
        free(head);
    }
}