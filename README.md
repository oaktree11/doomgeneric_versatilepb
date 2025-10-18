# doomgeneric_versatilepb
## stage 3 - Added in hooks to the doom source

### To clean up the compilation may need to run  
make clean
### To compile vid.c or sdc.c etc do
touch t.c  
./mk

## TODO Implement user keystroke input
keystrokes do not work - just the carriage return key is "implemented" 

Doom expects a key press and key release as two distinct actions by the user 
The key release will have to simulated.

