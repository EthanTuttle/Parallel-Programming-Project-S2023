#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include "queue.h"
//#include "clockcycle.h"

#define population 8 // should be evenly divisible by numranks
#define connections 5
#define infectChance 0.25
#define dieChance 0.05
#define clock_frequency 512000000
#define startInfectedChance 0.3

struct Person
{
    long long int id;
    int status; // 0 = alive/not infected, 1 = infected, 2 = dead
    long long int friends[connections]; //stores IDs of connections
};


int main(int argc, char *argv[]) {

    // mpi init
    struct Person people[population];
    MPI_Init(&argc, &argv);
    int myrank;
    int numranks;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &numranks);

    // initialize rank vars
    long long int infectedCount = 0;
    long long int infected[population/numranks]; // will hold just person IDs
    long long int deadCount = 0;
    struct Queue* infectedQueue = createQueue(population/numranks);


    // initialize structs
    srand(myrank);
    int i = myrank;
    while (i < population) {
        //generate list of connections
        long long int friends[connections];
        for (int c = 0; c < connections; c++) {
            friends[c] = rand() % connections;
        }
        //determine of node starts infected
        float chance = (double)rand()/RAND_MAX;
        int isInfected = 0;
        if (chance < startInfectedChance) {
            infected[infectedCount] = i;
            infectedCount += 1;
            isInfected = 1;
        }
        //generate struct and copy in list of connections
        struct Person p = {i, isInfected}; 
        memcpy(p.friends, friends, connections * sizeof(long long int));
        people[i] = p;

        i+= numranks;
    }

    // day cycle
    for (int i = 0; i < infectedCount; i++) {
        long long int id = infected[i];

        //check for connections being infected
        for (int c = 0; c < connections; c++) {
            float chance = (double)rand()/RAND_MAX;
            if (chance < infectChance) {
                long long int infectedID = people[id].friends[c];
                // if infected id is owned by this rank, add to self queue
                if (infectedID % numranks == myrank) {
                    // TODO: check to see if node status = 0, 
                    // if it is add it to infectedQueue and print to parallel I/O that it was infected and by who
                } else { // else, send a message to owning rank
                    // TODO: send MPI message including infected ID and the ID of the node that infected it
                }
            }
        }

        // after checking for infections by the node, check to see if the node dies
        float chance = (double)rand()/RAND_MAX;
        if (chance < dieChance) {
            people[id].status = 2;
            // TODO: Remove from list of infected nodes, need to make that list dynamic memory for good speed
            deadCount += 1;
            infectedCount -= 1;
            // TODO: print to parallel I/O that node has died
        }
    }
    
    // night cycle
    // TODO: check for nodes in MPI message queue and add to infectedQueue if status = 0
    // when they get added to the infectedQueue from mpi message queue, print to parallel I/O that they have been infected and by who

    MPI_Finalize();
    
    
}