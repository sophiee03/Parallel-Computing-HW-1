# H1 Serial and Parallel Execution Comparison
Homework for the first deliverable of the course "Introduction to parallel computing" in University of Trento.

In this repository we will analyze the behavior of a code execution with different parallelization techniques: serial execution, implicit parallelism execution and explicit parallelism execution.
The task that we want to implement is a code for a matrix transposition with different matrix sizes to compare the wall-clock time taken for each approach and find the best solution to optimize the performance and the resource usage.  

# Index
1. [Set up the Project](#set-up-the-project)
2. [Parallel Implementation](#parallel-implementation)
3. [Compilation and Execution](#compilation-and-execution)
4. [Performance Analisys](#performance-analisys)

## Set up the Project
The first thing to do is accessing the cluster (if necessary) with the `ssh` command. Then we have to start an interactive session with the following line:
```
qsub -I -q name_queue 
```
we can also include some specifications on which characteristics the queue we are entering must have:
```
qsub -I -q name_queue -l select=1:ncpus=60:ompthreads=60:mem=1mb
//number of nodes, number of CPU/core, number of threads and memory per node 
```
After we start an interactive session we must enter the folder in which we want to work and create/import the file for our project, then we can start writing our sequential code that must contain:
- control on the [size of the matrix](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/f0f57507d67a5b9177c49b7338466a276ca22a54/code.c#L22)
- control on the [matrix allocation](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/f0f57507d67a5b9177c49b7338466a276ca22a54/code.c#L198)
- control of [symmetry](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/04e2b838caaa45dd578e82cc1c8653a569859f2b/code.c#L56) (if true the transposition is not needed)
- function to [transpose the matrix](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/f0f57507d67a5b9177c49b7338466a276ca22a54/code.c#L124)
- function to control if the implicit/explicit transposition is [executed correctly](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/04e2b838caaa45dd578e82cc1c8653a569859f2b/code.c#L39)

After we implemented these functions we will have the base code with which we can then compare the parallel one.

## Parallel Implementation
RECORD THE TIMINGS:

To obtain the time taken for the execution we have to use the `gettimeofday()` function of the `sys/time.h` library. 

For the OpenMP timings we have another tool to record the time taken: we store the start and end times in two variables with the `omp_get_wtime()` function included in the `omp.h` library and then compute the difference to find the wall-clock time.

IMPLICIT PARALLELISM:

It consists in adding some optimization flags in the compilation and the most suitable pragmas above the code that we want to optimize. In our code, for the [transposed-matrix](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/f0f57507d67a5b9177c49b7338466a276ca22a54/code.c#L141) and the [symmetric-check](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/f0f57507d67a5b9177c49b7338466a276ca22a54/code.c#L77), these pragmas are the most suitable: 
- the `#pragma simd` directive tells the compiler to force the vectorization
- the `#pragma unroll(n)` directive will unroll the loops on a certain degree (n).

For what concern the optimization flags, after a few trials with different ones we can conclude that the most performant combination of flags for our code is `-O2 -funroll-loops`.

EXPLICIT PARALLELISM:

The openMP method for the [matrix-transposition](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/f0f57507d67a5b9177c49b7338466a276ca22a54/code.c#L160) and for the [symmetric-check](https://github.com/sophiee03/IntroPARCO-2024-H1/blob/f0f57507d67a5b9177c49b7338466a276ca22a54/code.c#L102) will execute with a certain number of threads the parallel regions and use even more optimizations added by clauses:
- `for collapse(2)` directive to compress loops in a single amount of iterations divided among the threads. 
- `schedule(auto)` clause tell the compiler that at runtime it must choose the best scheduling strategy based on the system characteristics.

## Compilation and Execution
Once we have our code with the three different approaches we can compile and execute it to record the timings and observe the improvements. 
To compile the code (we have to be in the folder in which the code is saved):
```
gcc -o result homework.c -fopenmp
```
***N.B.*** it is necessary to include in the compilation the `-fopenmp` flag to tell the compiler to use the OpenMP functionality (otherwise the explicit paralellism code will not be executed in paralell with threads)

To run our compiled code we have two possibilities: 
- the first method is to use the interactive session that we have started and execute many times with different number of threads and matrix sizes. This is done with the `export OMP_NUM_THREADS=2; ./result 64` command, but it is time expensive, especially if we need a big number of executions.
- the second and faster method is to create a [.pbs script](script.pbs) in which we tell the compiler the number N of executions that we want. This is done by using Job Arrays: the code in the PBS script will be executed N times in the same environment with the parameters that we have included in the code (matrix size and number of threads). If we want to change parameters and test with different ones we need only to change these values. To submit this job we must write the following command: `qsub ./script.pbs` and then in our folder we will find the N output files generated. To submit more times the job it is necessary to compile only one time the code (and have only one executable file), to do that it is possible to compile the code one time and remove from the pbs script the compilation line before submitting it.

***N.B.*** if you use my pbs script remember to change the folder path in which the codes are saved and the PBS directives based on your system

***N.B.*** [for windows users] if you encounter errors in the submission of the psb script, it could be a problem caused by the operating system (because you are writing in a 'windows format'). To avoid this problem use this command before submitting the job: `dos2unix script.pbs`

## Performance Analisys
After we recorded a sufficient number of executions we can make the averages for each method and compare the results by calculating the speedup, that is how much the code is faster with respect to the serial one, and efficiency, that measure how efficiently the resources (threads) are utilized.

To calculate the speedup:
```Speedup = SerialTime / ParallelTime```

To calculate the efficiency:
```Efficiency = Speedup / N_Threads```

We can observe the trend of these metrics in the following graphs:
<div style="display: flex; justify-content: space-around;">
  <img src="images/speedupmat.png" alt="Image 1" width="500" />
  <img src="images/speedupsym.png" alt="Image 2" width="500" />
</div>
<div style="display: flex; justify-content: space-around;">
  <img src="images/efficiencymat.png" alt="Image 3" width="500" />
  <img src="images/efficiencysym.png" alt="Image 4" width="500" />
</div>
