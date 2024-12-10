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

#define SIGINT 2


// https://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c

// https://www.geeksforgeeks.org/signals-c-language/

// https://stackoverflow.com/questions/73550612/using-getopt-to-parse-command-line-options-and-their-arguments

// https://stackoverflow.com/questions/26138994/should-we-break-the-default-case-in-switch-statement
int socket_fd;
int connection;
char * buffer;

void handle_sigint(int sig){
    printf(" Caught signal %d, exiting gracefully\n", sig);
    close(connection);
    close(socket_fd);
    free(buffer);
    remove("/var/tmp/aesdsocket");

    exit(0);
    
}

void handle_sigterm(int sig){
    printf(" Caught signal %d, exiting gracefully\n", sig);
    close(connection);
    close(socket_fd);
    free(buffer);
    remove("/var/tmp/aesdsocket");

    exit(0);
    
}

int start_socket(){

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    int sockopt; 
    struct sockaddr_in address;  


    int opt = 1;
    ssize_t input;
    socklen_t addrlen = sizeof(address);
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
    while(1){

        connection = accept(socket_fd, (struct sockaddr *)&address, &addrlen);
        ssize_t size = recv(connection, buffer, 128000,  0);

        FILE *fp = fopen("/var/tmp/aesdsocket", "a");
        int write_to_file = fwrite(buffer, sizeof(char), size, fp);
        fclose(fp);
        char * client_ip_address = inet_ntoa(address.sin_addr);
        syslog(LOG_DEBUG, "Accepting connection from: %s", client_ip_address);


        if(strchr(buffer,'\n')){
            FILE *fr = fopen("/var/tmp/aesdsocket", "r");
            int bytes = 0;
            bytes = fread(buffer, sizeof(char), 128000, fr);
            fclose(fr);
            
            send(connection, buffer, bytes, 0 );
            syslog(LOG_DEBUG, "Closing connection from: %s", client_ip_address);
        }

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