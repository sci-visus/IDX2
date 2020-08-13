#pragma once

namespace idx2 {

struct mutex;
/*
RAII struct that acquires the mutex on construction and releases it on destruction */
struct lock;

/*
Block until the lock can be acquired then return true.
Return false if something goes wrong.  */
bool Lock(mutex* Mutex);
/* Return immediately: true if the lock can be acquired, false if not */
bool TryLock(mutex* Mutex);
/* Return true if the lock can be released. Return false if something goes wrong. */
bool Unlock(mutex* Mutex);

}

#include "mg_macros.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace idx2 {
struct mutex {
  CRITICAL_SECTION Crit;
  mutex();
  ~mutex();
};

struct lock {
  mutex* Mutex = nullptr;
  lock(mutex* MutexIn);
  ~lock();
};

mg_Inline mutex::
mutex() { InitializeCriticalSection(&Crit); }

mg_Inline mutex::
~mutex() { DeleteCriticalSection(&Crit); }

mg_Inline lock::
lock(mutex* MutexIn) : Mutex(MutexIn) { Lock(Mutex); }

mg_Inline lock::
~lock() { Unlock(Mutex); }

mg_Inline bool
Lock(mutex* Mutex) {
  EnterCriticalSection(&Mutex->Crit); // TODO: handle exception
  return true;
}

mg_Inline bool
TryLock(mutex* Mutex) { return TryEnterCriticalSection(&Mutex->Crit) != 0; }

mg_Inline bool
Unlock(mutex* Mutex) {
  LeaveCriticalSection(&Mutex->Crit); // TODO: handle exception
  return true;
}

} // namespace idx2
#elif defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
namespace idx2 {
struct mutex {
  pthread_mutex_t Mx;
};

struct lock {
  mutex* Mutex = nullptr;
  lock(mutex* Mutex);
  ~lock();
};

mg_Inline lock::
lock(mutex* Mutex) : Mutex(Mutex) { Lock(Mutex); }

mg_Inline lock::
~lock() { Unlock(Mutex); }

mg_Inline bool
Lock(mutex* Mutex) { return pthread_mutex_lock(&Mutex->Mx) == 0; }

mg_Inline bool
TryLock(mutex* Mutex) { return pthread_mutex_trylock(&Mutex->Mx) == 0; }

mg_Inline bool
Unlock(mutex* Mutex) { return pthread_mutex_unlock(&Mutex->Mx) == 0; }

} // namespace idx2
#endif

