/** @file
  T500 BootLab v0.4: verbose Windows Boot Manager chainload diagnostics.

  Scans UEFI Simple File System handles and attempts to load and start the
  existing Windows Boot Manager. This application does not intentionally
  write partitions, BCD, EFI files, or NVRAM.
**/

#include <Uefi.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
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
  EFI_STATUS                       ImageProtocolStatus;
  UINTN                            HandleCount;
  EFI_HANDLE                       *Handles;
  EFI_HANDLE                       FoundHandle;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *FileSystem;
  EFI_FILE_PROTOCOL                *Root;
  EFI_FILE_PROTOCOL                *File;
  EFI_DEVICE_PATH_PROTOCOL         *BootManagerPath;
  EFI_HANDLE                       WindowsImage;
  EFI_LOADED_IMAGE_PROTOCOL        *LoadedImage;
  UINTN                            Index;
  UINTN                            ExitDataSize;
  CHAR16                           *ExitData;

  (VOID)SystemTable;

  FoundHandle = NULL;
  Handles = NULL;
  HandleCount = 0;
  BootManagerPath = NULL;
  WindowsImage = NULL;
  LoadedImage = NULL;
  ExitDataSize = 0;
  ExitData = NULL;

  Print(L"\n========================================\n");
  Print(L"          T500 BootLab v0.4              \n");
  Print(L"========================================\n");
  Print(L"Verbose Windows ARM64 chainload test\n");
  Print(L"Firmware vendor  : %s\n", gST->FirmwareVendor);
  Print(L"Firmware revision: 0x%08x\n", gST->FirmwareRevision);
  Print(L"SystemTable      : %p\n", SystemTable);
  Print(L"Parent image     : %p\n", ImageHandle);

  Print(L"\n[ENUM 01] Calling LocateHandleBuffer(SimpleFileSystem)...\n");
  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );
  Print(L"[ENUM 02] LocateHandleBuffer returned: %r (0x%lx)\n",
        Status, (UINT64)Status);

  if (EFI_ERROR(Status)) {
    goto Done;
  }

  Print(L"[ENUM 03] Filesystem handle count: %u\n", HandleCount);

  for (Index = 0; Index < HandleCount; Index++) {
    FileSystem = NULL;
    Root = NULL;
    File = NULL;

    Print(L"\n[FS %u.1] Handle: %p; calling HandleProtocol...\n",
          Index, Handles[Index]);
    Status = gBS->HandleProtocol(
                    Handles[Index],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID **)&FileSystem
                    );
    Print(L"[FS %u.2] HandleProtocol returned: %r (0x%lx), interface=%p\n",
          Index, Status, (UINT64)Status, FileSystem);
    if (EFI_ERROR(Status) || (FileSystem == NULL)) {
      continue;
    }

    Print(L"[FS %u.3] Calling OpenVolume...\n", Index);
    Status = FileSystem->OpenVolume(FileSystem, &Root);
    Print(L"[FS %u.4] OpenVolume returned: %r (0x%lx), root=%p\n",
          Index, Status, (UINT64)Status, Root);
    if (EFI_ERROR(Status) || (Root == NULL)) {
      continue;
    }

    Print(L"[FS %u.5] Opening %s...\n",
          Index, WINDOWS_BOOT_MANAGER_PATH);
    Status = Root->Open(
                    Root,
                    &File,
                    WINDOWS_BOOT_MANAGER_PATH,
                    EFI_FILE_MODE_READ,
                    0
                    );
    Print(L"[FS %u.6] File Open returned: %r (0x%lx), file=%p\n",
          Index, Status, (UINT64)Status, File);

    if (!EFI_ERROR(Status) && (File != NULL)) {
      Print(L"[FS %u.7] Boot Manager file found; closing file/root...\n",
            Index);
      File->Close(File);
      Root->Close(Root);
      FoundHandle = Handles[Index];
      Print(L"[FS %u.8] Selected filesystem handle %p\n",
            Index, FoundHandle);
      break;
    }

    Print(L"[FS %u.7] File not found on this handle; closing root.\n",
          Index);
    Root->Close(Root);
  }

  if (Handles != NULL) {
    gBS->FreePool(Handles);
    Handles = NULL;
    Print(L"[ENUM 04] Freed filesystem handle buffer.\n");
  }

  if (FoundHandle == NULL) {
    Print(L"\n[SCAN] Windows Boot Manager was not found on any filesystem.\n");
    goto Done;
  }

  Print(L"\n[PATH 01] Calling FileDevicePath for selected handle...\n");
  BootManagerPath = FileDevicePath(
                      FoundHandle,
                      WINDOWS_BOOT_MANAGER_PATH
                      );
  Print(L"[PATH 02] FileDevicePath returned: %p\n", BootManagerPath);
  if (BootManagerPath == NULL) {
    Print(L"[PATH 03] Failed to construct device path.\n");
    goto Done;
  }

  Print(L"\n[LOAD 01] About to call LoadImage(FALSE, parent=%p, path=%p, source=NULL, size=0)...\n",
        ImageHandle, BootManagerPath);
  Print(L"[LOAD 02] If the display stops here, the firmware is not returning from LoadImage.\n");

  Status = gBS->LoadImage(
                  FALSE,
                  ImageHandle,
                  BootManagerPath,
                  NULL,
                  0,
                  &WindowsImage
                  );

  Print(L"[LOAD 03] LoadImage RETURNED: %r (0x%lx)\n",
        Status, (UINT64)Status);
  Print(L"[LOAD 04] Loaded image handle: %p\n", WindowsImage);

  if (EFI_ERROR(Status)) {
    Print(L"[LOAD 05] LoadImage failed; StartImage will NOT be called.\n");
    goto Done;
  }

  Print(L"[IMAGE 01] Calling HandleProtocol(LoadedImage) on %p...\n",
        WindowsImage);
  ImageProtocolStatus = gBS->HandleProtocol(
                             WindowsImage,
                             &gEfiLoadedImageProtocolGuid,
                             (VOID **)&LoadedImage
                             );
  Print(L"[IMAGE 02] LoadedImage protocol status: %r (0x%lx), interface=%p\n",
        ImageProtocolStatus, (UINT64)ImageProtocolStatus, LoadedImage);

  if (!EFI_ERROR(ImageProtocolStatus) && (LoadedImage != NULL)) {
    Print(L"[IMAGE 03] ParentHandle=%p DeviceHandle=%p\n",
          LoadedImage->ParentHandle, LoadedImage->DeviceHandle);
    Print(L"[IMAGE 04] ImageBase=%p ImageSize=0x%lx LoadOptionsSize=%u\n",
          LoadedImage->ImageBase, (UINT64)LoadedImage->ImageSize,
          LoadedImage->LoadOptionsSize);
    Print(L"[IMAGE 05] Image loaded successfully; preparing StartImage.\n");
  }

  Print(L"\n[START 01] About to call StartImage(%p)...\n", WindowsImage);
  Print(L"[START 02] If the display stops here, StartImage has not returned.\n");
  StartStatus = gBS->StartImage(
                       WindowsImage,
                       &ExitDataSize,
                       &ExitData
                       );

  Print(L"[START 03] StartImage RETURNED: %r (0x%lx)\n",
        StartStatus, (UINT64)StartStatus);
  Print(L"[START 04] ExitDataSize=%u ExitData=%p\n",
        ExitDataSize, ExitData);

  if ((ExitData != NULL) && (ExitDataSize >= sizeof(CHAR16))) {
    Print(L"[START 05] Child image exit text: %s\n", ExitData);
    gBS->FreePool(ExitData);
    ExitData = NULL;
  }

  Print(L"[IMAGE 06] Calling UnloadImage(%p)...\n", WindowsImage);
  Status = gBS->UnloadImage(WindowsImage);
  Print(L"[IMAGE 07] UnloadImage returned: %r (0x%lx)\n",
        Status, (UINT64)Status);

Done:
  if (ExitData != NULL) {
    gBS->FreePool(ExitData);
  }
  if (BootManagerPath != NULL) {
    gBS->FreePool(BootManagerPath);
  }
  if (Handles != NULL) {
    gBS->FreePool(Handles);
  }

  Print(L"\n[DONE] BootLab finished. No EFI files, BCD, partitions, or NVRAM were intentionally modified.\n");
  Print(L"[DONE] Final application status: EFI_SUCCESS\n");
  return EFI_SUCCESS;
}
