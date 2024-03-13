#include "types_client.h"

char* server_name; //arguments
int port_num;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s [serverName] [portNum] [inputFile.txt]\n", argv[0]);
        return 1;
    }

    server_name = argv[1];
    port_num = atoi(argv[2]);
    const char* input_file = argv[3];

    Node* head = NULL;

    FILE* file = fopen(input_file, "r");
    if (!file) {
        perror("Failed to open input file");
        return 1;
    }

    char line[MAX_BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) { //for every line in file
        // Extract voter name and party name from the line
        char *word[3];
        int i=0;
        word[0] = strtok(line, " \n");  // Split line into words using space as a delimiter
        if(word[0]==NULL)
            continue;

        while (word[i] != NULL) {
            i++;
            word[i] = strtok(NULL, " \n");  // Get next word
        }

        char names[100];
        sprintf(names, "%s %s", word[0], word[1]); //first 2 words is the full name

        // Create a vote object
        Vote* vote = (Vote*)malloc(sizeof(Vote));
        strncpy(vote->voter_name, names, MAX_NAME_LENGTH);
        strncpy(vote->party_name, word[2], MAX_NAME_LENGTH);

        // Create a thread to send the vote
        pthread_t thread;
        if (pthread_create(&thread, NULL, send_vote, vote) != 0) {
            perror("Failed to create thread");
            free(vote);
            continue;
        }

        insert_thread(&head, thread);
    }

    // Join all the threads
    Node* current = head;
    while (current != NULL) {
        pthread_join(current->thread, NULL);
        current = current->next;
    }

    // Free memory allocated for threads
    free_threads(head);

    fclose(file);
    return 0;
}