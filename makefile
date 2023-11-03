all: boot/*.S boot/*.c
	i686-elf-gcc -nostdlib -ffreestanding -T boot/bootloader.ld boot/bootloader.c boot/bootsec.S -o boot.a -g
	objcopy -O binary boot.a boot.bin
	dd if=boot.bin of=disk.img bs=512 count=2 conv=notrunc
