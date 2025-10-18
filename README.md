# doomgeneric_versatilepb
## stage 1 buffered I/O

Can now include <stdio.h> and use functions like fopen  
writes are not implemented

## Create a FAT file system
Need to create the sdimage file as a FAT file system For example:

dd if=/dev/zero of=sdimage bs=1M count=64  
mkfs.vfat -F 16 sdimage 
sudo mkdir /mnt/sdcard_test 
sudo mount -o loop sdimage /mnt/sdcard_test 

## copy files to /mnt/sdcard_test which copies them into the file system in sdimage
### Need to be root to do the copying
sudo cp t.c /mnt/sdimage

### when finished with the sdimage file
umount /mnt/sdcard_test 
### If you wish to modify or list files in the file system then do
sudo mount -o loop sdimage /mnt/sdcard_test 

## TODO implement writes and a corresponding test case
### there is a #define of printf to uprintf in the code your 
#include <stdio.h> 
### needs to be after that #define
You will need to: 
1. write the code for _write (does not exist) in t.c 
2. modify the _open function to handle open for writing.
