/*
    COMP3520 Assignment 2 - Very Simple Experimental Dispatcher (VSED)

    usage:

        ./vsed <TESTFILE>
        where <TESTFILE> is the name of a job list
*/

/************************************************************************************************************************

    ** Revision history **

    Current version: 1.3 bis
    Date: 18 September 2019

    1.3: Minor reformatting 
    1.2: Modified instructions and variable names for teaching purposes
    1.1: Added instructions to allow additional functions
    1.0: Original version

    Contributors:
    1. COMP3520 teaching staff
       Centre for Distributed and High Performance Computing
       School of Computer Science
       The University of Sydney
       NSW 2006
       Australia

    2. Dr Ian G Graham

    Copyright of this code and associated material is vested in the original contributors.

    This code is NOT in the Public Domain. Unauthorized posting of this code or derivatives thereof is not permitted.

    ** DO NOT REMOVE THIS NOTICE. **

 ***********************************************************************************************************************/

/* Include files */
#include "vsed_0.h"

/******************************************************
 
   internal functions
   
 ******************************************************/

int CheckQueues(PcbPtr *);

/******************************************************/

int main (int argc, char *argv[])
{
    FILE * inputliststream;
    PcbPtr inputqueue = NULL;     // input queue buffer
    PcbPtr fbqueue[N_FB_QUEUES];  // feedback queues
    PcbPtr currentprocess = NULL; // current process
    PcbPtr process = NULL;        // working pcb pointer
    int timer = 0;                // dispatcher timer
    int quantum = QUANTUM;        // current time-slice quantum
    int i;                        // working index
    int * quantums_lookup;
    FILE * output_stream = NULL;
    FILE * read_statistics;
    //reads all statistics for averages
    int total_wait_time = 0;
    int total_turnaround_time = 0;
    int full_total_quantums = 0;
    int number_processes = 0;

//  0. Parse command line

     if (argc <= 0)
    {
        fprintf(stderr, "FATAL: Bad arguments array\n");
        exit(EXIT_FAILURE);
    }
    else if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <TESTFILE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    quantums_lookup = malloc(sizeof(int) * 3);

    printf("Please enter quanta for priority level 0 (highest): ");
    scanf("%d", &(quantums_lookup[0]));

    printf("Please enter quanta for priority level 1 (medium): ");
    scanf("%d", &(quantums_lookup[1]));

    printf("Please enter quanta for priority level 2 (lowest): ");
    scanf("%d", &(quantums_lookup[2]));
	
//  1. Initialize dispatcher queues;

    for (i = 0; i < N_FB_QUEUES; fbqueue[i++] = NULL);
    
//  initialise the output stream
if ((output_stream = fopen("output.csv", "w")) == NULL)
    {
        fprintf(stderr, "FATAL: Unable to open file for writing.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(output_stream, "PID, ARRIVAL_TIME, TERMINATION_TIME, WAITING_TIME, TURNAROUND_TIME, INITIAL_PRIORITY, FINAL_PRIORITY, INITIAL_CPU_TIME, INTERRUPTIONS, TOTAL_QUANTUMS\n");
    
if (!(inputliststream = fopen(argv[1], "r")))
    {
        fprintf(stderr, "ERROR: Could not open \"%s\"\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    //adds all processes from input queue into the input_queue buffer
    while (!feof(inputliststream)) {  // put processes into input_queue
        process = createnullPcb();
        if (fscanf(inputliststream,"%d, %d, %d",
             &(process->arrivaltime), &(process->priority),
             &(process->remainingcputime)) != 3) {
            free(process);
            continue;
        }
        process->initial_priority = process->priority;
        process->initial_cpu_time = process->remainingcputime;
        process->status = PCB_INITIALIZED;
        number_processes++;
        inputqueue = enqPcb(inputqueue, process);
    }

//  3. Start dispatcher timer;
//     (already set to zero above)
        
//  4. While there's anything in any of the queues or there is a currently running process:

    while (inputqueue || (CheckQueues(fbqueue) >= 0) || currentprocess ) {

//      i. Unload any pending processes from the input queue:
//         While (head-of-input-queue.arrival-time <= dispatcher timer)
//         dequeue process from input queue and and enqueue on highest
//         priority feedback queue (assigning it the appropriate priority);

        //adds all processes from buffer into their appropriate queue
        while (inputqueue && inputqueue->arrivaltime <= timer) {
            process = deqPcb(&inputqueue);          // dequeue process
            process->status = PCB_READY;            // set pcb ready
            //process->priority = 0;                  // override any priority 
            fbqueue[process->priority] = enqPcb(fbqueue[process->priority], process);
                                                    // & put on queue
        }

//     ii. If a process is currently running;
        
        //case in which a processes was busy executing when the program was still running
        if (currentprocess) {
            // printf("There is a current process running, reducing its CPU time by %d\n", quantum);

//          a. Decrement process remainingcputime;

            currentprocess->remainingcputime -= quantum;
            
//          b. If times up:

            if (currentprocess->remainingcputime <= 0) {
                
//             A. Send SIGINT to the process to terminate it;
                currentprocess->turnaround_time = timer - (currentprocess->arrivaltime);
                currentprocess->waiting_time = (currentprocess->turnaround_time) - (currentprocess->initial_cpu_time);
                currentprocess->termination_time = timer;
                full_total_quantums = full_total_quantums + (currentprocess->total_quantums);
                total_turnaround_time = total_turnaround_time + (currentprocess->turnaround_time);
                total_wait_time = total_wait_time + (currentprocess->waiting_time);
                //printf("total quantums %d\n", full_total_quantums);
                // printPcbHdr();
                // printPcb(currentprocess);


                fprintf(output_stream, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", currentprocess->pid, currentprocess->arrivaltime, 
                currentprocess->termination_time, currentprocess->waiting_time, currentprocess->turnaround_time, currentprocess->initial_priority, 
                currentprocess->priority, currentprocess->initial_cpu_time, currentprocess->suspensions, currentprocess->total_quantums);
                terminatePcb(currentprocess);
                
//             B. Free up process structure memory

                free(currentprocess);
                currentprocess = NULL;
                
//         c. else if other processes are waiting in feedback queues:
//           process has run for its allocated quanta, parameters now need to be re-evaluated
            } else if (CheckQueues(fbqueue) >= 0) {
                
//             A. Send SIGTSTP to suspend it;
                //printf("Sending pcd suspend\n");
                ++(currentprocess->suspensions);

                suspendPcb(currentprocess);

//             B. Reduce the priority of the process (if possible) and enqueue it on
//                the appropriate feedback queue;;

                if (++(currentprocess->priority) >= N_FB_QUEUES)
                    currentprocess->priority = N_FB_QUEUES - 1;
                fbqueue[currentprocess->priority] = 
                    enqPcb(fbqueue[currentprocess->priority], currentprocess);
                currentprocess = NULL;
            }
        }
        
//    iii. If no process currently running && feedback queues are not empty:

        if (!currentprocess && (i = CheckQueues(fbqueue)) >= 0) {

//         a. Dequeue process from RR queue

            currentprocess = deqPcb(&fbqueue[i]);
            ++(currentprocess->total_quantums);
            
//         b. If already started but suspended, restart it (send SIGCONT to it)
//              else start it (fork & exec)
//         c. Set it as currently running process;
            
            startPcb(currentprocess);
        }
        
//      iv. sleep for quantum;
        //if there is a current process running, sleep for its allocated quantum
        //if the remaining CPU time is less than the full quantum, only sleep for remaining cputime
        //else sleep for the full cpu time
        if (currentprocess) {
            if (currentprocess->remainingcputime < quantums_lookup[process->priority]) {
                quantum = currentprocess->remainingcputime;
            } else {
                quantum = quantums_lookup[process->priority];
            }
            ++(currentprocess->total_quantums);
        }
        //otherwise, sleep for 1 second
        else {
            quantum = 1;
        }
        //printf("Quantum time = %d\n", quantum);
        sleep(quantum);
            
//       v. Increment dispatcher timer;

        timer += quantum;
            
//      vi. Go back to 4.

    }
        
//    5. Exit
    fclose(output_stream);
    if (!(read_statistics = fopen("output.csv", "r")))
    {
        fprintf(stderr, "ERROR: Could not open statistics file");
        exit(EXIT_FAILURE);
    }

    
    float average_wait = (float)total_wait_time / number_processes;
    float average_turnaround = (float)total_turnaround_time / number_processes;
    float average_quantums = (float)full_total_quantums / number_processes;

    printf("========================================\n");
    printf("***************AVERAGES*****************\n\n\n");
    printf("TOTAL PROCESSES %d\n", number_processes);
    printf("TOTAL WAIT %d\n", total_wait_time);
    printf("TOTAL TURNAROUND %d\n", total_turnaround_time);
    printf("TOTAL QUANTUM %d\n", full_total_quantums);
    printf("AVERAGE WAIT TIME %f\n\n", average_wait);
    printf("AVERAGE TURN AROUND TIME %f\n\n", average_turnaround);
    printf("AVERAGE TOTAL QUANTUMS %f\n\n", average_quantums);
    printf("========================================\n");
    exit (0);
}    


/*******************************************************************

int CheckQueues(PcbPtr * queues)

  check array of dispatcher queues

  return priority of highest non-empty queue
          -1 if all queues are empty
*******************************************************************/
int CheckQueues(PcbPtr * queues)
{
    int n;

    for (n = 0; n < N_FB_QUEUES; n++)
        if (queues[n]) return n;
    return -1;
}
