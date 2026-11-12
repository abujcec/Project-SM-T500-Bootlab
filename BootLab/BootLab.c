/** @file
  T500 BootLab v0.3: automatic Windows Boot Manager chainload test.

  Scans UEFI Simple File System handles for the existing Microsoft ARM64
  boot manager and launches it immediately when found. It does not write
  partitions, BCD, EFI files, or NVRAM.
**/

#include <Uefi.h>
#include <Protocol/SimpleFileSystem.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/DevicePathLib.h>

#define WINDOWS_BOOT_MANAGER_PATH L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi"

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                       Status;
  EFI_STATUS                       StartStatus;
  UINTN                            HandleCount;
  EFI_HANDLE                       *Handles;
  EFI_HANDLE                       FoundHandle;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *FileSystem;
  EFI_FILE_PROTOCOL                *Root;
  EFI_FILE_PROTOCOL                *File;
  EFI_DEVICE_PATH_PROTOCOL         *BootManagerPath;
  EFI_HANDLE                       WindowsImage;
  UINTN                            Index;

  (VOID)SystemTable;

  FoundHandle = NULL;
  Handles = NULL;
  HandleCount = 0;
  BootManagerPath = NULL;
  WindowsImage = NULL;

  Print(L"\n========================================\n");
  Print(L"          T500 BootLab v0.3              \n");
  Print(L"========================================\n");
  Print(L"Automatic Windows ARM64 chainload\n");
  Print(L"Firmware vendor  : %s\n", gST->FirmwareVendor);
  Print(L"Firmware revision: 0x%08x\n", gST->FirmwareRevision);

  Print(L"\nScanning %s on UEFI filesystem volumes...\n",
        WINDOWS_BOOT_MANAGER_PATH);

  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );

  if (EFI_ERROR(Status)) {
    Print(L"Filesystem enumeration failed: %r\n", Status);
    goto Done;
  }

  Print(L"Found %u filesystem handles.\n", HandleCount);

  for (Index = 0; Index < HandleCount; Index++) {
    FileSystem = NULL;
    Root = NULL;
    File = NULL;

    Status = gBS->HandleProtocol(
                    Handles[Index],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID **)&FileSystem
                    );
    if (EFI_ERROR(Status) || (FileSystem == NULL)) {
      continue;
    }

    Status = FileSystem->OpenVolume(FileSystem, &Root);
    if (EFI_ERROR(Status) || (Root == NULL)) {
      continue;
    }

    Status = Root->Open(
                    Root,
                    &File,
                    WINDOWS_BOOT_MANAGER_PATH,
                    EFI_FILE_MODE_READ,
                    0
                    );

    if (!EFI_ERROR(Status) && (File != NULL)) {
      File->Close(File);
      Root->Close(Root);
      FoundHandle = Handles[Index];
      Print(L"Windows Boot Manager found on filesystem handle %u.\n",
            Index);
      break;
    }

    Root->Close(Root);
  }

  if (Handles != NULL) {
    gBS->FreePool(Handles);
    Handles = NULL;
  }

  if (FoundHandle == NULL) {
    Print(L"\nWindows Boot Manager was not found.\n");
    Print(L"Check that the EFI partition is visible to this firmware.\n");
    goto Done;
  }

  BootManagerPath = FileDevicePath(
                      FoundHandle,
                      WINDOWS_BOOT_MANAGER_PATH
                      );
  if (BootManagerPath == NULL) {
    Print(L"Could not construct the boot manager device path.\n");
    goto Done;
  }

  Print(L"\nLaunching Windows Boot Manager automatically...\n");
  Status = gBS->LoadImage(
                  FALSE,
                  ImageHandle,
                  BootManagerPath,
                  NULL,
                  0,
                  &WindowsImage
                  );
  if (EFI_ERROR(Status)) {
    Print(L"LoadImage failed: %r\n", Status);
    goto Done;
  }

  Print(L"Starting Windows Boot Manager...\n");
  StartStatus = gBS->StartImage(WindowsImage, NULL, NULL);
  Print(L"Windows Boot Manager returned: %r\n", StartStatus);

  gBS->UnloadImage(WindowsImage);

Done:
  if (BootManagerPath != NULL) {
    gBS->FreePool(BootManagerPath);
  }

  if (Handles != NULL) {
    gBS->FreePool(Handles);
  }

  Print(L"\nBootLab finished. No EFI files, BCD, partitions, or NVRAM were modified.\n");
  return EFI_SUCCESS;
}
