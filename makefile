toolchain ?= ~/opt/cross/bin
prefix ?= i686-elf

CC = $(toolchain)/$(prefix)-gcc

.phony: all
all: disk.img
disk.img: boot/*.S boot/*.c
	$(CC) -nostdlib -ffreestanding -I include -T boot/bootloader.ld $^ -o boot.a -g
	objcopy -O binary boot.a boot.bin
	truncate -s 1M disk.img
	truncate -s 8M part1
	mkfs.fat -F 32 part1
	cat part1 >> disk.img
	truncate -s 55M part2
	mkfs.fat -F 32 part2
	cat part2 >> disk.img
	rm part1 part2
	fdisk disk.img < fdisk.txt
	dd if=boot.bin of=disk.img conv=notrunc bs=440 count=1
	dd if=boot.bin of=disk.img conv=notrunc bs=512 skip=1 seek=1

.phony: debug
debug: disk.img boot.a
	qemu-system-i386 -s -S -curses -hda disk.img -m 128M

.phony: clean
clean:
	rm *.a *.bin *.img