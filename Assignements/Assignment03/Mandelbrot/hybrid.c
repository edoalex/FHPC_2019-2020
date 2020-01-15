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
#include <mpi.h>
#include <omp.h>

#define I_max_default 255
#define x_L_default -2.5
#define y_L_default -1.25
#define x_R_default 1
#define y_R_default 1.25
#define n_x_default 4200
#define n_y_default 3000
#define N_THREADS_PER_PROCESS 3

//----------------------------------------------------------------------------------------
//        this hybrid version will exploit p processes, each of which will spawn 
//        N_THREADS_PER_PROCESS threads. To make things more interesting, 
//        N_THREADS_PER_PROCESS lines of pixel have been assigned as a "pack of job"
//        to a process in a master call. In this way the time taken for a process to 
//        complete the job will be the same as in the simpler MPI version of this code.
//----------------------------------------------------------------------------------------

int get_cpu_id( void );
int write_pgm_header(int maxval, int xsize, int ysize, const char *image_name);
unsigned char compute_mandelbrot(const double c_x, const double c_y, short int I_max);
int main(int argc, char * argv[]){
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
    I_max = atoi(argv[7]);
  }
  else{
    x_R = x_R_default;
    x_L = x_L_default;
    y_R = y_R_default;
    y_L = y_L_default;
    I_max = I_max_default;
  }

  int myid, numproc;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  
  MPI_File file;
    
  int header_offset;
  // write header in pgm file
  if(myid == 0){
    header_offset = write_pgm_header(I_max, n_x, n_y, "image.pgm");
  }

  int root = 0;
  MPI_Bcast(&header_offset, 1, MPI_INT, root, MPI_COMM_WORLD);
  MPI_File_open(MPI_COMM_WORLD, "image.pgm", MPI_MODE_WRONLY, MPI_INFO_NULL, &file);

  // define some dummy variables to make code more readable   
  int im_ready_tag = 0;
  int workpile_tag = 1;
  int finish_tag = 42;
  int ntpp = N_THREADS_PER_PROCESS;

  double start_t, end_t;
  start_t = MPI_Wtime();

  if(myid == 0){
    // printf("n_x=%d \tn_y=%d \tx_L=%f \ty_L=%f \tx_R=%f \ty_R=%f \tI_max=%d \n", n_x, n_y, x_L, y_L, x_R, y_R, I_max);
    int* line = (int*)malloc( 2 * sizeof(int));      // line[0] will contain the number of the first line to compute
    line[0] = 0;      // line[1] will contain the number of lines to compute
    line[1] = ntpp;
    int worker;
    while(line[0] < n_y - ntpp + 1 ){
    // listen who says 'Im ready'
    MPI_Recv(&worker, 1, MPI_INT, MPI_ANY_SOURCE, im_ready_tag, MPI_COMM_WORLD, &status);
    // send him ntpp lines to work on, starting from number=line[0]
    MPI_Send(line, 2, MPI_INT, worker, workpile_tag, MPI_COMM_WORLD);
    line[0] += ntpp;
    }
    //send the final remaining (line[0] - n_y) < ntpp  lines to next and last process
    line[1] = line[0] - n_y;    
    if(line[1] > 0){
    line[0] -= 4;
    MPI_Recv(&worker, 1, MPI_INT, MPI_ANY_SOURCE, im_ready_tag, MPI_COMM_WORLD, &status);
    MPI_Send(&line, 2, MPI_INT, worker, workpile_tag, MPI_COMM_WORLD);
    }
    // send message with special tag to say we're done
    for(int slave=1; slave < numproc; ++slave){
      MPI_Recv(&worker, 1, MPI_INT, MPI_ANY_SOURCE, im_ready_tag, MPI_COMM_WORLD, &status);
      MPI_Send(&slave, 0, MPI_INT, worker, finish_tag, MPI_COMM_WORLD);
    }
  }
  else{
    int* line = (int*)malloc( 2 * sizeof(int)); // line[0] will contain the number of the first line to compute 
                 // line[1] will contain the number of lines to compute          
    unsigned char* buffer = (unsigned char*)malloc( ntpp*n_x*sizeof(unsigned char) );
    double x, y;
    double delta_x = (x_R - x_L)/(n_x - 1);
    double delta_y = (y_R - y_L)/(n_y - 1);

    /*
#pragma omp parallel num_threads(ntpp)
    {
      int all = omp_get_num_threads();
      int me = omp_get_thread_num();
      printf("I'm thread %d/%d of process %d/%d and i'm running on core %d\n", me, all, myid, numproc, get_cpu_id());
    }
    */

    while( 1 ){
      // say to master you're ready to work
      MPI_Send(&myid, 1, MPI_INT, root, im_ready_tag, MPI_COMM_WORLD);
      // listen your next job
      MPI_Recv(line, 2, MPI_INT, root, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if(status.MPI_TAG == finish_tag){
	break;
      }
      // work from line line[0] until line ( line[0] + line[1] - 1 )
#pragma omp parallel num_threads(ntpp)
	  {
#pragma omp for schedule(guided) collapse(2)
	for(unsigned int j=0; j < line[1]; j++){
	  for(unsigned int i = 0; i < n_x; ++i){
	    y = y_R - (line[0]+j) * delta_y;
  	    x = x_L + i*delta_x;
	    buffer[i + n_x*j] = compute_mandelbrot(x, y, I_max);
          }
	}
      }
      // write your results on file
	  MPI_File_write_at(file, (header_offset + line[0]*n_x), buffer, (n_x*line[1]), MPI_UNSIGNED_CHAR, &status);
    }
    free(buffer);
  }

  
  end_t = MPI_Wtime();

  printf("I'm %d out of %d\t time: %f\n", myid, numproc, end_t - start_t);
  MPI_File_close(&file);
  MPI_Finalize();
  return 0;
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

int write_pgm_header(int maxval, int xsize, int ysize, const char *image_name)
{
  FILE* image_file; 
  image_file = fopen(image_name, "w"); 
  int length;
  length = fprintf(image_file, "P5\n%d %d\n%d\n", xsize, ysize, maxval);  
  fclose(image_file); 
  return length;
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
