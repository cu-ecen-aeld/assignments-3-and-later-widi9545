#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


int main (int argc, char *argv[]){


openlog(NULL, 0, LOG_USER);

if(argc < 2){
    syslog(LOG_ERR, "We are missing arguments, please input a file path, and a string to write");
    return 1;
}

if(argc == 2){
    return 1;
}

printf("Here is the file name: %s\n", argv[1]);
printf("Here is the string: %s\n", argv[2]);
char* writefile = argv[1];
char* writestr = argv[2];


int fd = open(writefile, O_RDWR|O_CREAT, 0777);

int text = write(fd, writestr, strlen(writestr));

close(fd);
syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);

return 0;
}
