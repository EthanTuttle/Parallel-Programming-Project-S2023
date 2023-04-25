#!/bin/sh
#sbatch -p el8-rpi -N 8 --gres=gpu:4 -t 5 -o sim256.stdout -e sim256.stderr ./sim256.sh

module load xl_r spectrum-mpi cuda

taskset -c 0-159:4 mpirun -N 32 /gpfs/u/home/PCPC/PCPCtttl/scratch/a.out -c