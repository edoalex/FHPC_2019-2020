!/usr/bin/bash
#PBS -l nodes=1:ppn=20
#PBS -l walltime=00:10:00

cd FHPC_2019-2020/Assignements/Assignment03/Mandelbrot
module load intel/18.4
icc openmp.c -fopenmp -std=c99 -lrt -o openmp.x
icc openmp_w.c -fopenmp -std=c99 -lrt -o openmp_w.x -lm
echo -n > dat_os.dat
echo -n > dat_ow.dat
export KMP_AFFINITY=granularity=fine,scatter
export TIMEFORMAT='%3R'
for thread in {1..20}; do
    export OMP_NUM_THREADS=${thread}
    (time ./openmp.x | grep asgb) > raw.dat 2>&1
    (time ./openmp_w.x | grep asgb) > raw_w.dat 2>&1
    op=$(cat raw.dat)
    echo $op >> dat_os.dat
    op=$(cat raw_w.dat)
    echo $op >> dat_ow.dat
    #printf '%f\t%f\t%f\t%f\t%f\t%f\n' "$d3" "$d4" "$g" "$t" "$tl" "$s" >> dat.dat
    done

module unload intel/18.4
module load openmpi

mpicc mpi.c -std=c99 -o mpi.x
mpicc mpi_w.c -std=c99 -o mpi_w.x -lm
echo -n > dat_ms.dat
echo -n > dat_mw.dat
for proc in {2..20}; do
    mpirun -np $proc mpi.x | grep time | cut -d " " -f 7 | sort | sed -n '$p' >> dat_ms.dat
    mpirun -np $proc mpi_w.x | grep time | cut -d " " -f 7 | sort | sed -n '$p' >> dat_mw.dat
    done