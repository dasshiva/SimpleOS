#include "../os.h"
#include <stdint.h>

EFI_SYSTEM_TABLE* ST;
EFI_BOOT_SERVICES* BS;
EFI_RUNTIME_SERVICES* RT;
EFI_SIMPLE_TEXT_IN_PROTOCOL* stdin;
EFI_SIMPLE_TEXT_OUT_PROTOCOL* stdout;
EFI_SIMPLE_TEXT_OUT_PROTOCOL* stderr;
EFI_FILE_HANDLE bootdev;
Framebuffer* framebuffer;
PSF1_FONT* finishedFont;
EFI_HANDLE IM;
uint64_t total_mem;

#define BUFSZ 1024

EFIAPI
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    // First Save all needed things
    ST = SystemTable;
    IM = ImageHandle;
    BS = ST->BootServices;
    RT = ST->RuntimeServices;
    stdin = ST->ConIn;
    stdout = ST->ConOut;
    stderr = ST->StdErr;

    EFI_STATUS Status;
    
    // Find and save the boot device
    // First open the handle
    EFI_LOADED_IMAGE *loaded_image = NULL;                  /* image interface */
    EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;      /* image interface GUID */
    EFI_FILE_IO_INTERFACE *IOVolume;                        /* file system interface */
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID; /* file system interface GUID */
    EFI_CALL(BS->HandleProtocol(ImageHandle, &lipGuid, (void**) &loaded_image), "Could not find loaded image protocol");
    EFI_CALL(BS->HandleProtocol(loaded_image->DeviceHandle, &fsGuid, (void**)&IOVolume), "Could not find boot volume");
    EFI_CALL(IOVolume->OpenVolume(IOVolume, &bootdev), "Could not open boot device volume");

    // Once the handle has been opened search for the kernel file
    EFI_FILE_HANDLE file;
    EFI_FILE_INFO* details;
    UINTN sz = 1;

    // Then load the font file and enable rendering via gop
    EFI_CALL(bootdev->Open(bootdev, &file, L"font.psf", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM),
        "Could not open font file");
    PSF1_HEADER* fontHeader;
    alloc(EfiLoaderData, sizeof(PSF1_HEADER), fontHeader);
    sz = sizeof(PSF1_HEADER);
    EFI_CALL(file->Read(file, &sz, fontHeader), "Loading font failed");
    stdout->OutputString(stdout, L"Loaded font file header\r\n"); 

    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	EFI_STATUS status;

	status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
	if(EFI_ERROR(status)){
		stdout->OutputString(stdout, L"Unable to locate GOP\n\r");
		return EFI_SUCCESS;
	}
	else
	{
		stdout->OutputString(stdout, L"GOP located\n\r");
	}

    alloc(EfiLoaderData, sizeof(Framebuffer), framebuffer);
	framebuffer->BaseAddress = (void*)gop->Mode->FrameBufferBase;
	framebuffer->BufferSize = gop->Mode->FrameBufferSize;
	framebuffer->Width = gop->Mode->Info->HorizontalResolution;
	framebuffer->Height = gop->Mode->Info->VerticalResolution;
	framebuffer->PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;


	if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1){
        stdout->OutputString(stdout, L"Invalid font file");
		return EFI_SUCCESS;
	}

    UINTN glyphBufferSize = fontHeader->charsize * 256;
	if (fontHeader->mode == 1) {
		glyphBufferSize = fontHeader->charsize * 512;
	}
    
    void* glyphBuffer;
	file->SetPosition(file, sizeof(PSF1_HEADER));
	alloc(EfiLoaderData, glyphBufferSize, glyphBuffer);
	file->Read(file, &glyphBufferSize, glyphBuffer);
    SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)&finishedFont);
	finishedFont->psf1_Header = fontHeader;
	finishedFont->glyphBuffer = glyphBuffer;

    stdout->OutputString(stdout, L"Loaded fonts\r\n");
    clear();
    print("Welcome to FreedomOS - Make your computer, yours.\n");
    print("Initialising system...hold on just a second\n");
    EFI_MEMORY_DESCRIPTOR* map = NULL;
    uint64_t mapsize, mapkey, descriptorsize;
    uint32_t descriptorversion;
    uint64_t memory_size = 0;

    BS->GetMemoryMap(&mapsize, map, &mapkey, &descriptorsize, &descriptorversion);
    alloc(EfiLoaderData, mapsize, map);
    BS->GetMemoryMap(&mapsize, map, &mapkey, &descriptorsize, &descriptorversion);
    int total = mapsize / descriptorsize, i = 0;
    while (i < total) {
        EFI_MEMORY_DESCRIPTOR* desc = (uint8_t*) map + i * descriptorsize;
        if (desc->Type == EfiConventionalMemory || desc->Type == EfiPersistentMemory) {
            manager_init(desc);
            total_mem += desc->NumberOfPages * EFI_PAGE_SIZE;
        }
        i++;
    }

    print("OS is Ready. Input commands below:\n");
    stdin->Reset(stdin, false);
    CHAR16 cmdbuf[BUFSZ];
    while(true) {
        print("> ");
        getline(cmdbuf);
        print("\n");
        if (cmp(cmdbuf, L"shutdown") == 1) {
            print("Shutting down");
            RT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        }
        else {
            print("Unknown command\n");
        }
        
    }
    return EFI_SUCCESS;
}

void getline(CHAR16* buf) {
    int i = 0;
    while (true) {
        EFI_INPUT_KEY key;
        while (stdin->ReadKeyStroke(stdin, &key) != EFI_SUCCESS);
        if (key.UnicodeChar == '\n' || key.UnicodeChar == '\r')
            break;
        else if (key.ScanCode == 0x08 || key.UnicodeChar == 0x08) {
            backspace();
            continue;
        }
        print("%c", (int) key.UnicodeChar);
        buf[i] = key.UnicodeChar;
        i++;
    }
    buf[i] = '\0';
}