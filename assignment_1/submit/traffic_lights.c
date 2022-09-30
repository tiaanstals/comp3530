#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "functions.h"

//mini_controller and vehicle structs
//you may make changes if necessary

typedef struct mini_controller_object
{
    int id;
    int time_green;
	int min_interval;
} mini_cntrl_t;

typedef struct vehicle_object
{
    int id;
    char direction[4];
} vehicle_t;



void * mini_controller_routine(void *);
void * vehicle_routine(void *);


pthread_mutex_t green_mutex;
pthread_cond_t controller_1;
pthread_cond_t controller_2;
pthread_cond_t controller_3;
pthread_cond_t vehicles_waiting_on_c1_n2s;
pthread_cond_t controller_1_waiting_for_vehicles_n2s;
pthread_cond_t vehicles_waiting_on_c1_s2n;
pthread_cond_t controller_1_waiting_for_vehicles_s2n;
pthread_mutex_t controller_1_mutex_n2s;
pthread_mutex_t controller_1_mutex_s2n;
pthread_mutex_t n2s_mutex_counter;
pthread_mutex_t s2n_mutex_counter;
pthread_cond_t vehicles_waiting_on_c2_e2w;
pthread_cond_t controller_2_waiting_for_vehicles_e2w;
pthread_cond_t vehicles_waiting_on_c2_w2e;
pthread_cond_t controller_2_waiting_for_vehicles_w2e;
pthread_mutex_t controller_2_mutex_e2w;
pthread_mutex_t e2w_mutex_counter;
pthread_mutex_t controller_2_mutex_w2e;
pthread_mutex_t w2e_mutex_counter;
pthread_cond_t vehicles_waiting_on_c3_n2w;
pthread_cond_t controller_3_waiting_for_vehicles_n2w;
pthread_cond_t vehicles_waiting_on_c3_s2e;
pthread_cond_t controller_3_waiting_for_vehicles_s2e;
pthread_mutex_t controller_3_mutex_n2w;
pthread_mutex_t n2w_mutex_counter;
pthread_mutex_t controller_3_mutex_s2e;
pthread_mutex_t s2e_mutex_counter;
int debug_mode;

int *n2s_counter;
int *s2n_counter;
int *e2w_counter;
int *w2e_counter;
int *n2w_counter;
int *s2e_counter;

int last_vehicle_n2s = -1;
int last_vehicle_s2n = -1;
int last_vehicle_e2w = -1;
int last_vehicle_w2e = -1;
int last_vehicle_n2w = -1;
int last_vehicle_s2e = -1;



