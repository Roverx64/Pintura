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

extern char *findDeviceEvent(char *name);
extern void initEvents(int padfd, int stripfd);