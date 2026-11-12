# T500 BootLab

Experimental ARM64 UEFI diagnostics and boot-chain research for the Samsung Galaxy Tab A7 Wi-Fi (SM-T500, `gta4lwifi`).

> Early scaffold. This project does not replace the device firmware or Microsoft's Windows boot manager.

## Initial goal

Build a standalone, read-only ARM64 UEFI diagnostic application before attempting any Windows boot-chain changes.

## Planned layout

- `BootLab/` — EDK II application source and INF.
- `docs/` — boot-flow notes and debugging checklist.
- `Build/` — build environment instructions.

## Safety

Keep the existing EFI files and firmware intact. Test the diagnostic application separately and retain a known-good recovery path. A launcher cannot by itself fix incompatible ACPI tables, memory maps, or drivers.
