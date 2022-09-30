#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct passenger_object
{
    int id;
    char direction[4]; //either n2s or n2s
} passenger_obj;

void * boat_routine(void *); 
void * passenger_routine(void *);

//declare global mutex and condition variables
pthread_mutex_t boat_mutex;
pthread_cond_t n2s_cond;
pthread_cond_t s2n_cond;


// you need to add something here.

int main(int argc, char ** argv)
{
	int no_of_passengers; //total number of passengers to be created
	int boat_pace, passenger_rate;
	pthread_t *c_thrd_ids, b_thrd_id; //system thread id
	passenger_obj *passenger; //user-defined thread id
 	int drct, n_c, s_c;
	int k, rc;
	
	// ask for the total number of passengers.
	printf("Enter the total number of passengers (int): \n");
	scanf("%d", &no_of_passengers);
	
	//ask for boat's working pace
	printf("Enter boat's working pace (int): \n");
	scanf("%d", &boat_pace);
	
	//ask for passengers' arrival rate
	printf("Enter passengers arrival rate (int): \n");
	scanf("%d", &passenger_rate);
		
	c_thrd_ids = malloc((no_of_passengers) * sizeof(pthread_t)); //passenger thread ids
	if(c_thrd_ids == NULL){
		fprintf(stderr, "threads out of memory\n");
		exit(1);
	}	
	
	passenger = malloc((no_of_passengers) * sizeof(passenger_obj)); //total is no_Of_passengers 
	if(passenger == NULL){
		fprintf(stderr, "t out of memory\n");
		exit(1);
	}	

	//Initialize condition variable objects 
	// you need to add something here.
	pthread_mutex_init(&boat_mutex, NULL);
	pthread_cond_init(&n2s_cond, NULL);
	pthread_cond_init(&s2n_cond, NULL);

	
	//create the boat thread.
	if (pthread_create(&b_thrd_id, NULL, boat_routine, (void *) &boat_pace)){
		printf("ERROR; return code from pthread_create() (boat) is %d\n", rc);
		exit(-1);
	}

	//create consumers according to the arrival rate
	n_c = s_c = 0;
	srand(time(0));
    for (k = 0; k<no_of_passengers; k++)
    {
		sleep((int)rand() % passenger_rate); 
		drct = (int)rand()% 2;
		if (drct == 0){
			strcpy(passenger[k].direction, "n2s");
			passenger[k].id = n_c;
			n_c++;
		}
		else if (drct == 1){
			strcpy(passenger[k].direction, "s2n");
			passenger[k].id = s_c;
			s_c++;
		}
		rc = pthread_create(&c_thrd_ids[k], NULL, passenger_routine, (void *)&passenger[k]);
		if (rc) {
			printf("ERROR; return code from pthread_create() (consumer) is %d\n", rc);
			exit(-1);
		}
    }
    
	//join passenger threads
    for (k = 1; k<no_of_passengers; k++) 
    {
		pthread_join(c_thrd_ids[k], NULL);
    }
	
	//After all passenger threads exited, terminate the cashier thread
    pthread_cancel(b_thrd_id); 
			
	//deallocate allocated memory
	free(c_thrd_ids);
	free(passenger);

	//destroy mutex and condition variable objects
	pthread_mutex_destroy(&boat_mutex);
	pthread_cond_destroy(&n2s_cond);
	pthread_cond_destroy(&s2n_cond);	

    exit(0);
}

void * boat_routine(void * arg)
{
	int boat_pace = *((int*)arg);
	printf("Boat starts working in pace %d \n\n", boat_pace);
	char current_state[4] = "s2n";
	char *current_state_ptr = current_state;

	char n2s_state[4] = "n2s";
	char *n2s_state_ptr = n2s_state;

	char s2n_state[4] = "s2n";
	char *s2n_state_ptr = s2n_state;



	while(1) {
		printf("Boat move %s\n", current_state_ptr);
		pthread_mutex_lock(&boat_mutex);

		if (strcmp(current_state_ptr, n2s_state_ptr) == 0) {
			//printf("current state: (should be n2s) %s\n", current_state_ptr);
			printf("all aboard s2n\n");
			pthread_cond_signal(&n2s_cond);
			memcpy((void*)current_state_ptr, (void*)s2n_state_ptr, 4);
		} else {
			//printf("current state: should be (s2n) %s\n", current_state_ptr);
			printf("all aboard n2s\n");
			pthread_cond_signal(&s2n_cond);
			memcpy((void*)current_state_ptr, (void*)n2s_state_ptr, 4);
		}
		pthread_mutex_unlock(&boat_mutex);

		sleep(boat_pace);
	}

	// you need to write a routine for boat.
	void* returnval = NULL;
	return returnval;
	
}

void * passenger_routine(void * arg)
{
	// you need to write a routine for passenger.
	// each passenger has an id, direction, and createes a passenger routine object
	passenger_obj *passenger = (passenger_obj*)arg;
	int passenger_id = passenger->id;
	char *direction = passenger->direction;

	printf("Passenger %s %d arrives and waits to cross the river\n", direction, passenger_id);

	pthread_mutex_lock(&boat_mutex);
	char *n2s_string = "n2s";
	char *s2n_string = "s2n";
	if (strcmp(n2s_string, direction)) {
		pthread_cond_wait(&n2s_cond, &boat_mutex);
		printf("Passenger %s %d is now on boat\n\n", direction, passenger_id);
	} else {
		pthread_cond_wait(&s2n_cond, &boat_mutex);
		printf("Passenger %s %d is now on boat\n\n", direction, passenger_id);
	}

	pthread_mutex_unlock(&boat_mutex);
	

	void* returnval = NULL;
	pthread_exit(returnval);
}
