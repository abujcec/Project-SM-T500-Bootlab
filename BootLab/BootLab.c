/** @file
  T500 BootLab: minimal ARM64 UEFI diagnostics application.

  This first version is intentionally read-only. It does not modify NVRAM,
  partitions, EFI files, or the Windows boot configuration.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  UINTN       HandleCount;
  EFI_HANDLE  *Handles;
  EFI_INPUT_KEY Key;

  (VOID)ImageHandle;

  Print(L"\n========================================\n");
  Print(L"          T500 BootLab v0.1              \n");
  Print(L"========================================\n");
  Print(L"Read-only UEFI diagnostics\n\n");
  Print(L"Firmware vendor  : %s\n", SystemTable->FirmwareVendor);
  Print(L"Firmware revision: 0x%08x\n", SystemTable->FirmwareRevision);
  Print(L"UEFI revision    : %u.%u\n",
        SystemTable->Hdr.Revision >> 16,
        SystemTable->Hdr.Revision & 0xffff);

  Print(L"\nLocating Simple File System handles...\n");
  HandleCount = 0;
  Handles = NULL;
  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );

  if (EFI_ERROR(Status)) {
    Print(L"Simple File System lookup failed: %r\n", Status);
  } else {
    Print(L"Simple File System handles: %u\n", HandleCount);
    if (Handles != NULL) {
      gBS->FreePool(Handles);
    }
  }

  Print(L"\nNo files or boot settings were changed.\n");
  Print(L"Press any key to return to the UEFI menu...\n");
  while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key) == EFI_NOT_READY) {
    gBS->Stall(10000);
  }

  return EFI_SUCCESS;
}
