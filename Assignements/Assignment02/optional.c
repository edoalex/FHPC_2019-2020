
/* ────────────────────────────────────────────────────────────────────────── *
 │                                                                            │
 │ This file is part of the exercises for the Lectures on                     │
 │   "Foundations of High Performance Computing"                              │
 │ given at                                                                   │
 │   Master in HPC and                                                        │
 │   Master in Data Science and Scientific Computing                          │
 │ @ SISSA, ICTP and University of Trieste                                    │
 │                                                                            │
 │ contact: luca.tornatore@inaf.it                                            │
 │                                                                            │
 │     This is free software; you can redistribute it and/or modify           │
 │     it under the terms of the GNU General Public License as published by   │
 │     the Free Software Foundation; either version 3 of the License, or      │
 │     (at your option) any later version.                                    │
 │     This code is distributed in the hope that it will be useful,           │
 │     but WITHOUT ANY WARRANTY; without even the implied warranty of         │
 │     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          │
 │     GNU General Public License for more details.                           │
 │                                                                            │
 │     You should have received a copy of the GNU General Public License      │
 │     along with this program.  If not, see <http://www.gnu.org/licenses/>   │
 │                                                                            │
 * ────────────────────────────────────────────────────────────────────────── */
  
#define _GNU_SOURCE
#if defined(__STDC__)
#  if (__STDC_VERSION__ >= 199901L)
#     define _XOPEN_SOURCE 700
#  endif
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>


#if defined(_OPENMP)
#define CPU_TIME (clock_gettime( CLOCK_REALTIME, &ts ), (double)ts.tv_sec + \
		  (double)ts.tv_nsec * 1e-9)
#define CPU_TIME_th (clock_gettime( CLOCK_THREAD_CPUTIME_ID, &myts ), (double)myts.tv_sec + \
		     (double)myts.tv_nsec * 1e-9)
#else
#define CPU_TIME (clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts ), (double)ts.tv_sec + \
		   (double)ts.tv_nsec * 1e-9)
#endif

#define CPU_ID_ENTRY_IN_PROCSTAT 39
#define HOSTNAME_MAX_LENGTH      200
#define N_default 100

int read_proc__self_stat ( int, int * );
int get_cpu_id           ( void       );

int main( int argc, char **argv )
{

  int     N        = N_default;
  int     nthreads = 1;
  struct  timespec ts;
  double *array;

  char *places = getenv("OMP_PLACES");
  char *bind   = getenv("OMP_PROC_BIND");
  if ( places != NULL )
    printf("\nOMP_PLACES is set to %s\n", places);
  if ( bind != NULL )
    printf("OMP_PROC_BIND is set to %s\n", bind);

#pragma omp parallel
  {

#pragma omp master
    {
      nthreads = omp_get_num_threads();
      printf("+ %d threads in execution - -\n", nthreads );
    }
    int me = omp_get_thread_num();

#pragma omp critical
    printf("thread %2d is running on core %2d\n", me, get_cpu_id() );
  }

  // check whether some arg has been passed on
  if ( argc > 1 )
    N = atoi( *(argv+1) );

  // allocate memory
  if ( (array = (double*)malloc( N * sizeof(double) )) == NULL )
    {
      printf("I'm sorry, there is not enough memory to host %lu bytes\n", N * sizeof(double) );
      return 1;
    }

  // just give notice of what will happen and get the number of threads used
#ifndef _OPENMP
  printf("\nserial summation\n");
#else
#pragma omp parallel
  {
#pragma omp master
    {
      nthreads = omp_get_num_threads();
      printf("\nomp summation with %d threads\n", nthreads );
      printf("Every thread will deal with array, from index 'start' until index 'stop'\n");
    }
  }
#endif

  /*  -----------------------------------------------------------------------------
   *   calculate
   *  -----------------------------------------------------------------------------
   */

  double S           = 0;                                   // this will store the summation
  double th_avg_time = 0;                                   // this will be the average thread runtime
  double th_min_time = 10;                                   // this will be the min thread runtime.
							    // contrasting the average and the min
							    // time taken by the threads, you may
							    // have an idea of the unbalance.


#if !defined(_OPENMP)
  double tstart  = CPU_TIME;
  for( int i = 0; i < N; ++i)
    array[i] = (double)i;
  
  for ( int ii = 0; ii < N; ii++ )                        
    S += array[ii];                                       

#else

  double tstart;  // as a wall-clock time we want to count only the computaional part, not the initialization too
#pragma omp parallel reduction(+:th_avg_time) reduction(min:th_min_time) reduction(+:S) 
{
  int me = omp_get_thread_num();
  int remainder = N % nthreads;
  long long int N_each = N / nthreads;
  int start, stop;

  // now we just equally split the initialization and summation among the different threads:

  /*    if(me < remainder){                                                                                                                            
      start = me*(N_each+1);                                                           
      ++N_each;                                                                                                 
    }                                                                                                                   
    else    
      start = me*N_each + remainder;                                    
  */
  // the piece of code above is the same as the following 2 lines:

  start = (me < remainder) ? ( me*(N_each+1) ):(me*N_each + remainder);
  N_each += (me < remainder) ? 1:0;
  stop = start + N_each;
  printf("I'm %d\tmy start:%d\tmy stop:%d\n", me, start, stop);                        

  //--------------------------------                                                                                               
  //       initialization                                                                                                                  
  //-------------------------------- 

  // every thread initializes (and physically allocates in memory) its chunk of the array
  for(unsigned int i=start; i<stop; i++)
    array[i] = (double)i;

    struct  timespec myts;                   
    double mystart = CPU_TIME_th;        

#pragma omp master
{
  tstart  = CPU_TIME;   // computational part is coming, start the clock
}      
  //--------------------------------                                                                                                      
  //       computation                                                                                                              
  //--------------------------------           
                          
 for ( int ii = start; ii < stop; ii++ )   // note that the scheduling must be the same! (between initial. and computing)
   S += array[ii];                         // otherwise the whole touch-stuff is pointless

    double mytime = CPU_TIME_th - mystart;

    th_avg_time += mytime;
    th_min_time  = (mytime < th_min_time)? mytime : th_min_time;
    
  }

#endif

  double tend = CPU_TIME; 
			
  /*-------------------------------------------------------------------------------
   *   finalize
   *  -----------------------------------------------------------------------------
   */

printf("Sum is %g, process took %g of wall-clock time\n\n"
       "<%g> sec of avg thread-time\n"
       "<%g> sec of min thread-time\n",
       S, tend - tstart, th_avg_time/nthreads, th_min_time );
  
  free( array );
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


int read_proc__self_stat( int field, int *ret_val )
{
  *ret_val = 0;

  FILE *file = fopen( "/proc/self/stat", "r" );
  if (file == NULL )
    return -1;

  char   *line = NULL;
  int     ret;
  size_t  len;
  ret = getline( &line, &len, file );
  fclose(file);

  if( ret == -1 )
    return -1;

  char *savetoken = line;
  char *token = strtok_r( line, " ", &savetoken);
  --field;
  do { token = strtok_r( NULL, " ", &savetoken); field--; } while( field );

  *ret_val = atoi(token);

  free(line);

  return 0;
}
