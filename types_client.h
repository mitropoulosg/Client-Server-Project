#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>           /* internet sockets */
#include <netdb.h>    

#define MAX_NAME_LENGTH 256
#define MAX_BUFFER_SIZE 1024

typedef struct {
    char voter_name[MAX_NAME_LENGTH];
    char party_name[MAX_NAME_LENGTH];
} Vote;

typedef struct Node {
    pthread_t thread;
    struct Node* next;
} Node;

void* send_vote(void* arg);
void insert_thread(Node** head, pthread_t thread);
void free_threads(Node* head);
