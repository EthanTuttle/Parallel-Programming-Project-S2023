#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
//#include "clockcycle.h"

#define population 8
#define connections 5
#define infectChance 0.25
#define dieChance 0.05
#define clock_frequency 512000000

struct Person
{
    long long int id;
    int status; // 0 = alive/not infected, 1 = infected, 2 = dead
    long long int friends[connections];
};


int main(int argc, char *argv[]) {

    // mpi init
    MPI_Init(&argc, &argv);
    int myrank;
    int numranks;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &numranks);

    srand(myrank);
    int i = myrank;
    double localSum = 0;
    while (i < population) {
        struct Person p = {i, 0}; 
        long long int friends[connections];
        for (int c = 0; c < connections; c++) {
            friends[c] = rand() % connections;
        }
        memcpy(p.friends, friends, connections * sizeof(long long int));
        i+= numranks;
    }

    

    MPI_Finalize();
}