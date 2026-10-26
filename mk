make
arm-none-eabi-objcopy -O binary doomgeneric t.bin
#qemu-system-arm -M versatilepb -m 256M -sd sdimage -kernel t.bin -serial mon:stdio
/home/paul/Downloads/qemu-11.1.2/build/qemu-system-arm -M versatilepb -m 256M -sd sdimage -kernel t.bin -serial mon:stdio 
