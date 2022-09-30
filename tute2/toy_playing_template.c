#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct child_object
{
    int id;
    int play_time;
} child_obj;

void * child_routine(void *);

//declare global mutex and condition variables

// You need to add something here.
pthread_mutex_t toy_mutex;
pthread_cond_t child0_cond;
pthread_cond_t child1_cond;

int main(int argc, char ** argv)
{
	pthread_t *child_thrd_id; //system thread id
	child_obj *child; //user-defined thread id
	int total_time; //total amount of time childs can play
 	int k, b_t, rc;

	child_thrd_id = malloc(2 * sizeof(pthread_t)); //system thread ids
	if(child_thrd_id == NULL){
		fprintf(stderr, "threads out of memory\n");
		exit(1);
	}	
	
	child = malloc(2 * sizeof(child_obj)); //child objects
	if(child == NULL){
		fprintf(stderr, "t out of memory\n");
		exit(1);
	}	

	//Initialize condition variable objects 

	pthread_mutex_init(&toy_mutex, NULL);
	pthread_cond_init(&child0_cond, NULL);
	pthread_cond_init(&child1_cond, NULL);
	
	// ask for the total time.
	printf("Enter the total amount children can play (int): \n");
	scanf("%d", &total_time);
	
	//ask for children playing time 
	printf("Enter child 0 playing time (int): \n");
	scanf("%d", &b_t);
	child[0].play_time = b_t;
	printf("Enter child 1 playing time (int): \n");
	scanf("%d", &b_t);
	child[1].play_time = b_t;	

	//create child threads 
    for (k = 0; k<2; k++)
    {
		child[k].id = k;
		rc = pthread_create(&child_thrd_id[k], NULL, child_routine, (void *)&child[k]);
		if (rc) {
			printf("ERROR; return code from pthread_create() (child) is %d\n", rc);
			exit(-1);
		}
    }
    
	//sleep total_time seconds
	sleep(total_time);
	
	//Time is up and "the children's parent" calls the children to stop playing, i.e., terminate child threads.
    for (k = 0; k<2; k++) 
    {
		pthread_cancel(child_thrd_id[k]); 
    }

	//deallocate allocated memory
	free(child_thrd_id);
	free(child);

	//destroy mutex and condition variable objects
 
	pthread_mutex_destroy(&toy_mutex);
	pthread_cond_destroy(&child0_cond);
	pthread_cond_destroy(&child1_cond);	
	
    exit(0);
}

void * child_routine(void * arg)
{
	child_obj *child = (child_obj*)arg;
	int id = child->id;
	int play_time = child->play_time;
	
	if (id == 0) {
		printf("child 0: I get to play with the toy for %d units of time.\n", play_time);
		sleep(play_time);
		printf("child 0:  I now give the toy to child 1.\n");
		pthread_mutex_lock(&toy_mutex);
		pthread_cond_signal(&child1_cond);
		pthread_mutex_unlock(&toy_mutex);
		while (1) {
			pthread_mutex_lock(&toy_mutex);
			pthread_cond_wait(&child0_cond,&toy_mutex);
			pthread_mutex_unlock(&toy_mutex);
			printf("child 0: I get to play with the toy for %d units of time.\n", play_time);
			sleep(play_time);
			printf("child 0:  I now give the toy to child 1.\n");
			pthread_mutex_lock(&toy_mutex);
			pthread_cond_signal(&child1_cond);
			pthread_mutex_unlock(&toy_mutex);
		}
	} else {
		while(1) {
			pthread_mutex_lock(&toy_mutex);
			pthread_cond_wait(&child1_cond, &toy_mutex);
			pthread_mutex_unlock(&toy_mutex);
			printf("child 1: I get to play with the toy for %d units of time.\n", play_time);
			sleep(play_time);
			printf("child 1:  I now give the toy to child 0.\n");
			pthread_mutex_lock(&toy_mutex);
			pthread_cond_signal(&child0_cond);
			pthread_mutex_unlock(&toy_mutex);
		}
	}

	void* returnval = NULL;
	pthread_exit(returnval);

}
