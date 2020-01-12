cd FHPC_2019-2020/Assignements/Assignment03/Mandelbrot
module load intel/18.4
#icc openmpd3.c -fopenmp -std=c99 -lrt -o openmpd3.x 
#icc openmpd4.c -fopenmp -std=c99 -lrt -o openmpd4.x
#icc openmpg.c -fopenmp -std=c99 -lrt -o openmpg.x
#icc openmpt.c -fopenmp -std=c99 -lrt -o openmpt.x
icc openmp_task.c -fopenmp -std=c99 -lrt -o openmp_task.x
icc openmp_dyn.c -fopenmp -std=c99 -lrt -o openmp_dyn.x
echo -n > dat.dat
export KMP_AFFINITY=granularity=fine,scatter
export TIMEFORMAT='%3R'
for proc in {1..20}; do
#for chunk in {1,3,5,7,10,25,35,50,70,80,100,300,500,700,1000}; do
    export OMP_NUM_THREADS=${proc}
    #(time ./openmpd3.x | grep adr) > rawd3.dat 2>&1
    #(time ./openmpd4.x | grep arc) > rawd4.dat 2>&1
    #(time ./openmpg.x | grep arag) > rawg.dat 2>&1
    #(time ./openmpt.x | grep arrv) > rawt.dat 2>&1
    (time ./openmp_task.x | grep eCC) > raw_task.dat 2>&1
    (time ./openmp_dyn.x | grep asgb) > raw_dyn.dat 2>&1
    #d3=$(cat rawd3.dat)
    #d4=$(cat rawd4.dat)
    #g=$(cat rawg.dat)
    #t=$(cat rawt.dat)
    task=$(cat raw_task.dat)
    dyn=$(cat raw_dyn.dat)
    #echo -n $d3 >> dat.dat
    #echo -n " " >> dat.dat
    #echo -n $d4 >> dat.dat
    #echo -n " ">> dat.dat
    #echo -n $g >> dat.dat
    #echo -n " ">> dat.dat
    #echo -n $t >> dat.dat
    #echo -n " ">> dat.dat
    echo -n $task >> dat.dat
    echo -n " ">> dat.dat
    echo $dyn >> dat.dat
    #printf '%f\t%f\t%f\t%f\t%f\t%f\n' "$d3" "$d4" "$g" "$t" "$tl" "$s" >> dat.dat
    done