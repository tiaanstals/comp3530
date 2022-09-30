

quantums = [ userinput1, userinput2, userinput3]
priority_queues = [queue1, queue2, queue3]

processes_from_file = read_file(user_provided_file_pointer)

for process in processes_from_file:
    intput_queue = intput_queue + process

while (processes_in_inputqueue or get_highest_priority_process_in_priority_queues() != null or current_process != null):

    #add all processes to their respective queues if they have arrived
    while (intput_queue != null || intput_queue[-1].arrival_time < now):
        intput_queue = intput_queue - process
        priority_queues[process.priority] = priority_queues[process.priority] + process

    #if this is true, the current_process has just been interrupted
    if (current_process):
        current_process.cpu_time = current_process.cpu_time - quantums[current_process.priority]

        if (current_process.cpu_time <= 0 ):
            write_process_statistics_tofile(process)
            terminate_process(current_process)
            current_process = null
        #this means there are some processes which remain
        else if (get_highest_priority_process_in_priority_queues() != null)
            pause_process(current_process)
            current_process.priority = current_process.priority -1
            put_onto_new_priorityqueue(current_process)
            current_process = null

    #if there is no current_process, the next process must be selected and enqueued
    else if current_process == null:
        current_process = get_highest_priority_process_in_priority_queues()
        start_process(current_process)

    ```
        -at this point, either a process has been interrupted and a new one selected, 
        -or the processor was sleeping and a new process had been selected
    ```
    if current_process != null
        if current_process.remaining_cpu_time < quantums[current_process.priority]
            sleep(current_process.remaining_cpu_time)
        else:
            sleep(quantums[current_process.priority])
    #no process is in the ready queue
    else:
        sleep(1)
    
    timer = timer + sleeptime
    

