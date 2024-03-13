#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>

#define MAX_PARTY_LENGTH 256
#define MAX_NAME_LENGTH 50

typedef struct { 
    int fd;
} Connection;

typedef struct {
    char name[MAX_NAME_LENGTH];
} Voter;

typedef struct {
    char party_name[MAX_NAME_LENGTH];
    int num_votes;
} Party;


void* leader_thread(void* arg);
void* worker_thread(void* arg);
void handle_client(int client_fd);
void print_close();
void signal_handler(int );
