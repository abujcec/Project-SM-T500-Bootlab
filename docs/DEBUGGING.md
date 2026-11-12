# Debugging checklist

## Before testing

- Keep a known-good way to return to the existing UEFI menu.
- Do not overwrite `bootaa64.efi`, `bootmgfw.efi`, or the Windows BCD.
- Keep a copy of the current EFI partition contents.
- Record the exact firmware build and device board ID.

## Verify PE machine type on the PC

Use PowerShell to inspect the PE/COFF machine field of each executable:

```powershell
$files = @(
  "D:\EFI\Boot\bootaa64.efi",
  "D:\EFI\Microsoft\Boot\bootmgfw.efi",
  "G:\Windows\system32\winload.efi",
  "G:\Windows\system32\ntoskrnl.exe"
)

foreach ($file in $files) {
    $bytes = [IO.File]::ReadAllBytes($file)
    $offset = [BitConverter]::ToInt32($bytes, 0x3C)
    $machine = [BitConverter]::ToUInt16($bytes, $offset + 4)
    "{0} : 0x{1:X4}" -f $file, $machine
}
```

Expected ARM64 PE machine value: `0xAA64`.

## Record each test

For every test, write down:

- BootLab build / commit.
- Whether BootLab appeared on screen.
- Firmware messages before and after launch.
- Whether the device returned to the UEFI menu.
- Any visible status code or serial/debug output.

Do not infer the cause of a boot failure from a repeated boot-entry label alone.
