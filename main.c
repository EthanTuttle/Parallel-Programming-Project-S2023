#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include "clockcycle.h"

#define population 20000000 // should be evenly divisible by numranks
#define connections 5
#define infectChance 0.3
#define dieChance 0.2
#define clock_frequency 512000000
#define startInfectedChance 0.1

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
        li->end->next = n;
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

void sim(int myrank, int numranks) {
    // initialize rank vars
    struct Person *people = calloc(population, sizeof(struct Person));
    struct List infectedList = {NULL, NULL, 0}; // linked list for easy add/remove
    long long int deadCount = 0;
    struct List infectedQueue = {NULL, NULL, 0};
    MPI_File file;
    MPI_File_open(MPI_COMM_WORLD, "output.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY,
            MPI_INFO_NULL, &file);
    
    
    // initialize structs
    srand(myrank);
    long long int id = myrank; // the id of the person we're creating
    while (id < population) {
        // generate list of connections
        long long int friends[connections];
        for (int c = 0; c < connections; c++) {
            friends[c] = rand() % population; // rand() % x = random number from 0 to x-1 ( e.g. [0-X) )
        }
        // determine if node starts infected
        float chance = (double)rand()/RAND_MAX;
        int isInfected = 0;
        if (chance < startInfectedChance) {
            addToList(&infectedList, id);
            isInfected = 1;
            //printf("Rank %i: Node %lli has been infected at start\n", myrank, id); // Debug print
        }
        // generate struct and copy in list of connections
        struct Person p = {id, isInfected};
        memcpy(p.friends, friends, connections * sizeof(long long int));
        people[id] = p;

        id+= numranks;
    }
   
    // day cycle
    int b = 0;
    while (true) {
        // Beginning of day information
        b += 1;
        long long int dayData[3] = {(population/numranks)-infectedList.size-deadCount, infectedList.size, deadCount};
        long long int dayTotals[3];
        MPI_Allreduce(&dayData, &dayTotals, 3, MPI_LONG_LONG_INT, MPI_SUM, MPI_COMM_WORLD);

        // Day summaries
        if (dayTotals[0] == 0 || dayTotals[1] == 0) { // sim ends when either no infected or non-infected people remain

            // Print final individual rank day summary
            char buffer[150]; //buffer needed for io
            memset(buffer, '\0', sizeof(buffer)); 
            sprintf(buffer, "End of Sim Day %i Rank %i: %lli node(s) not infected, %lli node(s) infected, %lli node(s) dead\n", b, myrank, dayData[0], dayData[1], dayData[2]);
            int count = strlen(buffer); // exclude null terminator
            // Determine the size of the file
            MPI_Offset file_size;
            MPI_File_get_size(file, &file_size);

            // Set the offset to the end of the file
            MPI_Offset offset = file_size + (count * myrank);
            MPI_File_write_at_all(file, offset, buffer, count, MPI_CHAR, MPI_STATUS_IGNORE);
            MPI_File_sync(file);

            // Print final total day summary
            if (myrank == 0) {
                char buffer0[150]; //buffer needed for io
                memset(buffer0, '\0', sizeof(buffer)); 
                sprintf(buffer0, "End of Sim Day %i Totals: %lli node(s) not infected, %lli node(s) infected, %lli node(s) dead\n", b, dayTotals[0], dayTotals[1], dayTotals[2]);
                int count = strlen(buffer0); // exclude null terminator
                // Determine the size of the file
                MPI_Offset file_size;
                MPI_File_get_size(file, &file_size);

                // Set the offset to the end of the file
                MPI_Offset offset = file_size;
                MPI_File_write_at(file, offset, buffer0, count, MPI_CHAR, MPI_STATUS_IGNORE);
            }
            break;
        } else {

            // Print individual rank day summary
            char buffer[150]; //buffer needed for io
            memset(buffer, '\0', sizeof(buffer)); 
            sprintf(buffer, "Day %i Rank %i: %lli node(s) not infected, %lli node(s) infected, %lli node(s) dead\n", b, myrank, dayData[0], dayData[1], dayData[2]);
            int count = strlen(buffer); // exclude null terminator
            // Determine the size of the file
            MPI_Offset file_size;
            MPI_File_get_size(file, &file_size);

            // Set the offset to the end of the file
            MPI_Offset offset = file_size + (count * myrank);
            MPI_File_write_at_all(file, offset, buffer, count, MPI_CHAR, MPI_STATUS_IGNORE);
            MPI_File_sync(file);

            // Print total day summary
            if (myrank == 0) {
                char buffer0[150]; //buffer needed for io
                memset(buffer0, '\0', sizeof(buffer)); 
                sprintf(buffer0, "Day %i Totals: %lli node(s) not infected, %lli node(s) infected, %lli node(s) dead\n", b, dayTotals[0], dayTotals[1], dayTotals[2]);
                int count = strlen(buffer0); // exclude null terminator
                // Determine the size of the file
                MPI_Offset file_size;
                MPI_File_get_size(file, &file_size);

                // Set the offset to the end of the file
                MPI_Offset offset = file_size;
                MPI_File_write_at(file, offset, buffer0, count, MPI_CHAR, MPI_STATUS_IGNORE);
            }
        }

        // loop through infected nodes, find new infections and deaths
        struct Node* n = infectedList.head;
        long long int messageCount[numranks]; // index = rank, count[index] = number of messages to that rank
        memset( messageCount, 0, numranks*sizeof(long long int) );
        while (n != NULL) { 
            long long int id = n->id;

            // check for connections being infected
            for (int c = 0; c < connections; c++) {
                float chance = (double)rand()/RAND_MAX;
                if (chance < infectChance) {
                    long long int infectedID = people[id].friends[c];
                    // if infected id is owned by this rank, add to self queue
                    if (infectedID % numranks == myrank) {
                        if (people[infectedID].status == 0) {
                            addToList(&infectedQueue, infectedID);
                            people[infectedID].status = 1;
                        }
                    } else { // else, increment corresponding message count and send message
                        messageCount[infectedID % numranks] += 1;
                        MPI_Request request;
                        MPI_Isend(&infectedID, 1, MPI_LONG_LONG_INT, infectedID % numranks, 0, MPI_COMM_WORLD, &request);
                    }
                }
            }

            // after checking for infections by the node, check to see if the node dies
            bool dead = false;
            float chance = (double)rand()/RAND_MAX;
           
            if (chance < dieChance) {
                people[id].status = 2;          
                struct Node* temp = n->next;
                deleteNode(&infectedList, n);
                deadCount += 1;
                n = temp;
                dead = true;
                //printf("Rank %i: Node %lli has died\n", myrank, id); // Debug printing
            }

            // increment node along linked list
            // don't increment if node died (already incremented with temp variable)
            if (!dead) {
                n = n->next;
            }
        }
        
        // night cleanup

        // Reduce messageCount across all ranks to get total message count for this rank (totalMessageCount[myrank])
        long long int totalMessageCount[numranks];
        MPI_Allreduce(&messageCount, &totalMessageCount, numranks, MPI_LONG_LONG_INT, MPI_SUM,  MPI_COMM_WORLD);

        // Receive all messages and add nodes not infected already to the infectedQueue
        for (long long int i = 0; i < totalMessageCount[myrank]; i++) {
            long long int recv;
            MPI_Recv(&recv, 1, MPI_LONG_LONG_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (people[recv].status == 0) { // if node is already in self infectedQueue then status = 1, so there will be no repeats
                people[recv].status = 1;
                addToList(&infectedQueue, recv);
                
            }
        }
        
        // Loop through queue and add nodes to infectedList
        n = infectedQueue.head;
        while(n != NULL) {
            long long int id = n->id;
            //printf("Rank %i: Node %lli has been infected\n", myrank, id); // Debug printing
            addToList(&infectedList, id);
            deleteNode(&infectedQueue, n);
            n = infectedQueue.head;
        }
    }
    MPI_File_close(&file);

}

int main(int argc, char *argv[]) {

    // mpi init
    MPI_Init(&argc, &argv);
    int myrank;
    int numranks;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &numranks);            
    double start_cycles= clock_now();
    sim(myrank, numranks);
    double end_cycles= clock_now();
    double time_in_secs = ((double)(end_cycles - start_cycles)) / clock_frequency;
    if (myrank == 0) {
        printf("CPU Reduce time: %f\n", time_in_secs);
    }
    MPI_Finalize();
    return 0;
}
