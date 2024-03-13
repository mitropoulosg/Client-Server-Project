#include "types_server.h"
int sock;
int flag=0; //for singal handler
int buffer_count = 0;
int buffer_out = 0;
int buffer_in = 0;
int num_party_votes = 0;
int num_voters=0;
pthread_mutex_t buffer_mtx,logfile_mtx,party_mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
Connection* buffer;
int bufferSize;
Party party_votes[1000];
Voter voters[1000];
int numWorkerthreads;
pthread_t *worker_threads;
FILE* log_file;
char* stats_filename;

int main(int argc, char* argv[]) {

    int client_sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    pthread_mutex_init(&buffer_mtx, 0);
    pthread_mutex_init(&logfile_mtx, 0);
    pthread_mutex_init(&party_mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);
	pthread_cond_init(&cond_nonfull, 0);

    if (argc != 6) {
        printf("Usage: %s [portnum] [numWorkerthreads] [bufferSize] [poll-log] [poll-stats]\n", argv[0]);
        return 1;
    }

    int portnum = atoi(argv[1]); //arguments
    numWorkerthreads = atoi(argv[2]);
    bufferSize = atoi(argv[3]);
    const char* log_filename=argv[4];
    stats_filename=argv[5];
    log_file = fopen(log_filename,"w");



    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket");

    // Set SO_REUSEADDR option
    int reuse = 1; //this is to avoid bind errors when reruning the program
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("Setsockopt failed");
        exit(1);
    }

    server.sin_family = AF_INET;       /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(portnum);      /* The given port */

    /* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror("bind");
    /* Listen for connections */
    if (listen(sock, 128) < 0) perror("listen");
   // printf("Listening for connections to port %d\n", portnum);


    // Create worker threads
    worker_threads = malloc(numWorkerthreads * sizeof(pthread_t));

    // Create connection buffer
    //stores connections
    buffer = malloc(bufferSize * sizeof(Connection));

    for (int i=0 ; i<numWorkerthreads ; i++)
        if (pthread_create(&worker_threads[i], NULL, worker_thread, NULL) != 0) {
            perror("Thread creation failed");
            close(sock);
            return 1;
        }

    // Register signal handler for SIGINT
    signal(SIGINT, signal_handler);

    while(1) {
        // Accept a new client connection
        client_sock = accept(sock, NULL, NULL);
        if (client_sock < 0) {
            perror("Failed to accept");
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&buffer_mtx);
	    while (buffer_count == bufferSize) {
		    pthread_cond_wait(&cond_nonfull, &buffer_mtx);
		}
        Connection connection; //create a connection
        connection.fd = client_sock;
        buffer[buffer_in] = connection;
        buffer_in = (buffer_in + 1) % bufferSize; //if reached end of buffer,start from 0
        buffer_count++;
        	    
        pthread_cond_signal(&cond_nonempty);
        pthread_mutex_unlock(&buffer_mtx);
    }

    return 0;
}
