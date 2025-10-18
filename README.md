# doomgeneric_versatilepb
## stage 0
Starting with the sdc example. The FAT file system middleware from here was added    
https://elm-chan.org/fsw/ff/


### Need to create the sdimage file as a FAT file system For example:

dd if=/dev/zero of=sdimage bs=1M count=64  
mkfs.vfat -F 16 sdimage  
sudo mkdir /mnt/sdcard_test 
sudo mount -o loop sdimage /mnt/sdcard_test

copy files to /mnt/sdcard_test which copies them into the file system in sdimage
### Need to be root to do the copying
sudo cp t.c /mnt/sdimage

### when finished with the sdimage file
umount /mnt/sdcard_test 
### If you wish to modify or list files in the file system then do
sudo mount -o loop sdimage /mnt/sdcard_test 

## TODO Create a test case for the firmware
see here  
https://elm-chan.org/fsw/ff/doc/open.html
