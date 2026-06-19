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
#include <signal.h>
#include <errno.h>
#include "pintura.h"

static int evtfd = -1;
static tabletConfig *tablet = NULL;

typedef struct {
    struct timeval time;
    unsigned short type;
    unsigned short code;
    unsigned int value;
}evIn;

void writeKey(int type, int code, int val){
    struct input_event event;
    event.type = type;
    event.code = code;
    event.time.tv_sec = 0;
    event.time.tv_usec = 0;
    event.value = val;
    write(evtfd,&event,sizeof(event));
    usleep(100);
}

#define PRESS_KEY(key) writeKey(EV_KEY,key,1)
#define RELEASE_KEY(key) writeKey(EV_KEY,key,0)
#define ADD_KEY(key) ioctl(evtfd,UI_SET_KEYBIT,key);


bool k0pressed = false;
bool k1pressed = false;
bool k2pressed = false;
bool k3pressed = false;

/*############# WRITE CUSTOM KEYS BELOW #############*/

void pad0Keys(){
    if(!k0pressed){
        //Keys to press here, in order of press
        k0pressed = true;
    }
    else{
        //Keys to release
        k0pressed = false;
    }
}

void pad1Keys(){
    if(!k1pressed){
        //Keys to press here, in order of press
        k1pressed = true;
    }
    else{
        //Keys to release
        k1pressed = false;
    }
}

void pad2Keys(){
    if(!k2pressed){
        //Keys to press here, in order of press
        PRESS_KEY(KEY_LEFTCTRL);
        PRESS_KEY(KEY_S);
        k2pressed = true;
    }
    else{
        //Keys to release
        RELEASE_KEY(KEY_LEFTCTRL);
        RELEASE_KEY(KEY_S);
        k2pressed = false;
    }
}

void pad3Keys(){
    if(!k3pressed){
        //Keys to press here, in order of press
        PRESS_KEY(KEY_LEFTCTRL);
        PRESS_KEY(KEY_Z);
        k3pressed = true;
    }
    else{
        //Keys to release
        RELEASE_KEY(KEY_LEFTCTRL);
        RELEASE_KEY(KEY_Z);
        k3pressed = false;
    }
}

/*############# END OF CUSTOM KEYS #############*/

bool touching = false;
int last = -1;

void stripEvent(evIn *event){
    if(event->type != 3){return;}
    if(event->value == tablet->codes.stripPressed){touching = true; return;}
    if(event->value == tablet->codes.stripReleased){touching = false; last = -1; return;}
    //Determine direction
    int cmd;
    if(last != -1){
        cmd = (last > event->value) ? tablet->stripUp : tablet->stripDown;
    }
    else{
        last = event->value;
        return;
    }
    last = event->value;
    writeKey(EV_KEY,cmd,1);
    writeKey(EV_KEY,cmd,0);
    writeKey(EV_SYN,0,0);
}

void padEvent(evIn *event){
    unsigned short pad = event->code;
    int bindNum = 0;
    bool found = false;
    for(; bindNum < tablet->padNum; ++bindNum){
        if(tablet->padIDs[bindNum] == pad){found = true; break;}
    }
    if(!found){return;} //Pad is not documented
    //Press keybinds
    for(int i = 0; i < tablet->padBinds[bindNum].seqNum; ++i){
        if(tablet->padBinds[bindNum].keyVal == NULL){printf("NULL kv\n"); return;}
        if(tablet->padBinds[bindNum].keyVal[0] == 0){printf("Disabled pad\n"); return;}
        if(tablet->codes.padPressed == event->value){
            PRESS_KEY(tablet->padBinds[bindNum].keyVal[i]);
        }
        else{
            RELEASE_KEY(tablet->padBinds[bindNum].keyVal[i]);
        }
    }
    writeKey(EV_SYN,0,0);
}

static int padfd = -1;
static int stripfd = -1;

void signalHandler(int sig){
    printf("-Cleaning up-\n");
    close(padfd);
    close(stripfd);
    if(evtfd >= 0){
        ioctl(evtfd,UI_DEV_DESTROY);
        close(evtfd);
    }
    printf("Finished\n");
    exit(EXIT_SUCCESS);
}

void enableFakeKey(int key){
    ADD_KEY(key);
}

bool initFakeDevice(){ //Creates the fake device at the start to make my life easier
    //Catch signals
    signal(SIGKILL,signalHandler);
    signal(SIGINT,signalHandler);
    signal(SIGTERM,signalHandler);
    signal(SIGTSTP,signalHandler);
    evtfd = open("/dev/uinput",O_WRONLY|O_NONBLOCK);
    if(evtfd < 0){printf("Failed to open uinput\n"); return false;}
    ioctl(evtfd,UI_SET_EVBIT,EV_KEY);
    return true;
}

void initEvents(int pfd, int sfd, tabletConfig *cnf){
    tablet = cnf;
    padfd = pfd;
    stripfd = sfd;
    //Finalize the device
    printf("Creating fake device\n");
    struct uinput_setup dev;
    memset(&dev,0x0,sizeof(dev));
    char *nm = "Pintura";
    memcpy(&dev.name,nm,sizeof(char)*(strlen(nm)+1));
    dev.id.vendor = 0x0;
    dev.id.bustype = BUS_USB;
    dev.id.product = 0x0;
    dev.id.version = 0x1;
    ioctl(evtfd,UI_DEV_SETUP,&dev);
    ioctl(evtfd,UI_DEV_CREATE);
    sleep(1);
    //Start getting events
    printf("HUION driver is ready\n");
    while(true){
        //Check for strip events
        usleep(2);
        evIn event;
        ssize_t sz = read(sfd,&event,sizeof(event));
        if((sz == -1) && (errno != EAGAIN)){
            printf("Error reading stripfd %i\n",errno);
            signalHandler(SIGINT);
        }
        if(sz > 0){
            stripEvent(&event);
        }
        sz = 0x0;
        //Check for pad events
        sz = read(padfd,&event,sizeof(event));
        if((sz == -1) && (errno != EAGAIN)){
            printf("Error reading padfd %i\n",errno);
            signalHandler(SIGINT);
        }
        if(sz > 0){
            padEvent(&event);
        }
    }
}