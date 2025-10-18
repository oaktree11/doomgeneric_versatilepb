# doomgeneric_versatilepb
## stage 2 added in doomgeneric source

https://github.com/ozkl/doomgeneric.git

The doomgeneric Makefile was updated to incorporate stage-0 and stage-1 

the ./mk script invokes make and then runs qemu with the appropriate exe  
### To clean up the compilation may need to run  
make clean
### To compile vid.c or sdc.c etc do
touch t.c  
./mk

## ToDO use vscode to debug stage-2
should get errors about not being able to find a wad file
### Example workflow below.
### A. View call stack for exit
1. use vscode to debug the stage 2 code
2. run the ./mkdebug
3. In vscode run->start debugging
4. the pause does not work
5. So need to put a break point in the code
6. The function _exit in t.c may get invoked.
7. Put a break point in the _exit function and then look at the call stack on the left hand menu.
8. Give an overview of the final functions call stack in your report

### B. Create a new sdimage file with a doom1.wad in it
Get doom1.wad from here 
https://doomwiki.org/wiki/DOOM1.WAD 

debug using vscode  
Give an overview of the final function call stack in your report

