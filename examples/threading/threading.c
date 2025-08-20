#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param){

struct thread_data* thread_func_args = (struct thread_data*) thread_param;


    
    usleep(thread_func_args->wait_to_obtain_ms*1000);

    
    int n = pthread_mutex_lock(thread_func_args->mutex);
  

    if(n !=0)
    {
      thread_func_args->thread_complete_success = false;
       ERROR_LOG("mutex  lock failed");
      pthread_exit(thread_func_args);

    }
    usleep(thread_func_args->wait_to_release_ms*1000);
       
    n = pthread_mutex_unlock(thread_func_args->mutex); 

    if(n !=0)
    {
      thread_func_args->thread_complete_success = false;
       ERROR_LOG("mutex  unlockfailed");
      pthread_exit(thread_func_args);

    }
    thread_func_args->thread_complete_success = true;
    pthread_exit(thread_func_args);

    return thread_param;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{

struct thread_data *data = (struct thread_data*)malloc(sizeof(struct thread_data));
  
    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms; 
    

     int rc = pthread_create(thread,NULL,threadfunc,(void*)data);

    if(rc == 0)
    {
         printf("Thread created sucessfully\n");

        return true;	
    }

  free(data);
  return false;
}
