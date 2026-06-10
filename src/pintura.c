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

typedef struct {
    struct timeval time;
    unsigned short type;
    unsigned short code;
    unsigned int value;
}evIn;

#define PAD_0 256 //Top pad
#define PAD_1 257
#define PAD_2 258
#define PAD_3 259 //Bottom pad

#define HUION_PAD "/dev/input/by-id/usb-HUION_Huion_Monitor-event-mouse" //Device path for the tablet pads
#define HUION_STRIP "/dev/input/by-id/usb-HUION_Huion_Monitor-event-joystick" //Device path for the tablet's touch strip

#define HUION_STRIP_TOUCHED 15
#define HUION_STRIP_RELEASED 0

#define HUION_STRIP_TOP 1
#define HUION_STRIP_BOTTOM 6

bool set = false;
bool touching = false;
int last = -1;
int evtfd = -1;

void end(){
    if(evtfd >= 0){
        ioctl(evtfd,UI_DEV_DESTROY);
        close(evtfd);
    }
    exit(EXIT_FAILURE);
}

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
        writeKey(EV_KEY,KEY_LEFTCTRL,1);
        writeKey(EV_KEY,KEY_S,1);
        k2pressed = true;
    }
    else{
        //Keys to release
        writeKey(EV_KEY,KEY_LEFTCTRL,0);
        writeKey(EV_KEY,KEY_S,0);
        k2pressed = false;
    }
}

void pad3Keys(){
    if(!k3pressed){
        //Keys to press here, in order of press
        writeKey(EV_KEY,KEY_LEFTCTRL,1);
        writeKey(EV_KEY,KEY_Z,1);
        k3pressed = true;
    }
    else{
        //Keys to release
        writeKey(EV_KEY,KEY_LEFTCTRL,0);
        writeKey(EV_KEY,KEY_Z,0);
        k3pressed = false;
    }
}

void initRequiredKeys(int padfd){
    ioctl(evtfd,UI_SET_EVBIT,EV_KEY);
    ioctl(evtfd,UI_SET_KEYBIT,KEY_EQUAL);
    ioctl(evtfd,UI_SET_KEYBIT,KEY_MINUS);
    if(padfd >= 0){
        /*############# YOU MUST ADD NEW KEYS HERE #############*/
        ioctl(evtfd,UI_SET_KEYBIT,KEY_LEFTCTRL);
        ioctl(evtfd,UI_SET_KEYBIT,KEY_Z);
        ioctl(evtfd,UI_SET_KEYBIT,KEY_S);
        //Example: ioctl(evtfd, UI_SET_KEYBIT, KEY_<UINPUT_KEYCODE>);
    }
}

/*############# END OF CUSTOM KEYS #############*/

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

int main(int argc, char **argv){
    printf("Getting linux events for HUION monitor\n");
    //Open huion input
    int stripfd = open(HUION_STRIP,O_RDONLY|O_NONBLOCK);
    if(stripfd < 0){printf("Failed to open event for strip\n"); end();}
    printf("Opened strip\n");
    int padfd = open(HUION_PAD,O_RDONLY|O_NONBLOCK);
    if(padfd < 0){printf("Failed to open event for pads\n");}
    else{printf("Opened pad\n");}
    //Startup uinput
    evtfd = open("/dev/uinput",O_WRONLY|O_NONBLOCK);
    if(evtfd < 0){printf("Failed to open uinput\n"); end();}
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
    //Start getting stuff
    printf("HUION driver is ready\n");
    while(true){
        //Do strip
        usleep(2);
        evIn event;
        ssize_t sz = read(stripfd,&event,sizeof(event));
        if(sz > 0){
            stripEvent(&event);
        }
        //Do pads
        sz = read(padfd,&event,sizeof(event));
        if(sz > 0){
            padEvent(&event);
        }
    }
    close(stripfd);
    if(padfd >= 0 ){close(padfd);}
    return 0;
}