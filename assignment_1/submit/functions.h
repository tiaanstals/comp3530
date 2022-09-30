#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void create_signal_threads(int time_green, int min_interval, pthread_mutex_t *mutex_dir_1, 
pthread_mutex_t *mutex_dir_2, pthread_cond_t *controller_signals_dir_1, 
pthread_cond_t *controller_signals_dir_2, pthread_cond_t *controller_waits_dir_1,
            pthread_cond_t *controller_waits_dir_2, int *counter_dir_1, int *counter_dir_2, 
            pthread_mutex_t *counter_mutex_dir_1, pthread_mutex_t *counter_mutex_dir_2);
            
int uniform_distribution(int rangeLow, int rangeHigh);

char *get_current_time();

void * sleep_and_signal(void *arg);

void *print_lights(int color, char* directions, int debug_mode);

void *print_vehicle_proceeding(int id, char* direction, int debug_mode);
