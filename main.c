#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include "queue.h"
//#include "clockcycle.h"

#define population 8 // should be evenly divisible by numranks
#define connections 5
#define infectChance 0.5
#define dieChance 0.05
#define clock_frequency 512000000
#define startInfectedChance 0.3

struct Person
{
    long long int id;
    int status; // 0 = alive/not infected, 1 = infected, 2 = dead
    long long int friends[connections]; //stores IDs of connections
};

struct List {
    struct Node* head;
    struct Node* end;
    long long int size;
};

struct Node {
    long long int id;
    struct Node* next;
    struct Node* prev;
};

// adds node with id = val to end of list
struct Node* addToList(struct List *li, long long int val) {
    struct Node* n = malloc(sizeof(struct Node));
    n->id = val;
    n->next = NULL;
    if (li->size == 0) {
        n->prev = NULL;
        li->head = n;
    } else {
        n->prev = li->end;
    }

    li->end = n;
    li->size += 1;
    return n;
}

// removes node from list and frees memory
void deleteNode(struct List *li, struct Node* n) {
    if (n->next != NULL && n->prev != NULL) { // middle case (node is in middle of list)
        n->next->prev = n->prev;
        n->prev->next = n->next;
    } else { // end case (node is head or end of list)
        if (n->next != NULL) { // node is head of list
            n->next->prev = NULL;
            li->head = n->next;
        } else if (n->prev != NULL) { // node is end of list
            n->prev->next = NULL;
            li->end = n->prev;
        } else { // node is head and end of list (prev and next == NULL)
            li->head = NULL;
            li->end = NULL;
        }
    }
    n->next = NULL;
    n->prev = NULL;
    free(n);
    li->size -= 1;
}

int main(int argc, char *argv[]) {

    // mpi init
    struct Person people[population];
    MPI_Init(&argc, &argv);
    int myrank;
    int numranks;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &numranks);

    // initialize rank vars
    struct List infectedList = {NULL, NULL, 0}; // linked list for easy add/remove
    long long int deadCount = 0;
    struct Queue* infectedQueue = createQueue(population/numranks);


    // initialize structs
    srand(myrank);
    long long int id = myrank; // the id of the person we're creating 
    while (id < population) {
        // generate list of connections
        long long int friends[connections];
        for (int c = 0; c < connections; c++) {
            friends[c] = rand() % connections;
        }
        // determine if node starts infected
        float chance = (double)rand()/RAND_MAX;
        int isInfected = 0;
        if (chance < startInfectedChance) {
            addToList(&infectedList, id);
            isInfected = 1;
            printf("Rank %i: Node %lli has been infected at start\n", myrank, id); // TODO: remove this
        }
        // generate struct and copy in list of connections
        struct Person p = {id, isInfected}; 
        memcpy(p.friends, friends, connections * sizeof(long long int));
        people[id] = p;

        id+= numranks;
    }

    // day cycle
    int b = 0;
    while (b < 10) { // TODO: come up with a stopping point
        printf("Rank %i, day %i: %lli node(s) infected\n", myrank, b, infectedList.size);
        b += 1;
        struct Node* n = infectedList.head;
        while (n != NULL) {
            long long int id = n->id;

            // check for connections being infected
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
                
                printf("Rank %i: Node %lli has died\n", myrank, n->id); // TODO: remove this
                // TODO: print to parallel I/O that node has died

                struct Node* temp = n->prev;
                deleteNode(&infectedList, n);
                deadCount += 1;
                n = temp;
            }

            // increment node along linked list
            if (n != NULL) { // might be null if the only infected node is killed
                n = n->next;
            }
        }
        
        // night cycle
        // TODO: check for nodes in MPI message queue and add to infectedQueue if status = 0
        // when they get added to the infectedQueue from mpi message queue, print to parallel I/O that they have been infected and by who
    }
    

    MPI_Finalize();
    
    
}