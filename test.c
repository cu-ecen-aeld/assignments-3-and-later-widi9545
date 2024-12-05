#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>



int main(){
    pid_t pid = fork();
    if(pid == 0){
        printf("hello\n");
    }
    if(pid > 0){
    printf("you see this message once\n");
    wait(NULL);
    }
    printf("you see this twice\n");

    return 1;
    
}