int main(int argc, char ** argv)
{
    int n_vehicles; //total number of vehicles
	int vehicle_rate; //vehicle arrival rate to the intersection
	int min_interval; // min time interval between two consecutive vehicles to pass the intersection
    debug_mode = 0;

    if( argc == 2 ) {
        if (!strcmp("debug", argv[1])) {
            debug_mode = 1;
        } else {
            printf("unknown argument");
        }
   }
	
	//more variable declarations
	pthread_t *mini_controller_thrd_id; //system thread id
    mini_cntrl_t *mini_controller; //user-defined thread id
    int g_t;
	
    // Initialize mutex and condition variables
    pthread_mutex_init(&green_mutex, NULL);

    
    pthread_mutex_init(&controller_1_mutex_n2s, NULL);
    pthread_mutex_init(&n2s_mutex_counter, NULL);

    pthread_mutex_init(&controller_1_mutex_s2n, NULL);
    pthread_mutex_init(&s2n_mutex_counter, NULL);

    pthread_mutex_init(&controller_2_mutex_e2w, NULL);
    pthread_mutex_init(&e2w_mutex_counter, NULL);

    pthread_mutex_init(&controller_2_mutex_w2e, NULL);
    pthread_mutex_init(&w2e_mutex_counter, NULL);

    pthread_mutex_init(&controller_3_mutex_n2w, NULL);
    pthread_mutex_init(&n2w_mutex_counter, NULL);

    pthread_mutex_init(&controller_3_mutex_s2e, NULL);
    pthread_mutex_init(&s2e_mutex_counter, NULL);

	pthread_cond_init(&controller_1, NULL);
	pthread_cond_init(&controller_2, NULL);
	pthread_cond_init(&controller_3, NULL);

    pthread_cond_init(&vehicles_waiting_on_c1_n2s, NULL);
    pthread_cond_init(&controller_1_waiting_for_vehicles_n2s, NULL);
    
    pthread_cond_init(&vehicles_waiting_on_c1_s2n, NULL);
    pthread_cond_init(&controller_1_waiting_for_vehicles_s2n, NULL);

    pthread_cond_init(&vehicles_waiting_on_c2_e2w, NULL);
    pthread_cond_init(&controller_2_waiting_for_vehicles_e2w, NULL);

    pthread_cond_init(&vehicles_waiting_on_c2_w2e, NULL);
    pthread_cond_init(&controller_2_waiting_for_vehicles_w2e, NULL);

	pthread_cond_init(&vehicles_waiting_on_c3_n2w, NULL);
    pthread_cond_init(&controller_3_waiting_for_vehicles_n2w, NULL);

    pthread_cond_init(&vehicles_waiting_on_c3_s2e, NULL);
    pthread_cond_init(&controller_3_waiting_for_vehicles_s2e, NULL);
    
	//allocate memory for mini_controllers 
	mini_controller_thrd_id = malloc(3 * sizeof(pthread_t)); //system thread ids
    if (mini_controller_thrd_id == NULL)
    {
        fprintf(stderr, "mini_controlle_thrds_id out of memory\n");
        exit(1);
    }
    mini_controller = malloc(3 * sizeof(mini_cntrl_t)); //mini_controller objects
    if (mini_controller == NULL)
    {
        fprintf(stderr, "mini_controller out of memory\n");
        exit(1);
    }

	// input parameters
    // ask for the total number of vehicles.
    printf("Enter the total number of vehicles (int): ");
    scanf("%d", &n_vehicles);

	//ask for vehicles' arrival rate to the intersection
	printf("Enter vehicles arrival rate (int): \n");
	scanf("%d", &vehicle_rate);
	
	//ask for the minimum time interval t (in seconds) between any two 
	//consecutive vehicles in one direction to pass through the intersection
	printf("Enter minimum interval between two consecutive vehicles (int): \n");
	scanf("%d", &min_interval);
	
    //ask for green time for each mini_controller
    printf("Enter green time for forward-moving vehicles on trunk road (int): ");
    scanf("%d", &g_t);
    mini_controller[0].time_green = g_t;
    printf("Enter green time for vehicles on minor road (int): ");
    scanf("%d", &g_t);
    mini_controller[1].time_green = g_t;	
    printf("Enter green time for right-turning vehicles on trunk road (int): ");
    scanf("%d", &g_t);
    mini_controller[2].time_green = g_t;
	

	//allocate memory for vehicles 
    pthread_t *v_thread_ids;
    vehicle_t *vehicle;

    //system based thread ids for vehicles
    v_thread_ids = malloc((n_vehicles) * sizeof(pthread_t)); 
	if(v_thread_ids == NULL){
		fprintf(stderr, "threads out of memory\n");
		exit(1);
	}	
	
    //create space for vehicle objects
	vehicle = malloc((n_vehicles) * sizeof(vehicle_t));
	if(vehicle == NULL){
		fprintf(stderr, "t out of memory\n");
		exit(1);
	}	
    //create mini_controller threads 
    int mini_controller_thread;
	for (int k = 0; k<3; k++)
    {
		mini_controller[k].id = k;
        mini_controller[k].min_interval = min_interval;
		mini_controller_thread = pthread_create(&mini_controller_thrd_id[k], NULL, mini_controller_routine, (void *)&mini_controller[k]);
		if (mini_controller_thread) {
			printf("ERROR; return code from pthread_create() (child) is %d\n", mini_controller_thread);
			exit(-1);
		}
    }
	
	//create vehicles threads

	int i;
    int previous_direction = 0;
    int direction;
    int sleep_time;
    int n2s_count, s2n_count, w2e_count, e2w_count, n2w_count, s2e_count;
    n2s_count = s2n_count = w2e_count = e2w_count = n2w_count = s2e_count = 0;
    
    

    n2s_counter = (int*)malloc(sizeof(int));
    *n2s_counter = 0;
    s2n_counter = (int*)malloc(sizeof(int));
    *s2n_counter = 0;
    e2w_counter = (int*)malloc(sizeof(int));
    *e2w_counter = 0;
    w2e_counter = (int*)malloc(sizeof(int));
    *w2e_counter = 0;
    n2w_counter = (int*)malloc(sizeof(int));
    *n2w_counter = 0;
    s2e_counter = (int*)malloc(sizeof(int));
    *s2e_counter = 0;
    //*s2n_counter = *n2s_counter = *e2w_counter = *w2e_counter = *n2w_counter = *s2e_counter = 0;
    int thread;
	for (i = 0; i < n_vehicles; i++)
    {
        sleep_time = (int)rand() % vehicle_rate;
        direction = uniform_distribution(0,5);
        if (i != 0) {
            if ((direction == previous_direction) && (sleep_time == 0)) {
                sleep_time = 1;
            }
        }
        sleep(sleep_time); 
		if (direction == 0){
			strcpy(vehicle[i].direction, "n2s");
			vehicle[i].id = n2s_count;
			n2s_count++;
		}
		else if (direction == 1){
			strcpy(vehicle[i].direction, "s2n");
			vehicle[i].id = s2n_count;
			s2n_count++;
		}
        else if (direction == 2){
			strcpy(vehicle[i].direction, "e2w");
			vehicle[i].id = e2w_count;
			e2w_count++;
		}
        else if (direction == 3){
			strcpy(vehicle[i].direction, "w2e");
			vehicle[i].id = w2e_count;
			w2e_count++;
		}
        else if (direction == 4){
			strcpy(vehicle[i].direction, "n2w");
			vehicle[i].id = n2w_count;
			n2w_count++;
		}
        else if (direction == 5){
			strcpy(vehicle[i].direction, "s2e");
			vehicle[i].id = s2e_count;
			s2e_count++;
		}
		thread = pthread_create(&v_thread_ids[i], NULL, vehicle_routine, (void *)&vehicle[i]);
		if (thread) {
			printf("ERROR; return code from pthread_create() (consumer) is %d\n", thread);
			exit(-1);
		}
    }
	
	
    
	//join and terminating threads.
    //first join all vehicle threads
	for (int z = 0; z < n_vehicles; z++) 
    {
		pthread_join(v_thread_ids[z], NULL);
    }
    printf("Main thread: There are no more vehicles to serve. The simulation will end now.\n");
    //then cancel mini controller threads
    for (int w = 0; w < 3; w++) 
    {
		pthread_cancel(mini_controller_thrd_id[w]);
    }
	
	

    //deallocate allocated memory
	free(vehicle);
    free(v_thread_ids);
    free(mini_controller);
    free(mini_controller_thrd_id);
    free(n2s_counter);
    free(s2n_counter);
    free(e2w_counter);
    free(w2e_counter);
    free(n2w_counter);
	free(s2e_counter);

	
    //destroy mutex and condition variable objects
    
	pthread_mutex_destroy(&green_mutex);
    pthread_mutex_destroy(&controller_1_mutex_n2s);
    pthread_mutex_destroy(&n2s_mutex_counter);
    
    pthread_mutex_destroy(&controller_1_mutex_s2n);
    pthread_mutex_destroy(&s2n_mutex_counter);

    pthread_mutex_destroy(&controller_2_mutex_e2w);
    pthread_mutex_destroy(&e2w_mutex_counter);

    pthread_mutex_destroy(&controller_2_mutex_w2e);
    pthread_mutex_destroy(&w2e_mutex_counter);

    pthread_mutex_destroy(&controller_3_mutex_n2w);
    pthread_mutex_destroy(&n2w_mutex_counter);

    pthread_mutex_destroy(&controller_3_mutex_s2e);
    pthread_mutex_destroy(&s2e_mutex_counter);

	pthread_cond_destroy(&controller_1);
	pthread_cond_destroy(&controller_2);
	pthread_cond_destroy(&controller_3);
	pthread_cond_destroy(&vehicles_waiting_on_c1_n2s);
    pthread_cond_destroy(&controller_1_waiting_for_vehicles_n2s);
    pthread_cond_destroy(&vehicles_waiting_on_c1_s2n);
    pthread_cond_destroy(&controller_1_waiting_for_vehicles_s2n);
	pthread_cond_destroy(&vehicles_waiting_on_c2_e2w);
    pthread_cond_destroy(&controller_2_waiting_for_vehicles_e2w);
    pthread_cond_destroy(&vehicles_waiting_on_c2_w2e);
    pthread_cond_destroy(&controller_2_waiting_for_vehicles_w2e);
	pthread_cond_destroy(&vehicles_waiting_on_c3_n2w);
    pthread_cond_destroy(&controller_3_waiting_for_vehicles_n2w);
    pthread_cond_destroy(&vehicles_waiting_on_c3_s2e);
    pthread_cond_destroy(&controller_3_waiting_for_vehicles_s2e);
	
    exit(0);
}

