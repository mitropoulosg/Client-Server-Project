#include "types_client.h"

extern char* server_name;
extern int port_num;

void insert_thread(Node** head, pthread_t thread) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->thread = thread;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

void free_threads(Node* head) {
    Node* current = head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
}

void* send_vote(void* arg) {
    Vote* vote = (Vote*)arg;

    struct hostent *hp;  

    // Create socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Failed to create socket");
        free(vote);
        return NULL;
    }

    // Set up server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_num);
    if ((hp = gethostbyname(server_name)) == NULL) {
        perror("gethostbyname"); exit(1);}
    memcpy(&server_addr.sin_addr, hp->h_addr_list[0], hp->h_length);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect to the server");
        free(vote);
        close(client_fd);
        return NULL;
    }

    ssize_t bytes;
    char buffer[MAX_BUFFER_SIZE]; //initialize buffer where the messages are stored

    // Receive "SEND NAME PLEASE" message
    bytes=recv(client_fd, buffer, sizeof(buffer), 0);
    buffer[bytes]='\0'; //at the end of the string to avoid false messages
    
    if (strcmp(buffer, "SEND NAME PLEASE") != 0) {
        printf("Unexpected message from server: %s\n", buffer);
        free(vote);
        close(client_fd);
        return NULL;
    }

    // Send voter name
    if (send(client_fd, vote->voter_name, strlen(vote->voter_name), 0) == -1) {
        perror("Failed to send voter name to server");
        free(vote);
        close(client_fd);
        return NULL;
    }

    // Receive response
    bytes=recv(client_fd, buffer, sizeof(buffer), 0);
    buffer[bytes]='\0';


    if (strcmp(buffer, "ALREADY VOTED") == 0) { //if voter has already voted
        printf("Voter %s has already voted\n", vote->voter_name);
    } else if (strcmp(buffer, "SEND VOTE PLEASE") == 0) {
        // Send party name
        if (send(client_fd, vote->party_name, strlen(vote->party_name), 0) == -1) {
            perror("Failed to send party name to server");
            free(vote);
            close(client_fd);
            return NULL;
        }

        // Receive response
        bytes=recv(client_fd, buffer, sizeof(buffer), 0);
        buffer[bytes]='\0';


        printf("%s\n", buffer);
    } else {
        printf("Unexpected message from servers: %s\n", buffer);
    }

    free(vote);
    close(client_fd);
    return NULL;
}