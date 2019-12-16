/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/********************************************
 * File:   main.c							*
 * Author: Aliaksei Prakapenka   1701212	*
 *											*
 * Created on 20. helmikuuta 2019, 9:51		*
 *******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//define return status in order to make code more readable
#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0

// reader and writer threads will increment these values
int sum_of_successful_reads = 0;
int sum_of_failed_reads = 0;
int sum_of_successful_writes = 0;
int sum_of_failed_writes = 0;
char buffer[10] = "123456789";  // max is 9 chars because trailing '\0' should fit
int reader_thread_count = 0;
int writer_thread_count = 0;
int read_loops = 0;
int write_loops = 0;

//initialize a mutex with default attributes
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//method that fills an array using random symbols
void write_random_data(char* buf, int len){
    int i;
    for(i=0;i<len;i++){
        buf[i] = rand()%25 + 65; // A..Z
    }
}

// ----------------------------------------------------------------------------------------
// conditional compilation flag MUTEX_IMPLEMENTATION
// if commented out --> reader_writer_version
//#define MUTEX_IMPLEMENTATION
// declare global data buffer and other global variables if needed


#ifdef MUTEX_IMPLEMENTATION

// declare global mutexes
pthread_mutex_t rw_mutex = PTHREAD_MUTEX_INITIALIZER;
int read_count = 0;
// thread functions
void* reader(void* args){
    int i;
    int fail = 0;
    for(i=0; i<read_loops; i++){
        // entering critical section
        fail = 0;
        pthread_mutex_lock(&mutex);
        read_count++;
        if(read_count == 1){    // if the first reader..
                        
            if(pthread_mutex_trylock(&rw_mutex)!=SUCCESS){   // if function returns 0 it means "Success", in other case mutex cant be locked by reader
                fail = 1;
                sum_of_failed_reads++;
            }
        }
        pthread_mutex_unlock(&mutex);
        
        if(fail==0){
            pthread_mutex_lock(&mutex);
            sum_of_successful_reads++;
            pthread_mutex_unlock(&mutex);
        
            // now we are int the critical section ready to read buffer data
            printf("Reading: %s\n",buffer);
        }
        // exit from critical section
        pthread_mutex_lock(&mutex);
        read_count--;
        if(read_count == 0 && fail == 0)
            pthread_mutex_unlock(&rw_mutex);
        pthread_mutex_unlock(&mutex);        
        sleep(rand()%3);
    }
    return NULL;
}
void* writer(void* args){
    int i;
    int fail = 0;
    for(i=0; i<write_loops; i++){
        fail = 0;
        // enter critical section..
        if(pthread_mutex_trylock(&rw_mutex)!=SUCCESS){
            fail = 1;
            pthread_mutex_lock(&mutex);
            sum_of_failed_writes++;
            pthread_mutex_unlock(&mutex);
        }
        // writing, if not failed..
        if(fail==0){
            pthread_mutex_lock(&mutex);
            sum_of_successful_writes++;
            pthread_mutex_unlock(&mutex);
            
            printf("Writing... \n", i);
            write_random_data(buffer,10);
          
        }
        // exit critical section
        if(fail==0){
            pthread_mutex_unlock(&rw_mutex);
        }   
        sleep(rand()%3);
    }
    return NULL;
}

#else

// declare global reader_writer_locks
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t rw_mutex = PTHREAD_MUTEX_INITIALIZER;

// thread functions
void* reader(void* args){
    int i;
    int fail = 0;
    for(i=0;i<read_loops;i++){
        // entering critical section
        fail = 0;
        if((pthread_rwlock_tryrdlock(&rwlock))!=SUCCESS){   // if return 0 it means failed 
            fail = 1;
            pthread_mutex_lock(&mutex);
            sum_of_failed_reads++;
            pthread_mutex_unlock(&mutex);
        }
    
        if(fail==0){
            pthread_mutex_lock(&mutex);
            sum_of_successful_reads++;
            pthread_mutex_unlock(&mutex);
           
           // now we are in the critical section ready to read buffer data
            printf("read: %s\n",buffer);
            // exit from critical section
             pthread_rwlock_unlock(&rwlock);
        }
        pthread_yield();
    }
    return NULL;
}
void* writer(void* args){  
    int i;
    int fail=0;
    for(i=0;i<write_loops;i++){
        fail = 0;
        // enter critical section..
        if((pthread_rwlock_trywrlock(&rwlock))!=SUCCESS){
            fail = 1;
            pthread_mutex_lock(&mutex); //locking the counter
            sum_of_failed_writes++;
            pthread_mutex_unlock(&mutex);
            
        }
        // writing, if not failed..
        if(fail==0){
            pthread_mutex_lock(&mutex);
            sum_of_successful_writes++;
            pthread_mutex_unlock(&mutex);
            printf("Writing...\n");
            write_random_data(buffer,10);
            pthread_rwlock_unlock(&rwlock);
        }
        pthread_yield();
    }
    return NULL;
}
#endif


int main(int argc, char* argv[]){
    // first check command line arguments
    // 1) should be 5 (prog + 4 actual
    if(argc != 5){
        printf("usage: <prog> <wr_thread_count> <rd_thread_count> <wr_loops> <rd_loops>\n");
        exit(-1);
    }
   
    // 2) all argv[1]..argv[4] ints?
    writer_thread_count = atoi(argv[1]);
    reader_thread_count = atoi(argv[2]);
    write_loops = atoi(argv[3]);
    read_loops = atoi(argv[4]);
    int status;
    if(writer_thread_count<=0 || reader_thread_count<=0 || write_loops<=0 || read_loops<=0){
        printf("illegal arguments!\n");
        exit(-1);
    }
    // declare and initalize threads and thread attrs
    pthread_attr_t pat; // this attr can be used in every threads
    pthread_t writers[writer_thread_count];
    pthread_t readers[reader_thread_count];
    pthread_attr_init(&pat);
    
    // create writer and reader threads
    int i;
    
    for(i=0; i<reader_thread_count; i++)
    {
        status = pthread_create(&readers[i], &pat, reader, NULL);
        if (status != SUCCESS) 
        {
            printf("main error: can't create thread, status = %d\n", status);
            exit(ERROR_CREATE_THREAD);
        }
    }
    for(i=0; i<writer_thread_count; i++)
    {
        status = pthread_create(&writers[i], &pat, writer, NULL);
        if (status != 0) 
        {
            printf("main error: can't create thread, status = %d\n", status);
            exit(ERROR_CREATE_THREAD);
        }
    } 
    // join them.. one by one..
    
    for(i=0; i<reader_thread_count; i++)
    {
        pthread_join(readers[i], NULL);
    }
    
    for(i=0; i<writer_thread_count; i++)
    {
        pthread_join(writers[i], NULL);
    }
    
    printf("Successful reads=%d, failed_reads %d\n",sum_of_successful_reads, sum_of_failed_reads);
    printf("Total number of reads: %d\n",sum_of_successful_reads + sum_of_failed_reads);
    
    printf("Successful writes=%d, failed_writes %d\n",sum_of_successful_writes, sum_of_failed_writes);
    printf("Total number of writes %d\n",sum_of_successful_writes+sum_of_failed_writes);
    
    return 0;
}
// ----------------------------------------------------------------------------------------