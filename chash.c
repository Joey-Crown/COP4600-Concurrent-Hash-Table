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
    hashRecord** head;
    char name[50];
    uint32_t salary;
    FILE *out;
} thread_args;

void printRecord(hashRecord *rec, FILE* out) {
    fprintf(out,"%u,%s,%u\n", rec->hash, rec->name, rec->salary);
}

void *insert(void *ptr) {
    thread_args *args = (thread_args*)ptr;
    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t*)args->name, strlen(args->name));
    fprintf(args->out,"INSERT,%u,%s,%u\n", hash, (char*) args->name, (uint32_t)args->salary);

    rwlock_acquire_writelock(&mutex);
    fprintf(args->out,"WRITE LOCK ACQUIRED\n");
    insert_record(args->head, hash, args->name, args->salary);
    rwlock_release_writelock(&mutex);
    fprintf(args->out,"WRITE LOCK RELEASED\n");

    free(args);
    return NULL;
}

void *delete(void *ptr) {
    thread_args *args = (thread_args*)ptr;
    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t*)args->name, strlen(args->name));
    fprintf(args->out,"DELETE,%s\n",args->name);

    rwlock_acquire_writelock(&mutex);
    fprintf(args->out,"WRITE LOCK ACQUIRED\n");
    delete_record(args->head, hash);
    rwlock_release_writelock(&mutex);
    fprintf(args->out,"WRITE LOCK RELEASED\n");

    free(args);
    return NULL;
}

void *search(void *ptr) {
    thread_args *args = (thread_args*)ptr;
    uint32_t hash = jenkins_one_at_a_time_hash((uint8_t*)args->name, strlen(args->name));
    fprintf(args->out,"SEARCH,%s\n", args->name);

    rwlock_acquire_readlock(&mutex);
    fprintf(args->out,"READ LOCK ACQUIRED\n");
    hashRecord* found = search_record(args->head, hash);
    rwlock_release_readlock(&mutex);
    fprintf(args->out,"READ LOCK RELEASED\n");

    if (found == NULL) {
        fprintf(args->out,"No Record Found\n");
    } else {
        fprintf(args->out,"%u,%s,%u\n", found->hash, found->name, found->salary);
    }

    free(args);
    return NULL;
}

void *print(void *ptr) {
    thread_args *args = (thread_args*)ptr;
    hashRecord *temp = *(args->head);

    rwlock_acquire_readlock(&mutex);
    fprintf(args->out,"READ LOCK ACQUIRED\n");
    // TODO NEEDS TO PRINT IN SORTED ORDER
    while (temp != NULL) {
        printRecord(temp, args->out);
        temp = temp->next;
    }
    rwlock_release_readlock(&mutex);
    fprintf(args->out,"READ LOCK RELEASED\n");

    free(args);
    return NULL;
}


int main(int argc, char *argv[]) 
{
    FILE *fp = fopen("commands.txt", "r");
    FILE *output = fopen("output.txt", "w");
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
        fprintf(output,"Running %d threads\n", num_threads);
    }

    rwlock_init(&mutex);

    hashRecord** table = (hashRecord**)malloc(sizeof(hashRecord*));
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
        th_args->out = output;
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

    fprintf(output,"Number of lock acquisitions: %d\n", mutex.acq_count);
    fprintf(output,"Number of lock releases: %d\n", mutex.rel_count);

    // Program shutdown
    hashRecord *temp = *table;
    while (temp != NULL) {
        printRecord(temp, output);
        temp = temp->next;
    }

    free_table(*table);
    free(table);
    free(th);
	if (fclose(fp) || fclose(output))
    {
        perror("commands.txt still open - mem leak occurrance");
        return (-1);
    }
    
    return 0;
}
