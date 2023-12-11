/**
 * `multitask.c`: a simple example to demonstrate the benefits of parallelism.
 *
 * Author: Paul Karhnak (pdevkarhnak@gmail.com)
 * For: Paul's Online CS Notes
 * Last updated: December 11, 2023
 *
 * Copyright 2023 Paul D. Karhnak
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER 
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */

// For serial, compile with: `gcc -O2 multitask.c -o multitask -lm -lpthread`
// For parallel, compile with: `gcc -O2 -DPARALLEL multitask.c -o multitask -lm -lpthread`

#include <pthread.h>	// link with `-lpthread`: provides `pthread_create`, `pthread_join`, mutex functions, etc.
#include <stdlib.h>	
#include <stdio.h>
#include <math.h>	// for `sqrtl`; link with `-lm`

// Allow code to branch based on compilation for parallelism (i.e., `-DPARALLEL` option) or serial
#ifdef PARALLEL
#define THREADS 4
#endif

#ifdef PARALLEL
typedef struct thread_info {
    long double* full_sum_ref;		// intended as pointer to heap variable (this is thread-safe as opposed to a global variable)
    pthread_mutex_t* sum_lock_ref;	// a pointer to a mutex that will reside on the stack of the thread which runs `main`
    long double start;			// the point along the domain where each thread starts taking square roots
    long double stop;			// the point along the domain where each thread stops taking square roots
} info;

void* sum_func(void* thread_args);	// function prototype; define later
#endif


int main() {

    long double max = 10000000000.0;	// 10 billion
    long double final_sum = 0.0;

    // One branch of the code: if compiled to enable parallelism...
    #ifdef PARALLEL
    pthread_mutex_t full_sum_lock;
    pthread_mutex_init(&full_sum_lock, NULL);

    // "Manager thread" (thread running `main` itself) initializes heap space for common sum
    long double* full_sum_ptr = (long double*)calloc(1, sizeof(long double));

    // Two arrays: one for the thread arguments, the other for the thread IDs of type `pthread_t`
    info thread_params[THREADS];
    pthread_t thread_ids[THREADS];

    // Sets up thread arguments: give each thread pointers to the common sum and mutex, then divide work up and "provision" accordingly.
    for (long i = 0; i < THREADS; i += 1) {
	thread_params[i].full_sum_ref = full_sum_ptr;
	thread_params[i].sum_lock_ref = &full_sum_lock;

	// A simple division of work here since we're crafting our example to have a "clean" (uniform) provision for each thread.
	// More generally, the division of work is a "map" step in the "map-reduce" parallel programming paradigm.
	thread_params[i].start = (((long double) i) * (max / THREADS)) + 1.0;
	thread_params[i].stop = ((long double) i + 1.0) * (max / THREADS);
    }	

    // Here, actually do the work of "spawning" the threads (give each thread something to run, then launch them)
    for (int i = 0; i < THREADS; i += 1) {
	pthread_create(&thread_ids[i], NULL, sum_func, (void*) &(thread_params[i]));
    }

    // Why do this separately? create-then-join in the same thread spawns threads *and* waits until done one at a time, which defeats the point (just makes it serial)
    for (int i = 0; i < THREADS; i += 1) {
	pthread_join(thread_ids[i], NULL);
    }

    pthread_mutex_destroy(&full_sum_lock);	// cleanup for mutex, which is a specialized "object"

    final_sum = *full_sum_ptr;

    free(full_sum_ptr);
   
    #else 
    // serial implementation here
    
    long double local_sum = 0;

    // just start taking square roots from 1 ... 10,000,000,000 and add them up.
    for (long double i = 1.0; i <= max; i += 1.0) {
	local_sum += sqrtl(i);
    }

    final_sum = local_sum;

    #endif

    // The answers shouldn't differ tremendously, but may differ somewhat.
    printf("Final sum of square root of 10,000,000,000 numbers is: %Lf\n", final_sum);

}

// Like branch above: if compiled to enable parallelism...
#ifdef PARALLEL
void* sum_func(void* thread_args) {

    // Beginning of "worker thread" work. Manager spawns and waits; workers actually do the raw computation needed to get the requested result.

    // Some rather inelegant typecasting necessary here. 
    // `pthread_create` specifically needs a `void*` function with a `void*` argument, so cast around that to make arg useful.
    info* this_thread_args = (info*) thread_args;

    // Read out arguments from the struct to set up this thread's work.
    long double* sum_ptr = this_thread_args->full_sum_ref;
    pthread_mutex_t* lock_ptr = this_thread_args->sum_lock_ref;

    long double this_thread_start = this_thread_args->start;
    long double this_thread_stop = this_thread_args->stop;

    // The specific sum that this thread computes. Step 1 is adding to this local sum; step 2 is combining results into the shared (common) sum.
    // More generally, an example of a "reduction" in what's termed a "map-reduce" parallel programming paradigm.
    long double local_sum = 0;

    for (long double i = this_thread_start; i <= this_thread_stop; i += 1) {
	local_sum += sqrtl(i);
    }

    // Beginning of "critical section": thread adds to the common sum once done with above loop.
    pthread_mutex_lock(lock_ptr);	// Only one thread can get this lock at once; otherwise, wait until available.
    *sum_ptr += local_sum;		// Addition is "atomic": threads can't interrupt each other since this is guarded by mutex locking.
    pthread_mutex_unlock(lock_ptr);	// All done; let other threads take their turn if they haven't yet.

    return NULL; // return value isn't really important here. We put our results on the heap where the "manager thread" can see.

}
#endif

