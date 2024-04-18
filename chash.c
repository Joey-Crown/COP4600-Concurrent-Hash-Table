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

    hashRecord* table = (hashRecord*)malloc(sizeof(hashRecord));
    uint32_t hash;
    //pthread_t *th = (pthread_t*)malloc(sizeof(pthread_t)*num_threads);
    for (size_t i = 0; i < num_threads; i++) 
    {
        fgets(cmd_buff, LINEMAX, fp);
        token = strtok(cmd_buff, delim);
        name = strtok(NULL, delim);
        if (strcmp(token, COMMAND_TYPE[INSERT]) == 0)
        {
            salary_buff = strtok(NULL, delim);
            salary = atoi(salary_buff);
            hash = jenkins_one_at_a_time_hash((uint8_t*)(name), strlen(name));
            printf("INSERT,%s,%d,Hash:%u\n", name, salary, hash);
            insert_record(table, hash, name, salary);
        }

        else if (strcmp(token, COMMAND_TYPE[DELETE]) == 0)
        {
            printf("DELETE,%s\n", name);
            hash = jenkins_one_at_a_time_hash((uint8_t*)(name), strlen(name));
            delete_record(table, hash);
        }

        else if (strcmp(token, COMMAND_TYPE[SEARCH]) == 0)
        {
            printf("SEARCH,%s\n", name);
            hash = jenkins_one_at_a_time_hash((uint8_t*)(name), strlen(name));
            hashRecord* found = search_record(table, hash);
            if (found == NULL) {
                printf("No Record Found\n");
            } else {
                printf("%u,%s,%u\n", found->hash, found->name, found->salary);
            }
        }

        else if (strcmp(token, COMMAND_TYPE[PRINT]) == 0)
        {
            printf("PRINT\n");
        }
    }

    return 0;
}