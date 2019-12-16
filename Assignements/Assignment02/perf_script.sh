cd FHPC_2019-2020/Assignements/Assignment02
module load intel/18.4
icc 06_touch_by_all.c -fopenmp -std=c99 -lrt -o 06_touch_by_all.x 
icc 01_array_sum.c -fopenmp -std=c99 -lrt -o 01_array_sum.x
echo -n > dat_01.dat
echo -n > dat_06.dat
export KMP_AFFINITY=granularity=fine,scatter
for proc in {1..20}; do
    export OMP_NUM_THREADS=${proc}
    (perf stat -e cache-misses:u,cache-references:u,cycles:u,instructions:u ./01_array_sum.x 1000000000 2>&1) > raw_01.dat
    (perf stat -e cache-misses:u,cache-references:u,cycles:u,instructions:u ./06_touch_by_all.x 1000000000 2>&1) > raw_06.dat
    cm=$(cat raw_01.dat | grep cache-misses | cut -b 1-18)
    cr=$(cat raw_01.dat | grep cache-references | cut -b 1-18)
    cy=$(cat raw_01.dat | grep cycles | cut -b 1-18)
    in=$(cat raw_01.dat | grep instructions | cut -b 1-18)
    printf '%d\t%d\t%d\t%d\n' "$cm" "$cr" "$cy" "$in" >> dat_01.dat
    cm=$(cat raw_06.dat | grep cache-misses | cut -b 1-18)
    cr=$(cat raw_06.dat | grep cache-references | cut -b 1-18)
    cy=$(cat raw_06.dat | grep cycles | cut -b 1-18)
    in=$(cat raw_06.dat | grep instructions | cut -b 1-18)
    printf '%d\t%d\t%d\t%d\n' "$cm" "$cr" "$cy" "$in" >> dat_06.dat
    done