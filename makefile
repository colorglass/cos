toolchain ?= ~/opt/cross/bin
prefix ?= i686-elf

CC = $(toolchain)/$(prefix)-gcc

.phony: all
all: disk.img
disk.img: boot/*.S boot/*.c
	$(CC) -nostdlib -ffreestanding -I include -T boot/bootloader.ld $^ -o boot.a -g
	objcopy -O binary boot.a boot.bin
	truncate -s 1M disk.img
	truncate -s 64M part_main		# fat32 has a min size of 32M
	mkfs.fat -F 32 part_main
	cat part_main >> disk.img		
	rm part_main
	truncate -s +512 disk.img		# the last sector of the disk seems to not be used in fdisk partition
	fdisk disk.img < fdisk.txt
	dd if=boot.bin of=disk.img conv=notrunc bs=440 count=1
	dd if=boot.bin of=disk.img conv=notrunc bs=512 skip=1 seek=1

.phony: debug
debug: disk.img boot.a
	qemu-system-i386 -s -S -curses -hda disk.img -m 128M

.phony: clean
clean:
	rm *.a *.bin *.img