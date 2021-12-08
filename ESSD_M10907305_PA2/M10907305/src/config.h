#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <sched.h>

#define PART 2

// Hardware dependency parameter
#define CORE_NUM 4
#define THREAD_NUM 4


// Workload parameter
#define PROGRAM_NUM 3
#define MATRIX_SIZE 500
#define MULTI_TIME 3


// Protect shared resource method
#define MUTEX 0
#define SPINLOCK 1

#define PROTECT_SHARED_RESOURCE MUTEX


// Synchronize method
#define BARRIER 0
#define SEMAPHORE 1

#define SYNCHRONIZE SEMAPHORE

#define pshared PTHREAD_PROCESS_SHARED
// #define pshared PTHREAD_PROCESS_PRIVATE

#endif
