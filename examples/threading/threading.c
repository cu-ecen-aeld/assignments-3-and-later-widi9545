#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)


// TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
// hint: use a cast like the one below to obtain thread arguments from your parameter
//struct thread_data* thread_func_args = (struct thread_data *) thread_param;
// https://man7.org/linux/man-pages/man3/pthread_exit.3.html
// https://man7.org/linux/man-pages/man3/pthread_create.3.html - thr struct into restrict

void* threadfunc(void* thread_param)
{
    printf("hello!");
    
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    usleep(1000 * thread_func_args -> wait_to_obtain_ms);
    
    
    int lock = pthread_mutex_lock(thread_func_args -> mutex);
    if(lock != 0){
        pthread_mutex_destroy(thread_func_args -> mutex);
        pthread_exit(thread_param);
    }
    usleep(1000 * thread_func_args -> wait_to_release_ms);


    int release = pthread_mutex_unlock(thread_func_args -> mutex);
    
    if(release != 0){
        pthread_mutex_destroy(thread_func_args -> mutex);
        pthread_exit(thread_param);
    }
    thread_func_args -> thread_complete_success = true;
 

    pthread_exit(thread_param);


}
/**
* TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
* using threadfunc() as entry point.
*
* return true if successful.
*
* See implementation details in threading.h file comment block
*/

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data *thr = (struct thread_data *)malloc(sizeof(struct thread_data));
    thr -> wait_to_obtain_ms = wait_to_obtain_ms;
    thr -> wait_to_release_ms = wait_to_release_ms;
    thr -> mutex = mutex;
    
    int error = pthread_create(thread, NULL, threadfunc, thr);
    if(error != 0){
        printf("this is the return status %d", error);
        free(thr);
        return false;
    }

    return true;
}

