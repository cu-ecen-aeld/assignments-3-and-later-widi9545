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
#include <time.h>
#include "linked-list.h"
#if USE_AESD_CHAR_DEVICE == 1
    #define FILE_LOCATION ("/dev/aesdchar")
#else
    #define FILE_LOCATION ("/tmp/aesdsocket")
#endif

#define SIGINT 2


// https://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c

// https://www.geeksforgeeks.org/signals-c-language/

// https://stackoverflow.com/questions/73550612/using-getopt-to-parse-command-line-options-and-their-arguments

// https://stackoverflow.com/questions/26138994/should-we-break-the-default-case-in-switch-statement
// https://www.geeksforgeeks.org/stack-using-linked-list-in-c/

// https://hpc-tutorials.llnl.gov/posix/passing_args/

// https://stackoverflow.com/questions/13923885/execute-a-method-every-x-seconds-in-c

int socket_fd;
int connection;
struct sockaddr_in address; 
socklen_t addrlen = sizeof(address);
pthread_mutex_t lock;
int completionFlag = 0;


void handle_sigint(int sig){
    printf(" Caught signal %d, exiting gracefully\n", sig);
    close(connection);
    close(socket_fd);
    pthread_mutex_destroy(&lock);
    #if USE_AESD_CHAR_DEVICE == 0
        remove("/tmp/aesdsocket");
    #endif


    exit(0);
    
}

void handle_sigterm(int sig){
    printf(" Caught signal %d, exiting gracefully\n", sig);
    close(connection);
    close(socket_fd);
    pthread_mutex_destroy(&lock);
    #if USE_AESD_CHAR_DEVICE == 0
        remove("/tmp/aesdsocket");
    #endif

    exit(0);
    
}


void * timer(void *args){
    while(1){
        sleep(10);
        pthread_mutex_lock(&lock);
        
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char i[] = "timestamp:";
        char s[64];
        char n[] = "\n";
        size_t ret = strftime(s, sizeof(s), "%c", tm);
        char * newstr = malloc(strlen(i)+ strlen(s) + strlen(n) + 1);
        newstr[0] = '\0';
        strcat(newstr, i);
        strcat(newstr, s);
        strcat(newstr, n);
        printf("here is the string: %s", newstr);



        FILE *fp = fopen(FILE_LOCATION, "a+");

        if (fp) {
            fwrite(newstr, sizeof(char), strlen(newstr), fp);
            fclose(fp);
        } else {
            perror("File open failed!");
        }
        pthread_mutex_unlock(&lock);
        free(newstr);

    }
}


void *connection_processing(void *args) {
    struct t_data *connection_thread_data = (struct t_data *)args;
    
    int connection = connection_thread_data->connection;
    
    char *local_buffer = (char *)malloc(32000 * sizeof(char));
    if (!local_buffer) {
        perror("Memory allocation failed");
        pthread_exit(NULL);
    }

    ssize_t size = recv(connection, local_buffer, 32000, 0);
    
    pthread_mutex_lock(&lock);
    FILE *fp = fopen(FILE_LOCATION, "a+");
    if (fp) {
        fwrite(local_buffer, sizeof(char), size, fp);
        fclose(fp);
    } else {
        perror("File open failed!");
    }
    pthread_mutex_unlock(&lock);


    char client_ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &address.sin_addr, client_ip_address, INET_ADDRSTRLEN);
    syslog(LOG_DEBUG, "Accepting connection from: %s", client_ip_address);

    if (strchr(local_buffer, '\n')) {
        FILE *fr = fopen(FILE_LOCATION, "r");
        int bytes = 0;
        if (fr) {
            bytes = fread(local_buffer, sizeof(char), 32000, fr);
            fclose(fr);
        }
        send(connection, local_buffer, bytes, 0);
        syslog(LOG_DEBUG, "Closing connection from: %s", client_ip_address);
    }

    free(local_buffer);
    free(connection_thread_data); // Free dynamically allocated struct
    pthread_exit(NULL);
}


int start_socket(){

    int threads = 3;
    int i;
    int ret = -1;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    int sockopt; 
    int opt = 1;
    ssize_t input;


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


    #if USE_AESD_CHAR_DEVICE == 0
        pthread_t timer_thread; 
        pthread_create(&timer_thread, NULL, timer, NULL);
    #endif

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

        struct t_data *thread_data = malloc(sizeof(struct t_data));
        if (!thread_data) {
            perror("Malloc Failed");
            exit(1);
        }
        
        pthread_t thread;
        thread_data -> connection = connection;
        thread_data -> completion = complete;
        thread_data -> thread_id = thread;
        
        pthread_create(&thread_data -> thread_id, NULL, connection_processing, (void *)thread_data);
        pthread_join(thread_data -> thread_id, NULL);
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