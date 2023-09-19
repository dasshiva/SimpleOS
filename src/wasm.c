#include "../os.h"

typedef struct wasm_file {

} file;

#define read_u1(c) c = buf[i++]
static void parse(UINT8* buf, UINTN size, BOOLEAN iskernel) {
    UINTN pos = 0;

}

static file* load(UINT8* buf, BOOLEAN iskernel, UINTN size, WCHAR* filename) {
    file* target;
    UINTN Status;
    if (iskernel) {
        alloc_return(EfiLoaderData, size, target, NULL);
        parse(buf, size, TRUE);
        return target;
    }
    else {
        EFI_FILE_HANDLE file;
        EFI_CALL_RETURN(bootdev->Open(bootdev, &file, filename, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM),
        "Could not open executable file", NULL);
        EFI_FILE_INFO* details;
        alloc_return(EfiConventionalMemory, 1, details, NULL);
        UINTN sz = 1;
        EFI_GUID finfo = EFI_FILE_INFO_ID;
        file->GetInfo(file, &finfo, &sz, details);
        free(details);
        alloc_return(EfiConventionalMemory, sz, details, NULL);
        EFI_CALL_RETURN(file->GetInfo(file, &finfo, &sz, details), "Could not get executable information", NULL);
        UINTN size = details->FileSize;
        UINT8* Buffer;
        alloc_return(EfiConventionalMemory, size, Buffer, NULL);
        EFI_CALL_RETURN(file->Read(file, &size, Buffer), "Loading executable failed", NULL);
        file->Close(file);
        return NULL;
    }
}

#define load_kernel(buf, size) load(buf, TRUE, size, NULL)
#define load_user(filename) load(NULL, FALSE, 0, filename)

__attribute__((__noreturn__))
EFI_STATUS execute_kernel(UINT8* buf, UINTN size) {
    file* kernel = load_kernel(buf, size);
    while(1) {}
}