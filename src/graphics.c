#include "../os.h"
#include <stdarg.h>
#include <stdint.h>

// All rendering functions have been taken from https://github.com/BBjaba/BAS-kernel/blob/master/kernel/src/BasicRenderer.cpp
// as well as the structures PSF1_FONT and FrameBuffer

static unsigned int hor, vert;

void putpixel(uint32_t x, uint32_t y, uint32_t color) {
    *(uint32_t*)((uint64_t)framebuffer->BaseAddress + (x*4) + (y * framebuffer->PixelsPerScanLine * 4)) = color;
}

void clear() {
    uint64_t fbBase = (uint64_t)framebuffer->BaseAddress;
    uint64_t bytesPerScanline = framebuffer->PixelsPerScanLine * 4;
    uint64_t fbHeight = framebuffer->Height;
    uint64_t fbSize = framebuffer->BufferSize;
    for (int verticalScanline = 0; verticalScanline < fbHeight; verticalScanline ++)
    {
        uint64_t pixPtrBase = fbBase + (bytesPerScanline * verticalScanline);
        for (uint32_t* pixPtr = (uint32_t*)pixPtrBase; pixPtr < (uint32_t*)(pixPtrBase + bytesPerScanline); pixPtr ++)
        {
            *pixPtr = 0;;
        }
    }
}

void putchar(char chr, unsigned int xOff, unsigned int yOff) {
    unsigned int* pixPtr = (unsigned int*)framebuffer->BaseAddress;
    char* fontPtr = (char*)finishedFont->glyphBuffer + (chr * finishedFont->psf1_Header->charsize);
    if (chr == '\n' || hor + 8 > framebuffer->Width) {
        hor = 0;
        vert += 16;
        if (chr == 'n')
            return;
    }
    for (unsigned long y = yOff; y < yOff + 16; y++)
    {
        for (unsigned long x = xOff; x < xOff+8; x++)
        {
            if ((*fontPtr & (0b10000000 >> (x - xOff))) > 0)
            {
                *(unsigned int*)(pixPtr + x + (y * framebuffer->PixelsPerScanLine)) = 0xFFFFFF;
            }

        }
        fontPtr++;
    }
}

static int tostring(char* str, uint64_t num, int base)
{
    uint64_t i, rem, len = 0, n;
 
    n = num;
    while (n != 0)
    {
        len++;
        n /= base;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % base;
        num = num / base;
        switch (rem) {
            case 10 : str[len - (i + 1)] = 'A';
            case 11 : str[len - (i + 1)] = 'B';
            case 12 : str[len - (i + 1)] = 'C';
            case 13 : str[len - (i + 1)] = 'D';
            case 14 : str[len - (i + 1)] = 'E';
            case 15 : str[len - (i + 1)] = 'F';
            default: str[len - (i + 1)] = rem + '0';
        }
    }
    return len;
}

void print(const char* str, ...) {
    char fmt[1024];
    va_list ap;
    va_start(ap, str);
    int i = 0;
    while (*str != 0 || i < 1024) {
        if (*str == '%') {
            str++;
            switch (*str) {
                case 'u': {
                    uint64_t arg = va_arg(ap, uint64_t);
                    i += tostring(&fmt[i], arg, 10);
                    break;
                }
                case 'h': {
                    uint64_t arg = va_arg(ap, uint64_t);
                    i += tostring(&fmt[i], arg, 16);
                    break;
                }
                case 'c': {
                    char c = va_arg(ap, int);
                    fmt[i] = c;
                    i++;
                    break;
                }
            }
            str++;
            continue;
        }
        fmt[i] = *str;
        i++;
        str++;
    }
    fmt[i] = '\n';
    fmt[++i] = '\0';
    char* chr = (char*)fmt;
    while(*chr != 0)
    {
        putchar(*chr, hor, vert);
        hor += 8;
        chr++;
    }
}

void backspace()
{
    if (hor == 0){
        hor = framebuffer->Width;
        vert -= 16;
        if (vert < 0) vert = 0;
    }

    unsigned int xOff = hor;
    unsigned int yOff = vert;

    unsigned int* pixPtr = (unsigned int*)framebuffer->BaseAddress;
    for (unsigned long y = yOff; y < yOff + 16; y++)
    {
        for (unsigned long x = xOff - 8; x < xOff; x++)
        {
                    *(unsigned int*)(pixPtr + x + (y * framebuffer->PixelsPerScanLine)) = 0;
        }
    }

    hor -= 8;

    if (hor < 0)
    {
        hor = framebuffer->Width;
        vert -= 16;
        if (vert < 0) vert = 0;
    }
}