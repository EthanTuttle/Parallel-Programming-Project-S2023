#!/bin/sh
#sbatch -p el8-rpi -N 1 --gres=gpu:4 -t 5 -o sim32.stdout -e sim32.stderr ./sim32.sh

module load xl_r spectrum-mpi cuda

taskset -c 0-159:4 mpirun -N 32 /gpfs/u/home/PCPC/PCPCtttl/scratch/a.out -c