void * mini_controller_routine(void * arg)
{
    mini_cntrl_t *mini_controller = (mini_cntrl_t*)arg;
	int id = mini_controller->id;
    int time_green = mini_controller->time_green;
	int min_interval = mini_controller->min_interval;

    char *directions = malloc(10 * sizeof(char));

    if (id == 0) {
        directions = "(n2s, s2n)";
    }
    else if (id == 1) {
        directions = "(e2w, w2e)";
    }
    else {
        directions = "(n2w, s2e)";
    }
    if (!debug_mode) {
        printf("Traffic light mini-controller %s: Initialization complete. I am ready.\n", directions);
    }

    /*
        -if id is 0, (n2s, s2n) controller. 
        -this is the default state, hence the traffic lights must be green automatically (without any waiting)
        -next controller is e2w, w2e (remember 2s sleep time)
    */
    if (id == 0) {
        print_lights(1 , directions, debug_mode);
        create_signal_threads(time_green, min_interval, &controller_1_mutex_n2s, &controller_1_mutex_s2n,
        &vehicles_waiting_on_c1_n2s, &vehicles_waiting_on_c1_s2n, &controller_1_waiting_for_vehicles_n2s,
        &controller_1_waiting_for_vehicles_s2n, n2s_counter, s2n_counter, &n2s_mutex_counter, &s2n_mutex_counter);
        print_lights(0 , directions, debug_mode);

        pthread_mutex_lock(&green_mutex);
        pthread_cond_signal(&controller_2);
        pthread_mutex_unlock(&green_mutex);
        while (1) {
            pthread_mutex_lock(&green_mutex);
            pthread_cond_wait(&controller_1, &green_mutex);
            pthread_mutex_unlock(&green_mutex);

            sleep(2);
            print_lights(1 , directions, debug_mode);
            create_signal_threads(time_green, min_interval, &controller_1_mutex_n2s, &controller_1_mutex_s2n,
            &vehicles_waiting_on_c1_n2s, &vehicles_waiting_on_c1_s2n, &controller_1_waiting_for_vehicles_n2s,
            &controller_1_waiting_for_vehicles_s2n, n2s_counter, s2n_counter, &n2s_mutex_counter, &s2n_mutex_counter);
            print_lights(0 , directions, debug_mode);
            
            pthread_mutex_lock(&green_mutex);
            pthread_cond_signal(&controller_2);
            pthread_mutex_unlock(&green_mutex);
        }
    } else if (id == 1) {
        while (1) {
            pthread_mutex_lock(&green_mutex);
            pthread_cond_wait(&controller_2, &green_mutex);
            pthread_mutex_unlock(&green_mutex);
            sleep(2);
            print_lights(1 , directions, debug_mode);
            create_signal_threads(time_green, min_interval, &controller_2_mutex_e2w, &controller_2_mutex_w2e,
            &vehicles_waiting_on_c2_e2w, &vehicles_waiting_on_c2_w2e, &controller_2_waiting_for_vehicles_e2w,
            &controller_2_waiting_for_vehicles_w2e, e2w_counter, w2e_counter, &e2w_mutex_counter, &w2e_mutex_counter);
            print_lights(0 , directions, debug_mode);

            pthread_mutex_lock(&green_mutex);
            pthread_cond_signal(&controller_3);
            pthread_mutex_unlock(&green_mutex);
            
        }
    } else {
        while (1) {
            pthread_mutex_lock(&green_mutex);
            pthread_cond_wait(&controller_3, &green_mutex);
            pthread_mutex_unlock(&green_mutex);

            sleep(2);
            print_lights(1 , directions, debug_mode);
            create_signal_threads(time_green, min_interval, &controller_3_mutex_n2w, &controller_3_mutex_s2e,
            &vehicles_waiting_on_c3_n2w, &vehicles_waiting_on_c3_s2e, &controller_3_waiting_for_vehicles_n2w,
            &controller_3_waiting_for_vehicles_s2e, n2w_counter, s2e_counter, &n2w_mutex_counter, &s2e_mutex_counter);
            print_lights(0 , directions, debug_mode);
            pthread_mutex_lock(&green_mutex);
            pthread_cond_signal(&controller_1);
            pthread_mutex_unlock(&green_mutex);
        }
    }

    pthread_exit(EXIT_SUCCESS);
    

}

