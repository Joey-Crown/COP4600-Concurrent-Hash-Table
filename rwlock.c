#include "rwlock.h"
#include "common.h"
#include "common_threads.h"


void rwlock_init(rwlock_t *lock) {
    lock->readers = 0;
    lock->acq_count = 0;
    lock->rel_count = 0;
    Sem_init(&lock->lock, 1);
    Sem_init(&lock->writelock, 1);
}

void rwlock_acquire_readlock(rwlock_t *lock) {
    Sem_wait(&lock->lock);
    lock->readers++;
    if (lock->readers == 1)
	    Sem_wait(&lock->writelock);
    Sem_post(&lock->lock);
    lock->acq_count++;
}

void rwlock_release_readlock(rwlock_t *lock) {
    Sem_wait(&lock->lock);
    lock->readers--;
    if (lock->readers == 0)
	    Sem_post(&lock->writelock);
    Sem_post(&lock->lock);
    lock->rel_count++;
}

void rwlock_acquire_writelock(rwlock_t *lock) {
    Sem_wait(&lock->writelock);
    lock->acq_count++;
}

void rwlock_release_writelock(rwlock_t *lock) {
    Sem_post(&lock->writelock);
    lock->rel_count++;
}