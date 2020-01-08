
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

#define I_max_default 300
#define x_L_default -2.5
#define y_L_default -1
#define x_R_default 1
#define y_R_default 1
#define n_x_default 3000
#define n_y_default 2000
#define MAX 32767

void write_pgm_image( void *image, int maxval, int xsize, int ysize, const char *image_name);
short int compute_mandelbrot(const double c_x, const double c_y, short int I_max);

//to do: remove openmp standards
int main(int argc, char* argv[]){
  double x_L, x_R, y_L, y_R;
  int n_x, n_y;
  short int I_max;

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
    if(atoi(argv[7]) > MAX){
      printf("I_max given %d is too large, it will be set to %d\n", atoi(argv[7]), I_max_default);
      I_max = I_max_default;
    }
    else
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

  short int* matrix;

  if( (matrix = (short int*)malloc( n_x * n_y * sizeof(short int))) == NULL){
      printf("I'm sorry, there is not enough memory to host %s bytes\n\n", n_x * n_y * sizeof(double));
      return 1;
    }

  double delta_x = (x_R - x_L)/(n_x - 1);
  double delta_y = (y_R - y_L)/(n_y - 1);
  double x, y;

  for(unsigned int i=0; i<n_y; ++i){
    y = y_R - i*delta_y;
    for(unsigned int j=0; j<n_x; ++j){
      // work with matrix[i*n_x + j] that is matrix[i][y]
      x = x_L + j*delta_x;
      matrix[i*n_x + j] = compute_mandelbrot(x, y, I_max);
    }
  }

  //-----------------------------------
  //             drawing image
  //-----------------------------------

  void* ptr = (void*)matrix;
  write_pgm_image( ptr, I_max, n_x, n_y, "image.pgm" );
  free(matrix);

  return 0;}



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

short int compute_mandelbrot(const double c_x, const double c_y, short int I_max){
  double x=0;
  double y=0;
  double mod_sq=0;
  short int iteration=0;

  while( ( mod_sq<4 ) && (iteration <= I_max) ){
    y = 2*x*y + c_y;
    x = 2*x*x - mod_sq + c_x;    // a bit of math to get here (avoiding creation of a temp variable)
    mod_sq = x*x +y*y;
    ++iteration;
  }
  //  iteration = (iteration >= I_max) ? 0 : iteration;

  return iteration;
}
