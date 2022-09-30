
#include "functions.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>


typedef struct light_and_direction_object
{
    int time_green;
    int min_interval; 
    pthread_mutex_t *direction_mutex;
    pthread_cond_t *controller_waits;
    pthread_cond_t *controller_signals;
    pthread_mutex_t *counter_mutex;
    int *counter;
} sleep_helper_t;

void create_signal_threads(int time_green, int min_interval, pthread_mutex_t *mutex_dir_1, 
pthread_mutex_t *mutex_dir_2, pthread_cond_t *controller_signals_dir_1, 
pthread_cond_t *controller_signals_dir_2, pthread_cond_t *controller_waits_dir_1,
            pthread_cond_t *controller_waits_dir_2, int *counter_dir_1, int *counter_dir_2, 
            pthread_mutex_t *counter_mutex_dir_1, pthread_mutex_t *counter_mutex_dir_2) {
    /*
    -create 2 threads. Each thread is responsible for signalling the vehicles in one direction
    - and managing sleeping for the period allocated
    - As a failsafe, cancel the threads after the alloted green time period
    */
        pthread_t *direction_controller_thread; //system thread id
        sleep_helper_t *direction_threads; //user-defined thread id
        direction_controller_thread = malloc(3 * sizeof(pthread_t)); //system thread ids
        if (direction_controller_thread == NULL)
        {
            fprintf(stderr, "direction_controller_thread out of memory\n");
            exit(1);
        }
        direction_threads = malloc(3 * sizeof(sleep_helper_t)); //mini_controller objects
        if (direction_threads == NULL)
        {
            fprintf(stderr, "direction_thread out of memory\n");
            exit(1);
        }

        direction_threads[0].time_green = time_green;
        direction_threads[0].min_interval = min_interval;
        direction_threads[0].direction_mutex = mutex_dir_1;
        direction_threads[0].controller_signals = controller_signals_dir_1;
        direction_threads[0].controller_waits = controller_waits_dir_1;
        direction_threads[0].counter = counter_dir_1;
        direction_threads[0].counter_mutex = counter_mutex_dir_1;
        int thread_1;
		thread_1 = pthread_create(&direction_controller_thread[0], NULL, sleep_and_signal, (void *)&direction_threads[0]);
		if (thread_1) {
			printf("ERROR on signal thread");
			exit(-1);
		}

        direction_threads[1].time_green = time_green;
        direction_threads[1].min_interval = min_interval;
        direction_threads[1].direction_mutex = mutex_dir_2;
        direction_threads[1].controller_waits = controller_waits_dir_2;
        direction_threads[1].controller_signals = controller_signals_dir_2;
        direction_threads[1].counter = counter_dir_2;
        direction_threads[1].counter_mutex = counter_mutex_dir_2;
        int thread_2;
		thread_2 = pthread_create(&direction_controller_thread[1], NULL, sleep_and_signal, (void *)&direction_threads[1]);
		if (thread_2) {
			printf("ERROR on signal thread");
			exit(-1);
		}

        sleep(time_green);
        pthread_cancel(direction_controller_thread[0]);
        pthread_cancel(direction_controller_thread[1]);
        pthread_mutex_unlock(mutex_dir_1);
        pthread_mutex_unlock(mutex_dir_2);
        pthread_mutex_unlock(counter_mutex_dir_1);
        pthread_mutex_unlock(counter_mutex_dir_2);
        free(direction_controller_thread);
        free(direction_threads);

    }




void * sleep_and_signal(void *arg) {
    sleep_helper_t *sleep_obj = (sleep_helper_t*)arg;
    int min_interval = sleep_obj->min_interval;
    int *counter = sleep_obj->counter;
    
    //loop will be ended by cancel from spawning thread

    while (1) {
            //printf("time green %d\n", time_green_function);

            //firstly, check if there are any vheicles waiting
            pthread_mutex_lock(sleep_obj->counter_mutex);
            int vehicles_waiting = *(counter);
            pthread_mutex_unlock(sleep_obj->counter_mutex);
            //printf("vehicles waiting: %d\n", vehicles_waiting);

            //if there are vehicles waiting, pass them through according to the time interval
            if (vehicles_waiting) {
                pthread_mutex_lock(sleep_obj->direction_mutex);
                pthread_cond_signal(sleep_obj->controller_signals);
                //printf("signalled\n");
                pthread_mutex_unlock(sleep_obj->direction_mutex);
                pthread_mutex_lock(sleep_obj->counter_mutex);
                *counter = *counter - 1;
                pthread_mutex_unlock(sleep_obj->counter_mutex);
                sleep(min_interval);
                //if there arent vehicles waiting, wait for a vehicle to arrive
                //when a vehicle arrives, pass it through. 
            } else {
                //printf("wait state\n");
                pthread_mutex_lock(sleep_obj->direction_mutex);
                pthread_cond_wait(sleep_obj->controller_waits, sleep_obj->direction_mutex);
                //printf("have been signalled");
                pthread_cond_signal(sleep_obj->controller_signals);
                pthread_mutex_unlock(sleep_obj->direction_mutex);
                pthread_mutex_lock(sleep_obj->counter_mutex);
                *counter = *counter - 1; 
                pthread_mutex_unlock(sleep_obj->counter_mutex);
                sleep(min_interval);
            }
    }
    return (void*) NULL;
}

/*
 source - https://stackoverflow.com/questions/11641629/generating-a-uniform-distribution-of-integers-in-c
 */

int uniform_distribution(int rangeLow, int rangeHigh) {
    double myRand = rand()/(1.0 + RAND_MAX); 
    int range = rangeHigh - rangeLow + 1;
    int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}

char * get_current_time() {
    time_t current_time;
    char* c_time_string;
    /* Obtain current time. */
    current_time = time(NULL);

    if (current_time == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        exit(EXIT_FAILURE);
    }

    /* Convert to local time format. */
    c_time_string = ctime(&current_time);

    if (c_time_string == NULL)
    {
        (void) fprintf(stderr, "Failure to convert the current time.\n");
        exit(EXIT_FAILURE);
    }

    return c_time_string;
}

void *print_lights(int color, char *directions, int debug_mode) {
    // if color == 1, lights turn green
    if (color) {
        if (debug_mode) {
            char* current_time = get_current_time();
            printf("%.2s:lights:%s:green\n", &current_time[17], directions);
        } else {
            printf("The traffic lights %s have changed to green.\n", directions);
        }
    } else {
        if (debug_mode) {
            char* current_time = get_current_time();
            printf("%.2s:lights:%s:red\n", &current_time[17], directions);
        } else {
            printf("The traffic lights %s will change to red now.\n", directions);
        }
    }
    return (void*) NULL;
}

void *print_vehicle_proceeding(int id, char* direction, int debug_mode) {
    if (debug_mode) {
        char* current_time = get_current_time();
        printf("%.2s:vehicle:%d:%s:proceeding\n",&current_time[17], id, direction);
    } else {
        printf("Vehicle %d %s is proceeding through the intersection.\n", id, direction);
    }
    return (void*) NULL;
}

/**
 * Returns the current time in microseconds.
 */
long getMicrotime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}
