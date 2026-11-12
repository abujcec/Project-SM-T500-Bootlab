# Build environment

BootLab is an EDK II UEFI application targeting AArch64.

## Prerequisites

- A supported EDK II checkout.
- A working AArch64 GCC toolchain (or another EDK II-supported AArch64 toolchain).
- Python and the normal EDK II build dependencies for your host OS.

## Integration

1. Set up EDK II following its upstream documentation.
2. Place this repository in the EDK II workspace, or add its path to the workspace package search path.
3. Ensure `MdePkg` is available in the workspace.
4. Add `BootLab/BootLab.inf` to a temporary platform DSC or build it through a small test DSC.
5. Build for the AARCH64 architecture using the selected toolchain.

The exact build command depends on the EDK II workspace, platform DSC, and toolchain configuration. Those details will be added once the target build environment is selected.

## Output

The expected UEFI application filename is `T500BootLab.efi`. Keep it separate from the device's existing boot files during initial testing.
