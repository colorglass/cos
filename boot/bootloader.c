#define VIDEO_MEMORY 0xB8000

void __attribute__((noreturn)) boot_main() 
{
    unsigned short *video_memory = (unsigned short *)VIDEO_MEMORY;
    *video_memory = (unsigned short)('A' | 0x0700);
    while(1);
}