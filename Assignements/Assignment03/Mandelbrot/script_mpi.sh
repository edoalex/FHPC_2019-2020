cd FHPC_2019-2020/Assignements/Assignment03/Mandelbrot
module load openmpi
#icc openmpd3.c -fopenmp -std=c99 -lrt -o openmpd3.x                               
mpicc mpi.c -std=c99 -o mpi.x
echo -n > dat.dat
#export KMP_AFFINITY=granularity=fine,scatter
#export TIMEFORMAT='%3R'
for proc in {2..20}; do
    #export OMP_NUM_THREADS=${proc}
    #(time ./openmpt.x | grep arrv) > rawt.dat 2>&1                                
    mpirun -np $proc mpi.x | grep time | cut -d " " -f 7 | sort | sed -n '$p' >> dat.dat
    #dyn=$(cat raw_dyn.dat)
    #echo $dyn >> dat.dat
    #printf '%f\t%f\t%f\t%f\t%f\t%f\n' "$d3" "$d4" "$g" "$t" "$tl" "$s" >> dat.dat 
    done
