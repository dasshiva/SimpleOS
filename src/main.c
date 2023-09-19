#include "../os.h"

EFI_SYSTEM_TABLE* ST;
EFI_BOOT_SERVICES* BS;
EFI_RUNTIME_SERVICES* RT;
EFI_SIMPLE_TEXT_IN_PROTOCOL* stdin;
EFI_SIMPLE_TEXT_OUT_PROTOCOL* stdout;
EFI_SIMPLE_TEXT_OUT_PROTOCOL* stderr;
EFI_FILE_HANDLE bootdev;

EFIAPI
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    // First Save all needed things
    ST = SystemTable;
    BS = ST->BootServices;
    RT = ST->RuntimeServices;
    stdin = ST->ConIn;
    stdout = ST->ConOut;
    stderr = ST->StdErr;

    // Welcome the user
    EFI_STATUS Status;
    stdout->ClearScreen(stdout);
    stdout->OutputString(stdout, L"Welcome to FreedomOS\r\n");
    stdout->OutputString(stdout, L"Please wait while the operating system initializes itself\r\n");
    
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
    EFI_CALL(bootdev->Open(bootdev, &file, L"kernel.wasm", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM),
        "Could not open kernel file");
    
    // Now try to get the file's size
    // First call with a buffer of size 1 (deliberately too small)
    // This will put the requiresd size in sz
    // Then again call with the new required size
    // The call is made twice because the size of the file descriptor is implentation defined and maybe different on
    // different machines
    EFI_FILE_INFO* details;
    alloc(EfiLoaderData, 1, details);
    UINTN sz = 1;
    EFI_GUID finfo = EFI_FILE_INFO_ID;
    file->GetInfo(file, &finfo, &sz, details);
    free(details);
    alloc(EfiLoaderData, sz, details);
    EFI_CALL(file->GetInfo(file, &finfo, &sz, details), "Could not get kernel file information");

    // We have the required size. Allocate a buffer for the kernel and print a message to the user when done
    UINTN size = details->FileSize;
    UINT8* Buffer;
    alloc(EfiLoaderData, size, Buffer);
    EFI_CALL(file->Read(file, &size, Buffer), "Loading kernel failed");
    file->Close(file);

    stdout->OutputString(stdout, L"Loaded the kernel"); 
    execute_kernel(Buffer, size); // This should never return
    return Status;
}