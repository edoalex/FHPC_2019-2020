
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

#if !defined(_OPENMP)
#error you need to use OpenMP to compile this code, use the appropriated flag for your compiler
#endif

#if defined(__STDC__)
#  if (__STDC_VERSION__ >= 199901L)
#     define _XOPEN_SOURCE 700
#  endif
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>


//
// HINT: play a bit with this code and the
//       environmental variables:
//       OMP_NUM_THREADS=n_0, n_1, n_2, ...
//       OMP_NESTED=<true|false>
//       OMP_MAX_ACTIVE_LEVELS=n
//


#if !defined(WATCH_THREADS)
#define WATCH_THREADS 2.5
#endif

#if !defined(WAIT)
#define WAIT 0
#endif

#define NAME_LENGTH         2
#define LEVEL_LENGTH        2
#define TAG_LENGTH  (NAME_LENGTH + LEVEL_LENGTH + 2)


/* ------------------------------------------------------------
 * 
 * some useful macros to keep the
 * source more readable
 *
 */


//
#define MIN( x, y ) ( ((x)<=(y))? (x) : (y) )


//
#define ENFORCE_ORDERED_OUTPUT( ORDER, ID, MSG ) {	\
  int done = 0;						\
  while( !done ) { if ( (ORDER) == (ID) ) {		\
    printf("%s", (MSG)); (ORDER)++; done=1; } } }


//
#define GET_LEVEL_INFO				\
  int myid = omp_get_thread_num();		\
  int this_level = omp_get_active_level();			\
  int eff_level  = MIN( max_nesting_level, this_level);		\
								\
  char buffer[max_nesting_level+1];				\
  memset( buffer, 0, max_nesting_level+1);			\
  for( int ii = 0; ii < eff_level; ii++)			\
    buffer[ii] = '\t'; 


//
#define SETUP_MYNAME 						\
  char myname[ strlen(father_name) + 1 + TAG_LENGTH + 1];	\
  if ( this_level > 1 )							\
    sprintf( myname, "%s-%0*d.%0*d", father_name, LEVEL_LENGTH, this_level, NAME_LENGTH, myid);	\
  else									\
    sprintf( myname, "%0*d.%0*d", LEVEL_LENGTH, this_level, NAME_LENGTH, myid);	\
									\
  char message[1000];							\
  sprintf(message, "%s%s at level %d/%d\n",				\
	  buffer, myname,this_level, max_nesting_level);

/*
 * ------------------------------------------------------------ */


/* ------------------------------------------------------------
 * 
 * funciton declaration &
 * global variables
 *
 */

int function( char*, int );


int nesting_is_active = 0;
int max_nesting_level = 1;

/*
 * ------------------------------------------------------------ */






int main( int argc, char **argv )
{
  int nthreads = 4;
  if ( argc > 1 )
    nthreads = atoi( *(argv+1) );

  if ( WAIT > 0 )
    {
      printf("\nwaiting %d seconds for you to start \"top -H -p %d\"\n",
	     WAIT, getpid());
      sleep(WAIT);
    }
  else
    printf("\nin case you want to spy me, compile with\n"
	   "  -DWAIT=XX\n"
	   "with XX large enough (for instance, ~10-15 seconds)\n\n");
  

#pragma omp parallel num_threads(nthreads)
#pragma omp single
    {
      if( nesting_is_active  = omp_get_nested() )
	  max_nesting_level  = omp_get_max_active_levels();
    }

    function( "00.00", nthreads );


  return 0;
}


int function( char *father_name, int next )
{
    if ( next < 2)
      {
    	sleep( WATCH_THREADS );
    	return 0;
      }

    int order = 0;
    
#pragma omp parallel num_threads(next)
    {
      // here each thread gets its own id and
      // the current level of parallel nesting
      //
      GET_LEVEL_INFO;

      // here each thread just setup the string
      // which contains his father's name and
      // its own "$level.$id"
      //
      SETUP_MYNAME;

      // here the strings just prepared are
      // printed respecting the id order
      //
      ENFORCE_ORDERED_OUTPUT( order, myid, message );

      // wait for all the threads
      // to arrive at this point, just to
      // keep the output more readable
      //
      #pragma omp barrier

      // sleep few seconds, so if you are watching
      // through top -H -p #PID you can see
      // the threads' creation
      //
      sleep( WATCH_THREADS );

      // recursively call to nest one more level
      //
      function( myname, next/2 );
    }

    return 0;
}
