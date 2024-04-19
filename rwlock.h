#ifndef RWLOCK_H
#define RWLOCK_H

#include <stddef.h>

#include <semaphore.h>

typedef struct _rwlock_t {
    sem_t writelock;
    sem_t lock;
    int readers;
    int acq_count;
    int rel_count;
} rwlock_t;

void rwlock_init(rwlock_t *lock);
void rwlock_acquire_readlock(rwlock_t *lock);
void rwlock_release_readlock(rwlock_t *lock);
void rwlock_acquire_writelock(rwlock_t *lock);
void rwlock_release_writelock(rwlock_t *lock);

#endif