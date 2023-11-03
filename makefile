.phony: all
all: disk.img
disk.img: boot/*.S boot/*.c
	i686-elf-gcc -nostdlib -ffreestanding -I include -T boot/bootloader.ld $^ -o boot.a -g
	objcopy -O binary boot.a boot.bin
	dd if=/dev/zero of=disk.img bs=1M count=64
	dd if=boot.bin of=disk.img conv=notrunc

.phony: debug
debug: disk.img boot.a
	qemu-system-i386 -s -S -hda disk.img -m 128M

.phony: clean
clean:
	rm *.a *.bin *.img