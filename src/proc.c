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

char waste[1000];

bool checkEntry(FILE *proc, char *check){
    char name[200] = {'\0'}; //Hopefully enough for HUION stuff
    //Read in name
    fscanf(proc,"%*s Name=\"%200[^\"]",(char*)&name);
    if(!strcmp(check,&name)){return true;}
    return false;
}

void nextEntry(FILE *proc){
    //Skip until next 'I' attribute
    waste[0] = '\0';
    while(waste[0] != 'I'){
        if(waste[0] == EOF){return;}
        fgets(&waste,1000,proc);
    }
}

char *getEntryHandler(FILE *proc){
    //Read until 'H'
    waste[0] = '\0';
    while(waste[0] != 'H'){
        if(waste[0] == EOF){return NULL;}
        fgets(&waste,1000,proc);
    }
    char *handlerName = malloc(sizeof(char)*100);
    handlerName[0] = '\0';
    sscanf(&waste,"H: Handlers=%s",handlerName);
    return handlerName;
}

//Returns device event name. I.E 'event15'
char *findDeviceEvent(char *name){
    FILE *proc = fopen("/proc/bus/input/devices","r");
    if(!proc){return NULL;}
    //Skip first bus entry
    nextEntry(proc);
    while(!checkEntry(proc,name)){
        nextEntry(proc);
        if(waste[0] == EOF){fclose(proc); return NULL;}
    }
    //Read in device handlers
    char *handler = getEntryHandler(proc);
    if(!handler){return NULL;}
    fclose(proc);
    return handler;
}