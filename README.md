# IntroPARCO-2024-H1
Homework for the first deliverable of the course "Introduction to parallel computing" in University of Trento.

In this repository we will analyze the behavior of a code execution with different parallelization techniques, in particular we will consider these performance metrics: speedup and efficiency. The techniques that we will discuss are: serial execution, implicit parallelism execution (with optimization flags and pragmas) and explicit parallelism execution (using OpenMP directives).
The task that we want to implement is a code for a matrix transposition with different matrix sizes and different parallelization methods to compare the wall-clock time of each approach and find the best solution to optimize the performance and the resource usage.  

# Index
1. [System Description](#system-description)
2. [Set up the Project](#set-up-the-project)
3. [Record the Wall-Clock Time](#record-the-wall-clock-time)
4. [Implicit Parallelism Implementation](#implicit-parallelism-implementation)
5. [Explicit Parallelism Implementation](#explicit-parallelism-implementation)
6. [Compilation and Execution](#compilation-and-execution)
7. [Performance Analisys](#performance-analisys)
8. [Results and Observations](#results-and-observations)

## System Description
We are working on the university cluster that has these characteristics:
- composed of 142 CPU calculation nodes running at 2.3GHz for a total of 7674 cores and 10 GPU calculation nodes for a total of 48.128 CUDA cores
- operating system: Linux CentOS 7
- cluster management software: PBS Professional
- compiler: GCC 9.1.0.
- nodes are interconnected with 10Gb/s network and some have Infiniband connectivity, others have Omni-Path connectivity

## Set up the Project
The first thing to do is accessing the cluster (if necessary) with the ssh command. Then we have to start an interactive session with the following line:
```
qsub -I -q name_queue 
```
we can also include some specifications on which characteristics the queue we are entering must have:
```
qsub -I -q name_queue -l select=1:ncpus=60:ompthreads=60:mem=1mb
//number of nodes, number of CPU/core, number of threads and memory per node 
```
After we start an interactive session we must enter the folder in which we want to work and create the file for our project, could be done with this command: `touch homework.c`
And then we can start writing our sequential code. It must contain.
- control on the [size-of-the-matrix](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/2aa730e45069cc39935a170f5c8d2ae640e9c7da/code.c#L22C1-L28C2)
- control on the [matrix-allocation](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/31003fb43f223152bd73e72a461ad5a9d2e3acca/code.c#L204)
- control on the values of the matrix to check if it is [symmetric](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/31003fb43f223152bd73e72a461ad5a9d2e3acca/code.c#L54C1-L74C2) (and so the transposition is not needed)
- function to [transpose-the-matrix](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/31003fb43f223152bd73e72a461ad5a9d2e3acca/code.c#L124)

***Nota*** The symmetric check function would be faster if we add a break command after the first non-symmetric value is found, but for our purpose we will continue without this command that avoid the parallelization (for more explanations view the report)
After we implemented these functions we will have the base code with which we can then compare the parallel one.

## Record The Wall-Clock Time
To obtain the time taken for the execution we have to use the `gettimeofday()` function of the `sys/time.h` library:
```
struct timeval start_tv, end_tv;
struct timespec start_ts, end_ts;
gettimeofday(&start_tv, NULL);
  //code to measure
gettimeofday(&end_tv, NULL); 
  long seconds_tv = end_tv.tv_sec - start_tv.tv_sec;
  long microseconds_tv = end_tv.tv_usec - start_tv.tv_usec;
  double elapsed_gettimeofday = seconds_tv + microseconds_tv*1e-6;
  printf("    wall-clock time for the symmetric check: %.8f milliseconds\n", elapsed_gettimeofday*1000);
```

## Implicit Parallelism Implementation
The first optimization that we can try to execute is the implicit parallelism one: it consists in adding some optimization flags in the compilation and the most suitable pragmas above the code that we want to optimize. For example, in our case, we have two nested loops to create the [transposed-matrix](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/31003fb43f223152bd73e72a461ad5a9d2e3acca/code.c#L141), the `#pragma simd` directive tells the compiler to force the vectorization and the `#pragma unroll(n)` will unroll the loops on a certain degree (n). These are the most suitable one for our code (the `#pragma ivdep` is not needed because we don't have dependencies). For what concern the optimization flags, after a few trials with different ones we can conclude that the most performant combination of flags is `-O2 -funroll-loops`. If we want to take the code all together in one file and make the flags affect only the implicit parallelism part we can add above the function `#pragma GCC optimize ("O2", "unroll-loops")`.

***Nota*** The same optimizations could be applied to the [symmetric-check](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/31003fb43f223152bd73e72a461ad5a9d2e3acca/code.c#L77) nested loops

## Explicit Parallelism Implementation
The first step is to include the `<omp.h>` library. The openMP method for the [matrix-transposition](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/31003fb43f223152bd73e72a461ad5a9d2e3acca/code.c#L160) will execute with a certain number of threads the parallel regions and use even more optimizations that we will add, for example: in our case, with the nested loops, we can add a `#pragma omp for collapse(2)` that will compress the two loops in a single amount of iterations to divide among the threads. Another clause that we can add is the `schedule(auto)` that will tell the compiler that at runtime it must choose the best scheduling strategy based on the system characteristics.

***Nota*** For the OpenMP timings we have another tool to record the wall-clock time. We store the start and end times in two variables with the `omp_get_wtime()` function included in the `omp.h` library
***Nota*** Also in this case, the same optimizations could be applied for the [symmetric-check](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/31003fb43f223152bd73e72a461ad5a9d2e3acca/code.c#L102)

## Compilation and Execution
Once we have our code with the three different approaches we can compile and execute it to record the timings and observe the improvements. 
To compile the code:
```
gcc -o result homework.c -fopenmp
```
***Nota*** it is necessary to include in the compilation the `-fopenmp` flag to tell the compiler to use the OpenMP functionality (otherwise the explicit paralellism code will not be executed in paralell with threads)

To run our compiled code we have two possibilities: 
- the first is to use the interactive session that we have started and execute many times with different number of threads and matrix sizes. This is done with the `export OMP_NUM_THREADS=2; ./result` command
- the second is to create a [.pbs-script](script.pbs) in which we tells the compiler the number of executions that we want and with which parameters (number of threads and size of the matrix). We can do that by creating a new file with the .pbs extension and write the following code:
```
#!/bin/bash
# name the output file 
#PBS -N 16_results
# choose the resources to execute the job
#PBS -l select=1:ncpus=60:ompthreads=60:mem=1mb
# specify the maximum execution time
#PBS -l walltime=0:10:00
# tells the compiler to take together the error and output files
#PBS -j oe
# choose the queue
#PBS -q short_cpuQ

# Modules for C
module load gcc91
gcc() {
    gcc-9.1.0 "$@"
}
gcc --version

# Select the working directory
cd /home/sophie.motter/exercises/homework1

# Run code, this will give us the output of 10 execution for each thread (2-4-8-16-20) with the matrix size of 16x16
for threads in 2 4 8 16 20; do
  echo "==========================================" >> o_16.log
  echo "---------------thread $threads ----------" >> o_16.log
  for i in {1..10}; do
    export OMP_NUM_THREADS=$threads; echo 16 | ./result >> o_16.log
    echo "-------------------------------------------" >> o_16.log
  done
  echo "==========================================" >> o_16.log
done
```
This script can be executed for each matrix size we want by changing this parameter in the echo command inside the inner for loop. To submit this job we must write the following command:
`qsub ./script.pbs` and then in our folder we will find the file in which there are written the wall-clock time requested for each number of thread.

## Performance Analisys
After we recorded a sufficient number of executions we can make the averages for each method and compare the results by calculating the speedup, that is how much the code is faster with respect to the serial one, and efficiency, that measure how efficiently the resources (threads) are utilized.
To calculate the speedup:
```Speedup = SerialTime / ParallelTime```
To calculate the efficiency:
```Efficiency = Speedup / N_Threads```

## Results and Observations
By studying the recorded values and the resulting speedup and efficiency calculations we can observe that:
- for small matrix sizes the performance are better with an implicit parallelism approach, this happens because the time taken for the creation and synchronization of the threads in an explicit parallelism execution will increase the wall-clock time and lead to a decrease of performance (especially with an high number of threads).
- for big matrix sizes the most appropriate method is the explicit parallelism one that will speedup the execution proportionally to the number of threads used (bigger=faster)

***Nota*** we have to take in count that for each matrix size there is a peak of performance above which the increase of execution units will decline the speedup and lead to inefficiencies. This happens for the same reason that we have notice in the small matrix sizes (time taken to create and synchronize threads will affect wall-clock time). 