void * vehicle_routine(void * arg)
{
    vehicle_t *vehicle = (vehicle_t*)arg;
	int id = vehicle->id;
    char *direction = vehicle->direction;
    //printf("direction %s", direction);

    if (!debug_mode) {
        printf("Vehicle %d %s has arrived at the intersection\n", id, direction);
    } else {
        char* current_time = get_current_time();
        printf("%.2s:vehicle:%d:%s:arrive\n",&current_time[17], id, direction);
    }
	if (!strcmp("n2s", direction)) {
        //printf("n2s dir\n");
        // check if the number of vehicles waiting is 0. If it is, signal the controller and proceed, else, wait for controller
        pthread_mutex_lock(&n2s_mutex_counter);
        *n2s_counter = *n2s_counter+ 1;
        pthread_mutex_unlock(&n2s_mutex_counter);
        pthread_mutex_lock(&controller_1_mutex_n2s);
        pthread_cond_signal(&controller_1_waiting_for_vehicles_n2s);
        //printf("SIGNAL %d\n", signal);
        pthread_cond_wait(&vehicles_waiting_on_c1_n2s, &controller_1_mutex_n2s);
        while (1) {
            //printf("signalled");
            //check that it is in fact next in line, otherwise wait
            if (id == last_vehicle_n2s+1) {
                last_vehicle_n2s++;
                print_vehicle_proceeding(id, direction, debug_mode);
                pthread_mutex_unlock(&controller_1_mutex_n2s);
                pthread_exit(EXIT_SUCCESS);
            } else {
                pthread_cond_wait(&vehicles_waiting_on_c1_n2s, &controller_1_mutex_n2s);
            }
        }     
	} else if (!strcmp("s2n", direction)) {
        //printf("n2s dir\n");
        // check if the number of vehicles waiting is 0. If it is, signal the controller and proceed, else, wait for controller
        pthread_mutex_lock(&s2n_mutex_counter);
        *s2n_counter = *s2e_counter+ 1;
        pthread_mutex_unlock(&s2n_mutex_counter);
        pthread_mutex_lock(&controller_1_mutex_s2n);
        pthread_cond_signal(&controller_1_waiting_for_vehicles_s2n);
        pthread_cond_wait(&vehicles_waiting_on_c1_s2n, &controller_1_mutex_s2n);
        while (1) {
            //check that it is in fact next in line, otherwise wait
            if (id == last_vehicle_s2n+1) {
                last_vehicle_s2n++;
                print_vehicle_proceeding(id, direction, debug_mode);
                pthread_mutex_unlock(&controller_1_mutex_s2n);
                pthread_exit(EXIT_SUCCESS);
            } else {
                pthread_cond_wait(&vehicles_waiting_on_c1_s2n, &controller_1_mutex_s2n);
            }
        } 
	} else if (!strcmp("e2w", direction)) {
        //printf("e2w dir\n");
        // check if the number of vehicles waiting is 0. If it is, signal the controller and proceed, else, wait for controller
        pthread_mutex_lock(&e2w_mutex_counter);
        *e2w_counter = *e2w_counter+ 1;
        pthread_mutex_unlock(&e2w_mutex_counter);
        pthread_mutex_lock(&controller_2_mutex_e2w);
        pthread_cond_signal(&controller_2_waiting_for_vehicles_e2w);
        pthread_cond_wait(&vehicles_waiting_on_c2_e2w, &controller_2_mutex_e2w);
        //printf("recevied\n");
        while (1) {
            //check that it is in fact next in line, otherwise wait
            if (id == last_vehicle_e2w+1) {
                last_vehicle_e2w++;
                print_vehicle_proceeding(id, direction, debug_mode);
                pthread_mutex_unlock(&controller_2_mutex_e2w);
                pthread_exit(EXIT_SUCCESS);
            } else {
                pthread_cond_wait(&vehicles_waiting_on_c2_e2w, &controller_2_mutex_e2w);
            }
        }     
    } else if (!strcmp("w2e", direction)) {
        pthread_mutex_lock(&w2e_mutex_counter);
        *w2e_counter = *w2e_counter+ 1;
        pthread_mutex_unlock(&w2e_mutex_counter);
        pthread_mutex_lock(&controller_2_mutex_w2e);
        pthread_cond_signal(&controller_2_waiting_for_vehicles_w2e);
        pthread_cond_wait(&vehicles_waiting_on_c2_w2e, &controller_2_mutex_w2e);
        //printf("recevied2\n");
        while (1) {
            //check that it is in fact next in line, otherwise wait
            if (id == last_vehicle_w2e+1) {
                last_vehicle_w2e++;
                print_vehicle_proceeding(id, direction, debug_mode);
                pthread_mutex_unlock(&controller_2_mutex_w2e);
                pthread_exit(EXIT_SUCCESS);
            } else {
                pthread_cond_wait(&vehicles_waiting_on_c2_w2e, &controller_2_mutex_w2e);
            }
        } 
    } else if (!strcmp("n2w", direction)) {
        pthread_mutex_lock(&n2w_mutex_counter);
        *n2w_counter = *n2w_counter +1;
        pthread_mutex_unlock(&n2w_mutex_counter);
        pthread_mutex_lock(&controller_3_mutex_n2w);
        pthread_cond_signal(&controller_3_waiting_for_vehicles_n2w);
        pthread_cond_wait(&vehicles_waiting_on_c3_n2w, &controller_3_mutex_n2w);
        while (1) {
            //check that it is in fact next in line, otherwise wait
            if (id == last_vehicle_n2w+1) {
                last_vehicle_n2w++;
                print_vehicle_proceeding(id, direction, debug_mode);
                pthread_mutex_unlock(&controller_3_mutex_n2w);
                pthread_exit(EXIT_SUCCESS);
            } else {
                pthread_cond_wait(&vehicles_waiting_on_c3_n2w, &controller_3_mutex_n2w);
            }
        } 
    } else if (!strcmp("s2e", direction)) {
        pthread_mutex_lock(&s2e_mutex_counter);
        *s2e_counter = *s2e_counter+1;
        pthread_mutex_unlock(&s2e_mutex_counter);
        pthread_mutex_lock(&controller_3_mutex_s2e);
        pthread_cond_signal(&controller_3_waiting_for_vehicles_s2e);
        pthread_cond_wait(&vehicles_waiting_on_c3_s2e, &controller_3_mutex_s2e);
        while (1) {
            //check that it is in fact next in line, otherwise wait
            if (id == last_vehicle_s2e+1) {
                last_vehicle_s2e++;
                print_vehicle_proceeding(id, direction, debug_mode);
                pthread_mutex_unlock(&controller_3_mutex_s2e);
                pthread_exit(EXIT_SUCCESS);
            } else {
                pthread_cond_wait(&vehicles_waiting_on_c3_s2e, &controller_3_mutex_s2e);
            }
        }
    } 
	pthread_exit(EXIT_SUCCESS);
}

