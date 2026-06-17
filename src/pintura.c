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
#include "pintura.h"

char basePath[12] = "/dev/input/\0";

int main(int argc, char **argv){
    printf("Getting linux events for HUION monitor\n");
    //Find handlers
    printf("Finding HUION device handlers\n");
    char *stripHandler = findDeviceEvent(HUION_STRIP);
    if(!stripHandler){printf("Could not find %s in proc\n",HUION_STRIP); return 0;}
    char *padHandler = findDeviceEvent(HUION_PAD);
    if(!padHandler){printf("Could not find %s in proc\n",HUION_PAD); free(stripHandler); return 0;}
    printf("%s -> %s\n",HUION_STRIP,stripHandler);
    printf("%s -> %s\n",HUION_PAD,padHandler);
    //Allocate paths
    char *stripPath = malloc(sizeof(char)*(strlen(stripHandler)+strlen(&basePath)+1));
    if(!stripPath){printf("Failed to malloc()\n"); free(stripHandler); free(padHandler); return 0;}
    char *padPath = malloc(sizeof(char)*(strlen(padHandler)+strlen(&basePath)+1));
    if(!padPath){printf("Failed to malloc()\n"); free(stripPath); free(stripHandler); free(padHandler); return 0;}
    //Combine strings
    sprintf(stripPath,"%s%s",&basePath,stripHandler);
    sprintf(padPath,"%s%s",&basePath,padHandler);
    free(padHandler);
    free(stripHandler);
    //Open touchstrip
    printf("Opening handlers\n");
    int stripfd = open(stripPath,O_RDONLY|O_NONBLOCK);
    free(stripPath);
    if(stripfd < 0){printf("Failed to open event for strip\n"); free(padPath); return 0;}
    //Open pads
    int padfd = open(padPath,O_RDONLY|O_NONBLOCK);
    free(padPath);
    if(padfd < 0){printf("Failed to open event for pads\n");}
    //Start events
    initEvents(padfd,stripfd);
    close(stripfd);
    close(padfd);
    return 0;
}