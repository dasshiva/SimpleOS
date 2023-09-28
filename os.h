#ifndef _OS_H_
#define _OS_H_

#include "inc/efi.h"
#include "inc/efiapi.h"
#include "inc/eficon.h"
#include "inc/efiprot.h"

extern EFI_SYSTEM_TABLE* ST;
extern EFI_BOOT_SERVICES* BS;
extern EFI_RUNTIME_SERVICES* RT;
extern EFI_SIMPLE_TEXT_IN_PROTOCOL* stdin;
extern EFI_SIMPLE_TEXT_OUT_PROTOCOL* stdout;
extern EFI_SIMPLE_TEXT_OUT_PROTOCOL* stderr;
extern EFI_FILE_HANDLE bootdev;
extern EFI_HANDLE IM;

#define EFI_CALL(action , a) \
Status = action; \
if (EFI_ERROR(Status)) \
{                 \
    stdout->OutputString(stdout, L##a "\r\n"); \
	return EFI_SUCCESS; \
}

#define EFI_CALL_RETURN(action , a, ret) \
Status = action; \
if (EFI_ERROR(Status)) \
{                 \
    stdout->OutputString(stdout, L##a "\r\n"); \
    return ret; \
}

#define alloc(type, size, dest) \
Status = BS->AllocatePool(type, size, (void**) &dest); \
if (EFI_ERROR(Status)) {\
   stdout->OutputString(stdout, L"Out of memory"); \
   return EFI_SUCCESS; \
}

#define alloc_return(type, size, dest, ret) \
Status = BS->AllocatePool(type, size, (void**) &dest); \
if (EFI_ERROR(Status)) {\
   stdout->OutputString(stdout, L"Out of memory"); \
   return ret; \
}

#define free(dest) \
BS->FreePool(dest);

typedef struct {
	void* BaseAddress;
	size_t BufferSize;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanLine;
} Framebuffer;

extern Framebuffer* framebuffer;
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;

typedef struct {
	PSF1_HEADER* psf1_Header;
	void* glyphBuffer;
} PSF1_FONT;

extern PSF1_FONT* finishedFont;

void putpixel(uint32_t x, uint32_t y, uint32_t color);
void clear();
void putchar(char chr, unsigned int xOff, unsigned int yOff);
void print(const char* str, ...);
void backspace();
void manager_init(EFI_MEMORY_DESCRIPTOR* desc);
void* malloc(uint64_t sz);
int len(const CHAR16* s);
int cmp(const CHAR16* s1, const CHAR16* s2);
void getline(CHAR16* buf);

#define true TRUE
#define false FALSE

#endif