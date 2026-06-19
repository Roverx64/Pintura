#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PAD_0 256 //Top pad
#define PAD_1 257
#define PAD_2 258
#define PAD_3 259 //Bottom pad

//Device names in /proc/bus/input/devices
#define HUION_PAD "HUION Huion Monitor Pad"
#define HUION_STRIP "HUION Huion Monitor Touch Strip" 

#define HUION_STRIP_TOUCHED 15
#define HUION_STRIP_RELEASED 0

#define HUION_STRIP_TOP 1
#define HUION_STRIP_BOTTOM 6

struct padKeybind{
    int seqNum;
    int *keyVal; //NULL when no binds are made
};

struct tabletEventCodes{
    //Press codes
    int padPressed;
    int padReleased;
    int stripPressed;
    int stripReleased;
    int penPressed;
    int penReleased;
    //Top and bottom values
    int stripTop;
    int stripBottom;
};

typedef struct{
    char *tabletName;
    char *stripEvent; //Event file name
    char *padEvent;
    char *penEvent;
    int stripUp; //Scroll up event key
    int stripDown; //Scroll down event key
    uint32_t padNum; //Number of pads on tablet
    uint32_t *padIDs; //Array of pad IDs
    struct padKeybind *padBinds; //Array of padNum number of binds
    struct tabletEventCodes codes;
}tabletConfig;

//config.c
extern tabletConfig *loadConfig();
extern void destroyConfig();
//proc.c
extern char *findDeviceEvent(char *name);
//events.c
extern void enableFakeKey(int key);
extern bool initFakeDevice();
extern void initEvents(int pfd, int sfd, tabletConfig *cnf);