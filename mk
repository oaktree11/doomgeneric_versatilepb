arm-none-eabi-as -mcpu=arm926ej-s -g ts.s -o ts.o
arm-none-eabi-gcc -O0 -c -mcpu=arm926ej-s -g t.c -o t.o
arm-none-eabi-gcc -T t.ld -specs=nosys.specs ts.o t.o -o t.elf
arm-none-eabi-objcopy -O binary t.elf t.bin



echo ready to go?
read dummy

qemu-system-arm -M versatilepb -m 256M -sd sdimage -kernel t.bin -serial mon:stdio



 





