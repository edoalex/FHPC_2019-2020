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
  register double* x;

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

  if ( (x = (double*)malloc( N * sizeof(double) )) == NULL){
    printf("I'm sorry, on some thread there is not enough memory to host %llu bytes\n\n", N * sizeof(double));
    return 1;
  }

  double t_start = CPU_TIME;

#if !defined(_OPENMP)

  for(long long int i=0; i<N; i++)
    x[i] = 1;

  for(long long int i=1; i<N; i++)
    x[i] += x[i-1];

#else

  int* stop_holder = malloc((nthreads)*sizeof(int));
#pragma omp parallel shared(x, stop_holder)
  {
  // ------------------------------------                                                                                                                
  //      initializing input array                                                                                                                       
  // ------------------------------------                                                                                                                
  #pragma omp for
    for(long long int i=0; i<N; i++)
      x[i] = 1;

  // ------------------------------------                                                                                                                
  //           computing                                                                                                                                 
  // ------------------------------------
        int me = omp_get_thread_num();
    int remainder = N % nthreads;
    long long int N_each = N / nthreads;
    int start, stop;

    /*    if(me < remainder){                                                                                                                            
      start = me*(N_each+1);                                                                                                                             
      ++N_each;                                                                                                                                          
    }                                                                                                                                                    
    else                                                                                                                                                 
      start = me*N_each + remainder;                                                                                                                     
    */

    start = (me < remainder) ? ( me*(N_each+1) ):(me*N_each + remainder);
    N_each += (me < remainder) ? 1:0;

    stop = start + N_each;
    stop_holder[me] = stop;

    // each thread will have to work with x[i] from start (included) till end (excluded)                                                                 

    // ------------------- scan by blocks ------------------------                                                                                       

    for(unsigned int i = start + 1; i<stop; i++)
      x[i] += x[i-1];
  }
      // ----------------- now join the different blocks -----------------                                                                               

    // the following piece of code means:                                                                                                                
    // for every block (not the very first one):      (we want the blocks to be handled sequentially, but we're                                          
    //                                                 not wasting parallel resources. #_of_blocks = n_threads )                                         
    //    for every element of that block:            (parallelization here)                                                                             
    //       add the last element of the previous block;                                                                                                 

  register int to_add;
  for(unsigned int i = 1; i<nthreads; i++){
    to_add = x[ stop_holder[i-1] - 1 ];                        // this variable avoids unnecessary load from x
    #pragma omp parallel for shared(x,i) schedule(guided)
    for(unsigned int k=stop_holder[i-1]; k<stop_holder[i]; k++){
       x[k] += to_add;
    }
  }

#endif

  double t_end  = CPU_TIME;

  printf("\nFinal element: \t\t %f \t(expected one:\t%lld)\n", x[N-1], N);
  printf("Parallel wall-time: \t%f s\n\n", t_end - t_start);

  free(x);
#if defined(_OPENMP)
  free(stop_holder);
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
