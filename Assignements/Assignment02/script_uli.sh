cd FHPC_2019-2020/Assignements/Assignment02                                                                                                           
module load intel/18.4                                                                                                                                  
#export KMP_AFFINITY=granularity=fine,scatter                                                                                                           
#export TIMEFORMAT='%3R'                                                                                                                             
export OMP_PLACES=CORES
export OMP_PROC_BINDINGS=CLOSE
echo -n > DATI.dat                                                                                                                                   
icc 06_touch_by_all.c -o 06_touch_by_all.x -std=c99 -lrt -fopenmp                                                                                      
icc 01_array_sum.c -o 01_array_sum.x -std=c99 -lrt -fopenmp
icc optional.c -o optional.x -std=c99 -lrt -fopenmp                       
#(time ./01_array_sum.x 1000000000 | grep Sum | cut -d " " -f 6 ) >> dat_01 2>&1   
#(time ./06_touch_by_all.x 1000000000 | grep Sum | cut -d " " -f 6 ) >> dat_06 2>&1                                                                     
#icc 06_touch_by_all.c -o 06_touch_by_all.x -std=c99 -lrt -DOUTPUT -fopenmp                                                                            
#icc 01_array_sum.c -o 01_array_sum.x -fopenmp -std=c99 -lrt                                                                                           
#icc prefix_sum_blocks.c -o prefix_sum_blocks.x -fopenmp -std=c99 -lrt -O3 -march=native
#icc prefix_sum_blocks_rest.c -o prefix_sum_blocks_rest.x -fopenmp -std=c99 -lrt -O3 -march=native
#icc prefix_sum_blocks_g.c -o prefix_sum_blocks_g.x -fopenmp -std=c99 -lrt -O3
for proc in {1..20}; do                                                                                                                                 
    export OMP_NUM_THREADS=${proc}                                                                                                                      
    #echo -n ${proc} >> dat_01                                                                                                                         
    #echo -n " " >> dat_01                                                                                                                             
    #echo -n ${proc} >> dat_06                                                                                                                         
    #echo -n " " >> dat_06                                                                                                                             
    (./01_array_sum.x 1000000000 | grep Sum | cut -d " " -f 6 ) > raw_01                                                                   
    (./06_touch_by_all.x 1000000000 | grep Sum | cut -d " " -f 6 ) > raw_06
    (./optional.x 1000000000 | grep Sum | cut -d " " -f 6 ) > raw_op
    a=$(cat raw_01)
    b=$(cat raw_06)
    c=$(cat raw_op)
    #printf '%f\t%f\t%f\n' "$a" "$b" "$c" >> DATI.dat
    echo -n $a >> DATI.dat
    echo -n " " >> DATI.dat
    echo -n $b >> DATI.dat
    echo -n " ">> DATI.dat
    echo $c >> DATI.dat
    #./prefix_sum_blocks.x 1000000000 | grep Parallel | cut -d " " -f3 >> dat_ps
    #./prefix_sum_blocks.x 1000000000 | grep Parallel | cut -d " " -f3 >> dat_ps
    #./prefix_sum_blocks_rest.x 1000000000 | grep Parallel | cut -d " " -f3 >> dat_ps_r
    done
