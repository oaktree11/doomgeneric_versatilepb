# doomgeneric_versatilepb

The code at in this github repository was used as the basis of the port to doom
to the versatilebp on QEMU:  

https://github.com/ozkl/doomgeneric.git  


filesystem - added in code for FAT filesystem and malloc mods  
buffered I/O - added in code for buffered I/O  
use

Use mk to build the versatilebp firmware. mk invokes make.  

You can use make clean to remove all object files  

mk also runs the firmware on the versatilepb board  

Uses a FAT file system in sdimage for the doom wad  
The newdoom1_1lev.wad was taken from here  
https://github.com/fragglet/squashware  






