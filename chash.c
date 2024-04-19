#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "hashdb.h"
#include "rwlock.h"

#ifdef linux
#include <semaphore.h>
#endif

#define LINEMAX 1024

enum hash_table_commands {
    THREADS,
    INSERT,
    DELETE,
    SEARCH,
    PRINT
};

static const char *COMMAND_TYPE[5] = {
    "threads", "insert", "delete", "search", "print"
};

rwlock_t mutex;

typedef struct thread_args_struct {
    hashRecord* head;
    char name[50];
    uint32_t salary;
} thread_args;

void printRecord(hashRecord *rec) {
    printf("%u,%s,%u\n", rec->hash, rec->name, rec->salary);
}

void *insert(void *ptr) {
    thread_args *args = (thread_args*)ptr;
    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t*)args->name, strlen(args->name));
    printf("INSERT,%u,%s,%u\n", hash, (char*) args->name, (uint32_t)args->salary);

    rwlock_acquire_writelock(&mutex);
    printf("WRITE LOCK ACQUIRED\n");
    insert_record(args->head, hash, args->name, args->salary);
    rwlock_release_writelock(&mutex);
    printf("WRITE LOCK RELEASED\n");

    free(args);
    return NULL;
}

void *delete(void *ptr) {
    thread_args *args = (thread_args*)ptr;
    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t*)args->name, strlen(args->name));
    printf("DELETE,%s\n",args->name);

    rwlock_acquire_writelock(&mutex);
    printf("WRITE LOCK ACQUIRED\n");
    delete_record(args->head, hash);
    rwlock_release_writelock(&mutex);
    printf("WRITE LOCK RELEASED\n");
    free(args);
    return NULL;
}

void *search(void *ptr) {
    thread_args *args = (thread_args*)ptr;
    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t*)args->name, strlen(args->name));
    printf("SEARCH,%s\n", args->name);

    rwlock_acquire_readlock(&mutex);
    printf("READ LOCK ACQUIRED\n");
    hashRecord* found = search_record(args->head, hash);
    rwlock_release_readlock(&mutex);
    printf("READ LOCK RELEASED\n");

    if (found == NULL) {
        printf("No Record Found\n");
    } else {
        printf("%u,%s,%u\n", found->hash, found->name, found->salary);
    }

    free(args);
    return NULL;
}

void *print(void *ptr) {
    thread_args *args = (thread_args*)ptr;
    hashRecord *temp = args->head;

    rwlock_acquire_readlock(&mutex);
    printf("READ LOCK ACQUIRED\n");
    // TODO NEEDS TO PRINT IN SORTED ORDER
    while (temp != NULL) {
        printRecord(temp);
        temp = temp->next;
    }
    rwlock_release_readlock(&mutex);
    printf("READ LOCK RELEASED\n");

    free(args);
    return NULL;
}


int main(int argc, char *argv[]) 
{
    FILE *fp = fopen("commands.txt", "r");
    char cmd_buff[LINEMAX];
    char *token;
    char *name;
    char *salary_buff;
    uint32_t salary;
    const char delim[2] = ",";
    int num_threads = 0;

    if (fp == NULL) 
    {
        perror("Error opening commands.txt");
        return (-1);
    }
    
    // Ensure valid input and get num threads
    if (fgets(cmd_buff, LINEMAX, fp) != NULL) 
    {
        token = strtok(cmd_buff, delim);

        if (strcmp(token, COMMAND_TYPE[THREADS]) != 0) {
            perror("First command not \"threads\"");
            return (-1);
        }

        token = strtok(NULL, delim);
        num_threads = atoi(token);
        printf("Number of threads: %d\n", num_threads);
    }

    rwlock_init(&mutex);

    hashRecord* table = (hashRecord*)malloc(sizeof(hashRecord));
    pthread_t *th = (pthread_t*)malloc(sizeof(pthread_t)*num_threads);
    for (size_t i = 0; i < num_threads; i++)
    {
        // Parse line for command
        fgets(cmd_buff, LINEMAX, fp);
        token = strtok(cmd_buff, delim);
        name = strtok(NULL, delim);
        salary_buff = strtok(NULL, delim);
        salary = atoi(salary_buff);

        // Create a thread argument struct
        thread_args *th_args = (thread_args*)malloc(sizeof(thread_args));
        th_args->head = table;
        strcpy(th_args->name, name);
        th_args->salary = salary;
        if (strcmp(token, COMMAND_TYPE[INSERT]) == 0)
        {
            pthread_create(&th[i], NULL, insert, (void*)th_args);
        }

        else if (strcmp(token, COMMAND_TYPE[DELETE]) == 0)
        {
            pthread_create(&th[i], NULL, delete, (void*)th_args);
        }

        else if (strcmp(token, COMMAND_TYPE[SEARCH]) == 0)
        {
            pthread_create(&th[i], NULL, search, (void*)th_args);
        }

        else if (strcmp(token, COMMAND_TYPE[PRINT]) == 0)
        {
            pthread_create(&th[i], NULL, print, (void*)th_args);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(th[i], NULL);
    }

    free(th);
    return 0;
}