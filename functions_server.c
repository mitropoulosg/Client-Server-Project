#include "types_server.h"
extern int sock;
extern int flag;
extern int buffer_count;
extern int buffer_out;
extern int num_party_votes;
extern int num_voters;
extern pthread_mutex_t buffer_mtx,logfile_mtx,party_mtx;
extern pthread_cond_t cond_nonempty;
extern pthread_cond_t cond_nonfull;
extern char* stats_filename;
extern FILE* log_file;
extern Connection* buffer;
extern int bufferSize;
extern Party party_votes[1000];
extern Voter voters[1000];
extern int numWorkerthreads;
extern pthread_t *worker_threads;


void signal_handler(int sig) {
    if (sig == SIGINT) { //if types ctrl-c then 
        flag=1;
        pthread_cond_broadcast(&cond_nonempty); //waked up all threads waiting to start finishing
        print_close(); //create stats file and free all variables 
        exit(0);
    }
}

void print_close() {
    // Wait for all threads to finish
    for (int i = 0; i < numWorkerthreads; i++) {
        pthread_join(worker_threads[i], NULL);
    }
    free(worker_threads); //free
    free(buffer);
    close(sock); //close server socket

    pthread_cond_destroy(&cond_nonempty);
	pthread_cond_destroy(&cond_nonfull);
    pthread_mutex_destroy(&buffer_mtx);
    pthread_mutex_destroy(&logfile_mtx);
    pthread_mutex_destroy(&party_mtx);

    // Write poll statistics to file
    int count=0;
    FILE* stats_file = fopen(stats_filename,"w");//open stats file for writing 
    for (int i = 0; i < num_party_votes; i++) { //for every party 
        fprintf(stats_file, "%s %d\n", party_votes[i].party_name, party_votes[i].num_votes);//print party name and its votes
        count = count + party_votes[i].num_votes;
    }
    fprintf(stats_file,"TOTAL %d\n",count);
    fclose(stats_file);
    fclose(log_file);
}

void handle_client(int client_fd)
{
    ssize_t bytes;
    char recvBuffer[256]; //buffer to store messages from client

    // Send "SEND NAME PLEASE" message
    if (send(client_fd, "SEND NAME PLEASE", strlen("SEND NAME PLEASE"), 0) == -1) {
        perror("Failed to send message to client");
        return;
    }

    // Receive voter name
    bytes=recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
    if (bytes == -1) {
        perror("Failed to receive voter name from client");
        return;
    }

    recvBuffer[bytes]='\0';

    char voter_name[MAX_NAME_LENGTH];
    strncpy(voter_name, recvBuffer, sizeof(voter_name));

    pthread_mutex_lock(&party_mtx);

    // Check if voter has already voted
    int already_voted = 0;
    for (int i = 0; i < num_voters; i++) {
        if (strcmp(voters[i].name, voter_name) == 0) {
            already_voted = 1;
        }
    }
    pthread_mutex_unlock(&party_mtx);

    

    // Send response to client
    if (already_voted) 
    {
            if (send(client_fd, "ALREADY VOTED", strlen("ALREADY VOTED"), 0) == -1) {
            perror("Failed to send message to client");
            return;
        }
    }
    else
    {
        if (send(client_fd, "SEND VOTE PLEASE", strlen("SEND VOTE PLEASE"), 0) == -1) {
            perror("Failed to send message to client");
            return;
        }

        // Receive vote
        bytes=recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
        if (bytes== -1) {
            perror("Failed to receive vote from client");
            return;
        }
        recvBuffer[bytes]='\0';

        char party_voted_for[MAX_NAME_LENGTH];
        strncpy(party_voted_for, recvBuffer, sizeof(party_voted_for));

        // Send response to client
        char vote_recorded_msg[300];
        sprintf(vote_recorded_msg, "VOTE for Party %s RECORDED", party_voted_for);
        

        if (send(client_fd, vote_recorded_msg, strlen(vote_recorded_msg), 0) == -1) {
            perror("Failed to send message to client");
            return;
        }

        
        pthread_mutex_lock(&logfile_mtx);
        //New voter
        Voter new_voter;
        strcpy(new_voter.name, voter_name);
        voters[num_voters] = new_voter;
        num_voters++;
        // Update poll log file
        fprintf(log_file, "%s %s\n",voter_name,party_voted_for);
        fflush(log_file);
        pthread_mutex_unlock(&logfile_mtx);
        

        // Update party votes
        pthread_mutex_lock(&party_mtx);
        int party_found = 0;
        for (int i = 0; i < num_party_votes; i++) {
            if (strcmp(party_votes[i].party_name, party_voted_for) == 0) {
                party_votes[i].num_votes++;
                party_found = 1;
                break;
            }
        }
        if (!party_found) { //create a new party
            Party new_party_vote;
            strcpy(new_party_vote.party_name, party_voted_for);
            new_party_vote.num_votes = 1;
            party_votes[num_party_votes] = new_party_vote;
            num_party_votes++;
        }
        pthread_mutex_unlock(&party_mtx);
          
    }
}


void* worker_thread(void* arg) {
//consumer
    while(1){
    pthread_mutex_lock(&buffer_mtx);
	    while (buffer_count == 0 && !flag) {
		    pthread_cond_wait(&cond_nonempty, &buffer_mtx);
		}
        if(flag) //if we have a ctrl-c signal
        {
            pthread_mutex_unlock(&buffer_mtx); //unlock the mutex 
            return(void*)0;
        }

        Connection connection = buffer[buffer_out]; //take a connection from the buffer
        buffer_out = (buffer_out + 1) % bufferSize;
        buffer_count--;

        pthread_cond_signal(&cond_nonfull);
        pthread_mutex_unlock(&buffer_mtx);

        handle_client(connection.fd); //handle the client (server-client communication)
        close(connection.fd);
    }
    return(void*)0;
}