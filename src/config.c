#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "pintura.h"

char *loadIn = NULL;
char *optName = NULL;
FILE *conf = NULL;
static tabletConfig *tablet = NULL;

char *cloneString(char *ostr){
    char *str = (char*)malloc(sizeof(char)*(strlen(ostr)+1));
    if(!str){printf("malloc() failed\n"); return NULL;}
    strcpy(str,ostr);
    return str;
}

char *loadConfString(){
    sscanf(loadIn,"%*[^=]=\"%[^\";]",optName);
    return cloneString(optName);
}

int loadConfInt(){
    int num = 0;
    sscanf(loadIn,"%*[^=]=%i[^;]",&num);
    return num;
}

char *loadArray(){
    sscanf(loadIn,"%*[^=]=(%[^)]",optName);
    return cloneString(optName);
}

int enumerateItems(char *array, char delim){
    uint32_t n = 1; //2 commas = 3 items, so add one premptively
    for(int i = 0; array[i] != '\0'; ++i){
        if(array[i] == delim){++n;}
    }
    return n;
}

bool nextArrayInt(char *array, bool start, int *set, char *delim){
    char *ptr = (start == true) ? array : NULL;
    char *ret = strtok(ptr,delim); //Change to strsep perhaps
    if(!ret){return false;}
    *set = atoi(ret);
    return true;
}

char *nextSubArray(char *array, bool start, char *delim){ //For loading keybinds (Must be free'd after use)
    char *ptr = (start == true) ? array : NULL;
    char *ret = strtok(ptr,delim); //Change to strsep perhaps
    if(!ret){return NULL;}
    return cloneString(ret);
}

void parseKeybinds(int padN, char *array){
    int cursor = 0;
    int start = 0;
    int key = 0;
    while(true){
        ++cursor;
        if((array[cursor] == '+') || (array[cursor] == '\0')){
            bool exitlp = (array[cursor] == '\0') ? true : false;
            array[cursor] = '\0';
            tablet->padBinds[padN].keyVal[key] = atoi(&array[start]);
            enableFakeKey(tablet->padBinds[padN].keyVal[key]);
            //printf(" %i ",tablet->padBinds[padN].keyVal[key]);
            if(exitlp){return;}
            start = cursor;
            array[cursor] = '+';
            ++cursor;
            ++key;
        }
    }
}

void loadPadKeybinds(){
    char *array = loadArray();
    int binds = enumerateItems(array,',');
    if(binds > tablet->padNum){printf("More binds exist than padIDs\n");} //Extra will be discarded
    bool start = true;
    for(int i = 0; i < tablet->padNum; ++i){
        char *sub = nextSubArray(array,start,",");
        start = false;
        if(!sub){printf("Error loading next subarray\n"); return;}
        tablet->padBinds[i].seqNum = enumerateItems(sub,'+');
        tablet->padBinds[i].keyVal = (int*)malloc(sizeof(int)*tablet->padBinds[i].seqNum);
        parseKeybinds(i,sub);
        free(sub);
    }
}

void parseLine(uint32_t line){
    //Get first half. I.E 'optName'
    sscanf(loadIn,"%[^= ]",optName);
    if(optName[0] == '\0'){printf("No valid option on line %u\n",line); return;}
    //printf("Parsed option %s\n",optName);
    //Determine option (note: I HATE THAT I MUST USE STRCMP LIKE THIS)
    //We can overwrite `optName` once strcmp'd
    if(!strcmp(optName,"TabletName")){
        tablet->tabletName = loadConfString();
        printf("Config made for '%s'\n",tablet->tabletName);
        goto end;
    }
    if(!strcmp(optName,"StripName")){
        tablet->stripEvent = loadConfString();
        goto end;
    }
    if(!strcmp(optName,"PadName")){
        tablet->padEvent = loadConfString();
        goto end;
    }
    if(!strcmp(optName,"PenName")){
        tablet->penEvent = loadConfString();
        goto end;
    }
    if(!strcmp(optName,"PadKeybinds")){
        loadPadKeybinds();
        goto end;
    }
    if(!strcmp(optName,"StripUp")){
        tablet->stripUp = loadConfInt();
        enableFakeKey(tablet->stripUp);
        goto end;
    }
    if(!strcmp(optName,"StripDown")){
        tablet->stripDown = loadConfInt();
        enableFakeKey(tablet->stripDown);
        goto end;
    }
    if(!strcmp(optName,"StripTouched")){
        tablet->codes.stripPressed = loadConfInt();
        goto end;
    }
    if(!strcmp(optName,"StripReleased")){
        tablet->codes.stripReleased = loadConfInt();
        goto end;
    }
    if(!strcmp(optName,"StripTop")){
        tablet->codes.stripTop = loadConfInt();
        goto end;
    }
    if(!strcmp(optName,"StripBottom")){
        tablet->codes.stripBottom = loadConfInt();
        goto end;
    }
    if(!strcmp(optName,"PadNum")){
        tablet->padNum = loadConfInt();
        tablet->padBinds = (struct padKeybind*)calloc(tablet->padNum,sizeof(struct padKeybind));
        goto end;
    }
    if(!strcmp(optName,"PadIDs")){
        char *array = loadArray();
        if(!array){printf("Failed to load array on line %u\n",line); goto end;}
        tablet->padIDs = (uint32_t*)malloc(sizeof(uint32_t)*tablet->padNum);
        for(uint32_t i = 0; i < tablet->padNum; ++i){
            bool start = false;
            if(i == 0){start = true;}
            if(!nextArrayInt(array,start,(int*)&tablet->padIDs[i],",)")){printf("strtok ended unexpectedly\n"); break;}
        }
        goto end;
    }
    if(!strcmp(optName,"PadPressed")){
        tablet->codes.padPressed = loadConfInt();
        goto end;
    }
    if(!strcmp(optName,"PadReleased")){
        tablet->codes.padReleased = loadConfInt();
        goto end;
    }
    end:
}

void cnfEnd(){
    if(loadIn){free(loadIn);}
    if(optName){free(optName);}
    if(conf){fclose(conf);}
}

tabletConfig *loadConfig(){
    conf = fopen("./pintura.conf","r");
    if(!conf){printf("Unable to open config file\n"); return NULL;}
    //Whole lot of malloc
    tablet = (tabletConfig*)malloc(sizeof(tabletConfig));
    if(!tablet){cnfEnd(); return NULL;}
    loadIn = (char*)malloc(sizeof(char)*500); //Enough for this probably.
    if(!loadIn){cnfEnd(); return NULL;}
    memset(loadIn,'\0',500);
    optName = (char*)malloc(sizeof(char)*100);
    if(!optName){cnfEnd(); return NULL;}
    optName[0] = '\0';
    //Start reading lines until EOF
    printf("Parsing config\n");
    for(uint32_t line = 0; loadIn[0] != EOF; ++line){
       if(!fgets(loadIn,500,conf)){break;} //File either error'd or EOF
        if(loadIn[0] == '#'){continue;} //Skip comments
        parseLine(line);
    }
    //Cleanup
    cnfEnd();
    printf("Loaded config\n");
    return tablet;
}

void destroyConfig(){
    free(tablet->tabletName);
    free(tablet->stripEvent);
    free(tablet->padEvent);
    free(tablet->penEvent);
    free(tablet->padIDs);
    for(uint32_t i = 0; i < tablet->padNum; ++i){
        free(tablet->padBinds[i].keyVal);
    }
    free(tablet->padBinds);
    free(tablet);
}