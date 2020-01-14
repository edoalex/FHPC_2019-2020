
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

#if defined(_OPENMP)
#define CPU_TIME (clock_gettime( CLOCK_REALTIME, &ts ), (double)ts.tv_sec + \
		  (double)ts.tv_nsec * 1e-9)
#else

#define CPU_TIME (clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts ), (double)ts.tv_sec + \
		  (double)ts.tv_nsec * 1e-9)
#endif

#define I_max_default 255
#define x_L_default -2.5
#define y_L_default -1.25
#define x_R_default 1
#define y_R_default 1.25
#define n_x_default 4200
#define n_y_default 3000
#define BEST_CHUNK_SIZE 10

int get_cpu_id( void );
void write_pgm_image( void *image, int maxval, int xsize, int ysize, const char *image_name);
unsigned char compute_mandelbrot(const double c_x, const double c_y, short int I_max);

int main(int argc, char* argv[]){
  struct  timespec ts;
  double x_L, x_R, y_L, y_R;
  int n_x, n_y;
  short int I_max;

#if defined(_OPENMP)

  int nthreads;
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
  
  //----------------------------------
  //       setting parameters
  //----------------------------------

  if(argc>2){
    n_x = atoi(argv[1]);
    n_y = atoi(argv[2]);
  }
  else{
    n_x = n_x_default;
    n_y = n_y_default;
  }

  if(argc>7){
    x_L = atof(argv[3]);
    y_L = atof(argv[4]);
    x_R = atof(argv[5]);
    y_R = atof(argv[6]);
    I_max = atoi(argv[7]);
  }
  else{
    x_R = x_R_default;
    x_L = x_L_default;
    y_R = y_R_default;
    y_L = y_L_default;
    I_max = I_max_default;
  }

  printf("n_x=%d \tn_y=%d \tx_L=%f \ty_L=%f \tx_R=%f \ty_R=%f \tI_max=%d \n", n_x, n_y, x_L, y_L, x_R, y_R, I_max);

  //-----------------------------------
  //               computing
  //-----------------------------------

  unsigned char* matrix;

  if( (matrix = (unsigned char*)calloc( n_x * n_y, sizeof(unsigned char))) == NULL){
      printf("I'm sorry, there is not enough memory to host %ld bytes\n\n", n_x * n_y * sizeof(unsigned char));
      return 1;
    }

  double delta_x = (x_R - x_L)/(n_x - 1);
  double delta_y = (y_R - y_L)/(n_y - 1);
  double x, y;

#pragma omp parallel for collapse(2) schedule(dynamic, (n_x*n_y/nthreads)/ BEST_CHUNK_SIZE)
  for(unsigned int i=0; i<n_y; ++i){
    for(unsigned int j=0; j<n_x; ++j){
      // work with matrix[i*n_x + j] that is matrix[i][y]
      y = y_R - i*delta_y;
      x = x_L + j*delta_x;
      matrix[i*n_x + j] = compute_mandelbrot(x, y, I_max);
    }
  }

  //-----------------------------------
  //             drawing image
  //-----------------------------------

  /*  for(unsigned int i=0; i<n_y; ++i){
    for(unsigned int j=0; j<n_x; ++j){
      printf("%d ", matrix[i*n_x + j]);
    }
    printf("\n");
  }
  */


  void* ptr = (void*)matrix;
  //  write_pgm_image( ptr, I_max, n_x, n_y, "image.pgm" );
  free(matrix);

  return 0;
#else

  printf("To run this code, you need to compile it using openmp flag:\tgcc -fopenmp \n");
  return 1;

#endif
}



void write_pgm_image( void *image, int maxval, int xsize, int ysize, const char *image_name)
{
  FILE* image_file; 
  image_file = fopen(image_name, "w");
  
  int color_depth = 1+((maxval>>8)>0);       // 1 if maxval < 256, 2 otherwise

  fprintf(image_file, "P5\n%d %d\n%d\n", xsize, ysize, maxval);
  
  fwrite( image, color_depth, xsize*ysize, image_file);  

  fclose(image_file);
  return;
}

unsigned char compute_mandelbrot(const double c_x, const double c_y, short int I_max){
  double x=0;
  double y=0;
  double mod_sq=0;
  short int iteration=0;

  while( ( mod_sq<4 ) && (iteration < I_max) ){
    y = 2*x*y + c_y;
    x = 2*x*x - mod_sq + c_x;    // a bit of math to get here (avoiding creation of a temp variable)
    mod_sq = x*x +y*y;
    ++iteration;
  }
  iteration = (iteration == I_max) ? 0 : iteration;

  unsigned char ret = iteration;
  return ret;
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
