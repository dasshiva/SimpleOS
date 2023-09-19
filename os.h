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

EFI_STATUS execute_kernel(UINT8* buf, UINTN size);

#endif