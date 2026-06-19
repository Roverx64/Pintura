# Pintura  
Pretty basic huion tablet driver for hyprland. Not for inexperienced linux or C users.  
This does not redirect pen movement to the tablet screen as that can be done via hyprland `device` configuration.  
  
**Made for the `huion kamvas pro 13` and may work with other models with some tinkering.**
  
I really only made this after no driver supported my tablet's touchstrip and later added pad support after `input-remapper` started having issues

##### Supports the tablet's touchstrip and pads

### Depenecies  
- GCC
- Linux's uinput

### Usage  
#### **Config names are case sensitive!**  
##### **Change `StripName` and `PadName` input name if necessary. (located in pintura.conf)**
The device names can be found using `cat /proc/bus/input/devices`.  

Find the target device, the atrribute normally looks like: `N: Name="HUION Huion Monitor Touch Strip"`  

Then change the `StripName` to `HUION Huion Monitor Touch Strip` or whatever text is present.  

#### Run `make`, then run `sudo ./pintura.o`

To run it as sudo on startup, you'll have to research how to do that for your distro.  
**OR**  
You can change the device to allow the `wheel group` to open the input files via [**udev**](https://www.man7.org/linux/man-pages/man7/udev.7.html).

## Changing pad keybinds
Edit the `pintura.conf` to add keybind codes in order of the pads. This will be changed later to allow plain text writing for keybinds.  
Keys can be found [**here**](https://hackage-content.haskell.org/package/evdev-2.3.1.2/docs/Evdev-Codes.html)

## Todo
- Add pen button support
- Add error checks in config loader
- Add a GUI via SDL (unlikely)