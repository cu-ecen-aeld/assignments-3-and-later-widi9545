#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include "linked-list.h"

#define SIGINT 2


// https://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c

// https://www.geeksforgeeks.org/signals-c-language/

// https://stackoverflow.com/questions/73550612/using-getopt-to-parse-command-line-options-and-their-arguments

// https://stackoverflow.com/questions/26138994/should-we-break-the-default-case-in-switch-statement
// https://www.geeksforgeeks.org/stack-using-linked-list-in-c/

// https://hpc-tutorials.llnl.gov/posix/passing_args/

int socket_fd;
int connection;
char * buffer;
struct sockaddr_in address; 
socklen_t addrlen = sizeof(address);
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int completionFlag = 0;
FILE *fp;
node* list = NULL;

void handle_sigint(int sig){
    printf(" Caught signal %d, exiting gracefully\n", sig);
    close(connection);
    close(socket_fd);
    free(buffer);
    remove("/tmp/aesdsocket");

    exit(0);
    
}

void handle_sigterm(int sig){
    printf(" Caught signal %d, exiting gracefully\n", sig);
    close(connection);
    close(socket_fd);
    free(buffer);
    remove("/tmp/aesdsocket");

    exit(0);
    
}


void * connection_processing(void *args){

    struct t_data *connection_thread_data = args;
    
    int connection = connection_thread_data -> connection;
    int completion = connection_thread_data -> completion;

    buffer = (char *)malloc(32000 * sizeof(char));
    ssize_t size = recv(connection, buffer, 128000,  0);
    pthread_mutex_lock(&lock);
    fp = fopen("/tmp/aesdsocket", "a+");
    int write_to_file = fwrite(buffer, sizeof(char), size, fp);
    fclose(fp);
    pthread_mutex_unlock(&lock);
    char * client_ip_address = inet_ntoa(address.sin_addr);
    syslog(LOG_DEBUG, "Accepting connection from: %s", client_ip_address);


    if(strchr(buffer,'\n')){
        FILE *fr = fopen("/tmp/aesdsocket", "r");
        int bytes = 0;
        bytes = fread(buffer, sizeof(char), 128000, fr);
        fclose(fr);
        send(connection, buffer, bytes, 0 );
        syslog(LOG_DEBUG, "Closing connection from: %s", client_ip_address);
        push(&list, connection_thread_data -> thread_id);
        
    }
    
    
}


int start_socket(){

    int threads = 3;
    int i;
    int ret = -1;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    int sockopt; 
    int opt = 1;
    ssize_t input;
    buffer = (char *)malloc(32000 * sizeof(char));



    if(socket_fd < 0){
        perror("The socket failed");
        exit(-1);
    }

    sockopt = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    if(sockopt == -1){
        perror("Setting the socket options had an error");
        exit(-1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9000);

    int socket_bind = bind(socket_fd, (struct sockaddr*)&address, sizeof(address));
    if(socket_bind < 0){
        perror("There was a binding issues");
        exit(-1);
    }


    int listen_socket = listen(socket_fd, 3);
    if(listen_socket < 0){
        perror("there was an error listening on the socket");
        exit(-1);
    }

    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigterm);

    if( pthread_mutex_init(&lock, NULL) != 0){
        perror("There was an error initiating the mutex\n");
        exit(-1);
    }

    while(completionFlag != 1){
        int complete = 0;
        /*1) create list
        2) create structure for thread data - x
        3) add structure for thread data to list - x
        4) get that into arguments for pthread_create variable - x
        5) process
        6) add data to thread data structure that was passed
        7) once the processing is done, return 
        8) go down list, get structures
        9) get the list of thread IDs
        10) join threads to free memory
        11) iterate down list and free each thread individually?
        */
        connection = accept(socket_fd, (struct sockaddr *)&address, &addrlen);

        if(connection < 0){
            perror("There was an acceptance issue");
            exit(-1);
        }

        struct t_data thread_data;
        pthread_t thread;
        thread_data.connection = connection;
        thread_data.completion = complete;
        thread_data.thread_id = thread;
        
        pthread_create(&thread_data.thread_id, NULL, connection_processing, (void *)&thread_data);
    }

    return 0; 
}

int main(int argc, char * argv[]){
    int child_process = 0;
    int handler = 0;

    while((handler = getopt(argc, argv, "d")) != -1){
        switch(handler){
            case 'd':
                child_process = 1;
                break;
            default:
                child_process = 0;
        }

    }

    if(child_process == 0){
    start_socket();
    }
    if(child_process == 1){
        pid_t pid = fork();
        if(pid < 0 ){
            perror("fork error");
            exit(1);
        }
        if(pid == 0){
            signal(SIGINT, handle_sigint);
            signal(SIGTERM, handle_sigterm);
            while(1){
                start_socket();
            }
        }
    
    }

    return 0;
}