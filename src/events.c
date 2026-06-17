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
#include "pintura.h"

int evtfd = -1;

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

void initRequiredKeys(int padfd){
    ioctl(evtfd,UI_SET_EVBIT,EV_KEY);
    ADD_KEY(KEY_EQUAL);
    ADD_KEY(KEY_MINUS);
    if(padfd >= 0){
        /*############# YOU MUST ADD NEW KEYS HERE #############*/
        ADD_KEY(KEY_LEFTCTRL);
        ADD_KEY(KEY_Z);
        ADD_KEY(KEY_S);
        //Example: ADD_KEY(KEY_<UINPUT_KEYCODE>);
    }
}

/*############# END OF CUSTOM KEYS #############*/

bool touching = false;
int last = -1;

void stripEvent(evIn *event){
    if(event->type != 3){return;}
    if(event->value == HUION_STRIP_TOUCHED){touching = true; return;}
    if(event->value == HUION_STRIP_RELEASED){touching = false; last = -1; return;}
    //Determine direction
    int cmd;
    if(last != -1){
        cmd = (last > event->value) ? KEY_EQUAL: KEY_MINUS;
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
    switch(pad){
        case PAD_0:
        pad0Keys();
        break;
        case PAD_1:
        pad1Keys();
        break;
        case PAD_2:
        pad2Keys();
        break;
        case PAD_3:
        pad3Keys();
        break;
        default:
        return;
        break;
    }
    writeKey(EV_SYN,0,0);
}

int padfd = -1;
int stripfd = -1;

void signalHandler(int sig){
    if((sig == SIGINT) || (sig == SIGKILL) || (sig == SIGTERM)){
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
}

void initEvents(int pfd, int sfd){
    padfd = pfd;
    stripfd = sfd;
    //Catch signals
    signal(SIGKILL,signalHandler);
    signal(SIGINT,signalHandler);
    signal(SIGTERM,signalHandler);
    printf("Creating fake device\n");
    evtfd = open("/dev/uinput",O_WRONLY|O_NONBLOCK);
    if(evtfd < 0){printf("Failed to open uinput\n"); return;}
    initRequiredKeys(padfd);
    //Create fake device
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
        ssize_t sz = read(stripfd,&event,sizeof(event));
        if(sz > 0){
            stripEvent(&event);
        }
        sz = 0x0;
        //Check for pad events
        sz = read(padfd,&event,sizeof(event));
        if(sz > 0){
            padEvent(&event);
        }
    }
}