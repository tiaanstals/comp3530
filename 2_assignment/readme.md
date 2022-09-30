# How to use VSED (Very Simple Experimental Dispatcher)

This readme will explain how to best utilise the VSED. 

## Compilation
There is a Makefile provided which will compile all the necessary files into their required executables 
```
cd your_vsed_directory && make
//run the VSED
make clean //this will clean this executables
```
## Generating Random Process Files
The random_aps.c program will compile into the **random** executable. In order to generate random processes for consumption by the vsed program you can run:
```
./random some_file_name
Please enter the number of jobs that are to be dispatched: 20
Please enter the mean of the random Poisson distribution for intervals between job arrivals: 8
Please enter the inverse of the mean of the random exponential distribution for job execution duration: 0.2
```
This will then generate a file with the name of some_file_name which will be needed to run the VSED. 

The 3 values enter are as follows:
 - Number of processes
 - The mean of the intervals between jobs (uses a possion distribution). Suggested values for this range between 1-10. The larger the number, the longer between process arrival times
 - The inverse of the mean of job duration (using an exponential poisson distribution). The smaller the number, the larger the job execution times. Suggested values range between 0.1-1. 

## Running the Program
In order to run the program, you will need to provide the process file to the vsed as an argument
```
./vsed some_file_name
Please enter quanta for priority level 0 (highest): 2
Please enter quanta for priority level 1 (medium): 4
Please enter quanta for priority level 2 (lowest): 8
```
The program will then ask you for inputs regarding the quantum length for programs of different priorities. 
The suggested length of quantum for priority level RQ<sub>i</sub> is 2<sup>i</sup> units of time (which in this case is seconds)
