#!/bin/sh
#sbatch -p el8-rpi -N 1 --gres=gpu:4 -t 5 -o sim4.stdout -e sim4.stderr ./sim4.sh

module load xl_r spectrum-mpi cuda

taskset -c 0-159:4 mpirun -N 4 /gpfs/u/home/PCPC/PCPCtttl/scratch/a.out -c