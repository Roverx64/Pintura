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
- Change `HUION_PAD` and `HUION_STRIP` input paths if necessary.
- Run `make`, then run `sudo ./pintura.o`

To run it as sudo on startup, you'll have to research how to do that for your distro.  

## Changing pad keybinds
You'll have to edit the C code to add new keypresses for the pads you desire. Comments have been left to help guide this.  
Keys can be found [here](https://hackage-content.haskell.org/package/evdev-2.3.1.2/docs/Evdev-Codes.html) and must be in all caps.  
