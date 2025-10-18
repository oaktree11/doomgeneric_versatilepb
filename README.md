# doomgeneric_versatilepb
## stage 0
Starting with the sdc example. Add the FAT file system middleware from here  
https://elm-chan.org/fsw/ff/


### Need to create the sdimage file as a FAT file system For example:

dd if=/dev/zero of=sdimage bs=1M count=64  
mkfs.vfat -F 16 sdimage  
sudo mkdir /mnt/sdcard_test 
sudo mount -o loop sdimage /mnt/sdcard_test

copy files to /mnt/sdcard_test which copies them into the file system in sdimage
### Need to be root to do the copying
sudo cp t.c /mnt/sdimage

