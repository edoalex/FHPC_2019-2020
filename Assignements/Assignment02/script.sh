for proc in 1 2 3 4 5 6 7 8 ; do
    export OMP_NUM_THREADS=${proc}
    ./06_touch_by_all.x 100000000 | grep Sum | cut -d " " -f 6 >> out01
    echo , >> out01
    done
