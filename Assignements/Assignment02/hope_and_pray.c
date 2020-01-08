#if defined(__STDC__)
#  if (__STDC_VERSION__ >= 199901L)
#     define _XOPEN_SOURCE 700
#  endif
#endif
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#include <omp.h>
#include <math.h>


#define N_default 1000

#if defined(_OPENMP)
#define CPU_TIME (clock_gettime( CLOCK_REALTIME, &ts ), (double)ts.tv_sec + \
                  (double)ts.tv_nsec * 1e-9)
#else

#define CPU_TIME (clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts ), (double)ts.tv_sec + \
                  (double)ts.tv_nsec * 1e-9)
#endif

int get_cpu_id();

int main(int argc, char* argv[]){
  struct  timespec ts;
  long long int N = N_default;
  int* y;

  // ------------------------------                                
  //      saying 'hello I'm...'              
  // ------------------------------
    int nthreads;
#if defined(_OPENMP)
#pragma omp parallel

    {
#pragma omp master
    {
      nthreads = omp_get_num_threads();
      printf("\nomp summation with %d threads\n", nthreads );
    }
    int me = omp_get_thread_num();
#pragma omp critical
    {
      printf("thread %2d is running on core %2d\n", me, get_cpu_id() );
    }
  }
#endif

  //---------------------------------                                
  //       allocating memory
  //---------------------------------

    if(argc>1)
    N = atoi(argv[1]);
  else
    printf("\nIf you want to choose array's lenght, just give desired lenght as argument while running\n");

 

  double t_start = CPU_TIME;

#if !defined(_OPENMP)

  int * x;
  if ( ((x = (int*)malloc( N * sizeof(int) )) == NULL) || ((y = (int*)malloc( N * sizeof(int) )) == NULL)){
    printf("I'm sorry, on some thread there is not enough memory to host %llu bytes\n\n", N * sizeof(int));
    return 1;
  }
   
  for(long long int i=0; i<N; i++)
    x[i] = 1;
  
  y[0] = x[0];
  for(long long int i=1; i<N; i++)
    y[i] = y[i-1] + x[i];

  free(x);
  
#else
  
   int max_split=0;
   int temp=N;
   int* level_length;
   while(temp > nthreads){
     temp /= 2;
     max_split += 1;
   }
   printf("height of pyramid is %d\n", max_split);

   level_length =  (int*)malloc( (max_split + 1) * sizeof(int) );
   for(unsigned int level=0; level <= max_split; level++)
     level_length[level] = N/pow(2,level);

   int ** pyramid;
   pyramid =  (int**)malloc( (max_split + 1) * sizeof(int*) );

   for(unsigned int level=0; level <= max_split; level++)
     pyramid[level] =  (int*)malloc( level_length[level] * sizeof(int) );





   /*




 // ------------------------------------ 
  //      initializing input array                                   
  // ------------------------- ----------                                 
  #pragma omp parallel for
   for(long long int i=0; i<N; i++)
     pyramid[0][i] = 1;

  if ( (y = (int*)malloc( N * sizeof(int) )) == NULL ){
     printf("I'm sorry, on some thread there is not enough memory to host %llu bytes\n\n", N * sizeof(int));
     return 1;
   }
   
  // ------------------------------------   
  //           computing                                    
  // ------------------------------------


  // ------------ first part: going up the pyramid
     
     for(unsigned int level=1; level <= max_split; ++level){
       // Let's build layer level, through layer level-1
       #pragma omp parallel for
       for(unsigned int i=0; i<level_length[level]; i++)
	 pyramid[level][i] = pyramid[level - 1][2*i] + pyramid[level - 1][2*i + 1];
     }

  // ------------ second part: going down the pyramid, filling the output array level by level
     
     // ---------------- the last level must be done serially
     y[(int)pow(2, max_split) - 1] = pyramid[ max_split ][ 0 ];
     // building prefix sum of layer max_split
     for(unsigned int i=1; i<level_length[max_split]; ++i)
        y[(i+1)*(int)pow(2, max_split) - 1] = y[i*(int)pow(2, max_split) - 1] + pyramid[ max_split ][ i ];
     
     for(int level=max_split-1; level >=0; --level){
       // building the prefix sum of layer level, from prefix sum of level+1 and from pyramid layer level+1
      y[(int)pow(2, level) - 1] = pyramid[ level ][ 0 ]; 
#pragma omp parallel for shared(y,pyramid)
      for(unsigned int i=1; i<level_length[level]; i += 2)
	y[(i+2)*(int)pow(2,level) - 1] = y[(i+1)*(int)pow(2,level) - 1] + pyramid[level][i+1];
      }







   */






#endif

  double t_end  = CPU_TIME;

   printf("\nFinal element: \t\t %d \t(expected one:\t%lld)\n", y[N-1], N);
   printf("Parallel wall-time: \t%f s\n\n", t_end - t_start);

   free(y);

#if defined(_OPENMP)
   //   for(unsigned int level=0; level <= max_split; level++)
   //  free(pyramid[level]);   
   //free(pyramid);
   //free(level_length);
#endif
   
  return 0;
}


int get_cpu_id( void )
{
#if defined(_GNU_SOURCE)                              // GNU SOURCE ------------                                                                         

  return  sched_getcpu( );

#else

#ifdef SYS_getcpu                                     //     direct sys call ---                                                                         

  int cpuid;
  if ( syscall( SYS_getcpu, &cpuid, NULL, NULL ) == -1 )
    return -1;
  else
    return cpuid;

#else

  unsigned val;
  if ( read_proc__self_stat( CPU_ID_ENTRY_IN_PROCSTAT, &val ) == -1 )
    return -1;

  return (int)val;

#endif                                                // -----------------------                                                                         
#endif

}
