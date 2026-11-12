# Windows ARM64 boot flow on the SM-T500

## Intended chain

1. Samsung Galaxy Tab A7 firmware / custom UEFI.
2. T500 BootLab diagnostic application (optional; does not replace firmware).
3. Existing ARM64 Microsoft Windows Boot Manager: `bootmgfw.efi`.
4. Windows OS loader: `winload.efi`.
5. Windows kernel and boot-start drivers.

## Current investigation

The deployment previously produced these expected files:

- EFI fallback: `EFI/Boot/bootaa64.efi`
- Microsoft boot manager: `EFI/Microsoft/Boot/bootmgfw.efi`
- Windows loader: `Windows/System32/winload.efi`
- Windows kernel: `Windows/System32/ntoskrnl.exe`

File presence alone does not prove that every executable is ARM64 or that firmware can execute it.

## First diagnostic milestone

Run BootLab as a standalone UEFI application and capture:

- Whether the application starts at all.
- Firmware vendor and revision.
- Number of EFI Simple File System handles.
- Any firmware console or protocol errors.

Only after this works should we add read-only inspection of candidate EFI files and consider a chain-load test.
