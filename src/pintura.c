#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <linux/uinput.h>
#include <errno.h>
#include "pintura.h"

char basePath[12] = "/dev/input/\0";

int openTabletfd(char *handler){
    char *path = (char*)malloc(sizeof(char)*(strlen(handler)+strlen((char*)&basePath)+1));
    if(!path){printf("malloc() failed\n"); return -1;}
    sprintf(path,"%s%s",(char*)&basePath,handler);
    int fd = open(path,O_RDONLY|O_NONBLOCK);
    if(fd < 0){
        printf("Failed to open '%s'\n",path);
        free(path);
        if(errno == EACCES){printf("Pintura has no access to the file\n"); return -1;}
        printf("Errno: %i\n",errno);
        return -1;
    }
    free(path);
    return fd;
}

int main(int argc, char **argv){
    if(!initFakeDevice()){
        printf("Failed to create fake device\n");
        return -1;
    }
    tabletConfig *tablet = loadConfig();
    //Find handlers
    printf("Finding HUION device handlers\n");
    char *stripHandler = findDeviceEvent(tablet->stripEvent);
    if(!stripHandler){printf("Could not find %s in proc\n",tablet->stripEvent); return 0;}
    char *padHandler = findDeviceEvent(tablet->padEvent);
    if(!padHandler){printf("Could not find %s in proc\n",tablet->padEvent); free(stripHandler); return 0;}
    //Open files
    printf("Opening handlers\n");
    int stripfd = openTabletfd(stripHandler);
    free(stripHandler);
    if(stripfd == -1){free(padHandler); return -1;}
    int padfd = openTabletfd(padHandler);
    free(padHandler);
    if(padfd == -1){return -1;}
    //Start events
    initEvents(padfd,stripfd,tablet);
    close(stripfd);
    close(padfd);
    return 0;
}