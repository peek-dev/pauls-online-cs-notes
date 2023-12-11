/**
 * `multitask.c`: a simple example to demonstrate the benefits of parallelism.
 *
 * Author: Paul Karhnak (pdevkarhnak@gmail.com)
 * For: Paul's Online Math Notes
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

#include <pthread.h>	// link with `-lpthread`
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Allow code to branch based on compilation for parallelism (i.e., `-DPARALLEL` option) or serial
#ifdef PARALLEL
#define THREADS 4
#endif

#ifdef PARALLEL
typedef struct thread_info {
    long double* full_sum_ref;		// intended as pointer to heap variable (this is thread-safe as opposed to a global variable)
    pthread_mutex_t* sum_lock_ref;	// a pointer to a mutex that will reside on the stack of the thread which runs `main`
    long double start;	// the point along the domain where each thread starts integrating
    long double stop;	// the point along the domain where each thread stops integrating
} info;

void* sum_func(void* thread_args);
#endif


int main() {

    long double max = 10000000000.0;
    long double final_sum = 0.0;

    #ifdef PARALLEL
    pthread_mutex_t full_sum_lock;
    pthread_mutex_init(&full_sum_lock, NULL);

    long double* full_sum_ptr = (long double*)calloc(1, sizeof(long double));

    info thread_params[THREADS];
    pthread_t thread_ids[THREADS];

    for (long i = 0; i < THREADS; i += 1) {
	thread_params[i].full_sum_ref = full_sum_ptr;
	thread_params[i].sum_lock_ref = &full_sum_lock;
	thread_params[i].start = (((long double) i) * (max / THREADS)) + 1.0;
	thread_params[i].stop = ((long double) i + 1.0) * (max / THREADS);
    }	

    for (int i = 0; i < THREADS; i += 1) {
	pthread_create(&thread_ids[i], NULL, sum_func, (void*) &(thread_params[i]));
    }

    for (int i = 0; i < THREADS; i += 1) {
	pthread_join(thread_ids[i], NULL);
    }

    pthread_mutex_destroy(&full_sum_lock); 

    final_sum = *full_sum_ptr;

    free(full_sum_ptr);
    #else 

    // put serial code here
    
    long double local_sum = 0;

    for (long double i = 1.0; i <= max; i += 1.0) {
	local_sum += sqrtl(i);
    }

    final_sum = local_sum;

    #endif

    printf("Final sum of square root of 1,000,000,000 numbers is: %Lf\n", final_sum);

}

#ifdef PARALLEL
void* sum_func(void* thread_args) {

    info* this_thread_args = (info*) thread_args;

    long double* sum_ptr = this_thread_args->full_sum_ref;
    pthread_mutex_t* lock_ptr = this_thread_args->sum_lock_ref;

    long double this_thread_start = this_thread_args->start;
    long double this_thread_stop = this_thread_args->stop;

    long double local_sum = 0;

    for (long double i = this_thread_start; i <= this_thread_stop; i += 1) {
	local_sum += sqrtl(i);
    }

    pthread_mutex_lock(lock_ptr);
    *sum_ptr += local_sum;
    pthread_mutex_unlock(lock_ptr);

    return NULL;

}
#endif

