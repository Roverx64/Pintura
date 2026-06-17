# Pintura  
Pretty basic huion tablet driver for hyprland. Not for inexperienced linux or C users.  
This does not redirect pen movement to the tablet screen as that can be done via hyprland `device` configuration.  
Made for the `huion kamvas pro 13` and likely won't work with other models.  
  
I really only made this after no driver supported my tablet's touchstrip and later added pad support after `input-remapper` started having issues

##### Supports the tablet's touchstrip and pads

### Depenecies  
- GCC
- Linux's uinput

### Usage  
##### **Change `HUION_PAD` and `HUION_STRIP` input name if necessary. (located in pintura.h)**
The device names can be found using `cat /proc/bus/input/devices`.  

Find the target device, the atrribute normally looks like: `N: Name="HUION Huion Monitor Touch Strip"`  

Then change the `HUION_STRIP` to `HUION Huion Monitor Touch Strip` or whatever text is present.  

#### Run `make`, then run `sudo ./pintura.o`

To run it as root on startup, you'll have to research how to do that for your distro.  
**OR**  
You can change the device to allow the `wheel group` to open the input files via [**udev**](https://www.man7.org/linux/man-pages/man7/udev.7.html).

## Changing pad keybinds
You'll have to edit the C code to add new keypresses for the pads you desire. Comments have been left to help guide this in `events.c`  
Keys can be found [**here**](https://hackage-content.haskell.org/package/evdev-2.3.1.2/docs/Evdev-Codes.html) and must be in all caps.  

## Todo
- Add pen button support
- Add a simple config file to allow easier button changes and device name changes
- Add a GUI via SDL (maybe)
