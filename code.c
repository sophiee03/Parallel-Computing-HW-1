```ruby
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#ifdef _OPENMP
  #include <omp.h>
#endif

#define MIN 0.0
#define MAX 10.0

int n=0;
struct timeval start_tv, end_tv;
struct timespec start_ts, end_ts;
long seconds_tv=0;
long microseconds_tv=0;
double elapsed_gettimeofday =0;
#ifdef _OPENMP
  double wt1,wt2;
#endif

int checkPower(int num){
  if (num<=0) return 0;
  while(num%2==0){
	  num/=2;
  }
  return (num==1);
}

void print(float** mat){
for (int i=0; i<n; i++){
  for (int j=0; j<n; j++){
    printf("%0.4f	", mat[i][j]);
  }
  printf("\n");
}
}

int check_correctness(float** T1, float** T2){
  //assume that it is correct
  int check=0; 
  //check if there are different values
  for(int i=0; i<n; i++){
    for (int j=0; j<n; j++){
      if (T1[i][j]!=T2[i][j]){
        check=1;
      }
    }
  }
  //return 0 if it is correct, 1 otherwise
  return check;
}

int checkSym(float** m){
  //assume the matrix is symmetric
  int check=1;
  //start timer
  gettimeofday(&start_tv, NULL);
  //check if there is a non-symmetric value 
  for(int i=0; i<n; i++){
    for (int j=0; j<n; j++){
      if (m[i][j]!=m[j][i]){
        check=0;
      }
    }
  }
  //stop timer
  gettimeofday(&end_tv, NULL); 
  seconds_tv = end_tv.tv_sec - start_tv.tv_sec;
  microseconds_tv = end_tv.tv_usec - start_tv.tv_usec;
  elapsed_gettimeofday = seconds_tv + microseconds_tv*1e-6;
  printf("    wall-clock time for the symmetric check: %.8f milliseconds\n", elapsed_gettimeofday*1000);
  return check;
}

//add optimization flags
#pragma GCC optimize ("O2", "unroll-loops")
int checkSymIMP(float** m){
  //assume the matrix is symmetric
  int check=1;
  //start timer
  gettimeofday(&start_tv, NULL);
  #pragma simd
  #pragma unroll(4)
  //check if there is a non-symmetric value
  for(int i=0; i<n; i++){
    for (int j=0; j<n; j++){
      if (m[i][j]!=m[j][i]){
        check=0;
      }
    }
  }
  //stop timer
  gettimeofday(&end_tv, NULL); 
  seconds_tv = end_tv.tv_sec - start_tv.tv_sec;
  microseconds_tv = end_tv.tv_usec - start_tv.tv_usec;
  elapsed_gettimeofday = seconds_tv + microseconds_tv*1e-6;
  printf("    wall-clock time for the symmetric check: %.8f milliseconds\n", elapsed_gettimeofday*1000);
  return check;
}

int checkSymOMP(float** m){
  int check=1;
  //start timer
  #ifdef _OPENMP
    wt1=omp_get_wtime();
  #endif 
  #pragma omp parallel for collapse(2)
  for(int i=0; i<n; i++){
    for (int j=0; j<n; j++){
      if (m[i][j]!=m[j][i]){
        check=0;
      }
    }
  }
  //stop timer
  #ifdef _OPENMP
    wt2=omp_get_wtime();
    printf("    wall-clock time for the symmetric check = %.8f milliseconds\n", (wt2-wt1)*1000 );
  #endif
  return check;
}

void matTranspose(float** m, float** t){
  //start timer
  gettimeofday(&start_tv, NULL);
  for (int i=0; i<n; i++){
    for (int j=0; j<n; j++){
      t[i][j]=m[j][i];
    }
  }
  //stop timer
  gettimeofday(&end_tv, NULL); 
  seconds_tv = end_tv.tv_sec - start_tv.tv_sec;
  microseconds_tv = end_tv.tv_usec - start_tv.tv_usec;
  elapsed_gettimeofday = seconds_tv + microseconds_tv*1e-6;
  printf("    wall-clock time for the matrix transposition: %.8f milliseconds\n", elapsed_gettimeofday*1000);
}

//add optimization flags
#pragma GCC optimize ("O2", "unroll-loops")
void matTransposeIMP(float** m, float** t){
  //start timer
  gettimeofday(&start_tv, NULL);
  #pragma simd
  #pragma unroll(4)
  for (int i=0; i<n; i++){
    for (int j=0; j<n; j++){
      t[i][j]=m[j][i];
    }
  }
  //stop timer
  gettimeofday(&end_tv, NULL); 
  seconds_tv = end_tv.tv_sec - start_tv.tv_sec;
  microseconds_tv = end_tv.tv_usec - start_tv.tv_usec;
  elapsed_gettimeofday = seconds_tv + microseconds_tv*1e-6;
  printf("    wall-clock time for the matrix transposition: %.8f milliseconds\n", elapsed_gettimeofday*1000);
}

void matTransposeOMP(float** m, float** t){
  #ifdef _OPENMP
    wt1=omp_get_wtime();
  #endif
  #pragma omp parallel for collapse(2) schedule(auto)
  for (int i=0; i<n; i++){
    for (int j=0; j<n; j++){
      t[i][j]=m[j][i];
    }
  }
  //stop timer
  #ifdef _OPENMP
    wt2=omp_get_wtime();
    printf("    wall-clock time for the matrix transposition: %.8f milliseconds\n", (wt2-wt1)*1000);
  #endif
}

int main(int argc, char *argv[]) {
  //check if the dimension of the matrix is valid
  while(checkPower(n)!=1){
    printf("Enter the dimension of the matrix (must be a power of 2)\n");
    scanf("%d", &n);
  }
  
  //allocate the matrices
  float** matrix=(float**) malloc(n*sizeof(float*));
  float** T_serial=(float**) malloc(n*sizeof(float*));
  float** T_implicit=(float**) malloc(n*sizeof(float*));
  float** T_explicit=(float**) malloc(n*sizeof(float*));
  for (int i=0; i<n; i++){
    matrix[i]=(float*) malloc(n*sizeof(float));
    T_serial[i]=(float*) malloc(n*sizeof(float));
    T_implicit[i]=(float*) malloc(n*sizeof(float));
    T_explicit[i]=(float*) malloc(n*sizeof(float));
  }
	
  //populate the initial matrix with random floating-point values
  for (int i=0; i<n; i++){
    for (int j=0; j<n; j++){
      matrix[i][j]=MIN + (float)rand() / (float)(RAND_MAX / (MAX - MIN));;
    }
  }
  
  //check if the allocation failed
  if (matrix == NULL){
     printf("memory allocation failed");
     return 1;
  }
  
  printf("\nSEQUENTIAL EXECUTION _________________________________________________\n");
  if (checkSym(matrix)==1){
     printf("the matrix is symmetic\n");
     return 0;
  }else{
    //transpose
    matTranspose(matrix, T_serial);
  }
  
  
  printf("\nIMPLICIT PARALLELISM EXECUTION ________________________________________\n");
  if (checkSymIMP(matrix)==1){
     printf("the matrix is symmetic\n");
     return 0;
  }else{
    //transpose
    matTransposeIMP(matrix, T_implicit);
    if (check_correctness(T_serial, T_implicit)==1){
      printf("transposition failed!");
      return 1;
    }
  }
  
  printf("\nEXPLICIT PARALLELISM EXECUTION ________________________________________\n");
  if (checkSymOMP(matrix)==1){
     printf("the matrix is symmetic\n");
     return 0;
  }else{
    //transpose
    matTransposeOMP(matrix, T_explicit);
    if (check_correctness(T_serial, T_explicit)==1){
      printf("transposition failed!");
      return 1;
    }
  }

  //free the allocated memory
  for (int i = 0; i < n; i++) {
      free(matrix[i]);
      free(T_serial[i]);
      free(T_implicit[i]);
      free(T_explicit[i]);
  }
  free(matrix);
  free(T_serial);
  free(T_implicit);
  free(T_explicit);
 return 0;
 }
```
