#!/bin/sh
#sbatch -p el8-rpi -N 1 --gres=gpu:4 -t 5 -o sim8.stdout -e sim8.stderr ./sim8.sh

module load xl_r spectrum-mpi cuda

taskset -c 0-159:4 mpirun -N 8 /gpfs/u/home/PCPC/PCPCtttl/scratch/a.out -c