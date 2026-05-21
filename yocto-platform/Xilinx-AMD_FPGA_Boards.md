# Xilinx / AMD FPGA Boards + Yocto Workflow

AMD Xilinx platforms are heavily used in:

* FPGA acceleration
* telecom
* SDR/radio systems
* industrial vision
* AI pipelines
* aerospace/defense
* high-speed networking

Popular platforms:

* ZCU102
* KV260 Vision AI Starter Kit
* PYNQ-Z2
* ZedBoard
* VCK190

Main SoC families:

* Zynq-7000
* Zynq UltraScale+ MPSoC
* Versal

---

# 1. How Xilinx Uses Yocto

AMD Xilinx officially supports Yocto through:

* `meta-xilinx`

Official ecosystem:

* PetaLinux
* Vivado
* Vitis

Main BSP layers:

* `meta-xilinx`
* `meta-xilinx-tools`
* `meta-petalinux`

Official source:

* [meta-xilinx GitHub](https://github.com/Xilinx/meta-xilinx?utm_source=chatgpt.com)

---

# 2. Important Concept

Xilinx platforms are different from Raspberry Pi/i.MX.

The FPGA hardware design itself affects Linux.

Workflow:

```text id="k3m9x7"
Vivado Hardware Design
        ↓
XSA/HDF Export
        ↓
Yocto/PetaLinux Build
        ↓
Boot Image Generation
        ↓
Flash
        ↓
Linux + FPGA Runtime
```

Linux and FPGA are tightly integrated.

---

# 3. Typical Yocto Layer Structure

```text id="9m2r4x"
poky/
meta-openembedded/
meta-xilinx/
meta-xilinx-tools/
meta-petalinux/
meta-company/
```

---

# 4. Main BSP Components

| Component     | Purpose                  |
| ------------- | ------------------------ |
| `meta-xilinx` | BSP support              |
| Vivado        | FPGA hardware design     |
| Vitis         | AI/software acceleration |
| PetaLinux     | Xilinx Linux tooling     |
| U-Boot        | Bootloader               |
| FSBL          | First stage bootloader   |
| PMUFW         | Platform firmware        |

---

# 5. Build Environment Setup

---

# Install Host Dependencies

Ubuntu example:

```bash id="2x9m4v"
sudo apt install gawk wget git diffstat unzip texinfo \
gcc build-essential chrpath socat cpio python3 python3-pip \
python3-pexpect xz-utils debianutils iputils-ping \
python3-git python3-jinja2 libsdl1.2-dev xterm \
zstd liblz4-tool
```

---

# 6. Clone Yocto Layers

---

# Poky

```bash id="7k2m8r"
git clone -b scarthgap git://git.yoctoproject.org/poky
```

---

# meta-xilinx

```bash id="5v1n7q"
git clone https://github.com/Xilinx/meta-xilinx
```

---

# meta-openembedded

```bash id="8x3r1m"
git clone -b scarthgap https://github.com/openembedded/meta-openembedded
```

---

# 7. Initialize Build Environment

```bash id="6q4m8w"
source poky/oe-init-build-env
```

---

# 8. Configure Layers

## `bblayers.conf`

```conf id="3n8x1v"
BBLAYERS += " \
../meta-xilinx/meta-xilinx-core \
../meta-xilinx/meta-xilinx-bsp \
../meta-openembedded/meta-oe \
"
```

---

# 9. Select Machine

Examples:

| Board    | MACHINE          |
| -------- | ---------------- |
| ZCU102   | `zcu102-zynqmp`  |
| KV260    | `kv260`          |
| ZedBoard | `zedboard-zynq7` |
| VCK190   | `vck190-versal`  |

Example:

```conf id="1m5x8q"
MACHINE = "zcu102-zynqmp"
```

---

# 10. FPGA Hardware Export

VERY IMPORTANT.

Vivado generates:

* XSA file

Example:

```text id="4x9m2r"
design.xsa
```

Contains:

* FPGA bitstream
* hardware description
* clocks
* peripherals

Linux build depends on this.

---

# 11. Build Images

Minimal image:

```bash id="7n1x4v"
bitbake core-image-minimal
```

PetaLinux-style image:

```bash id="5m2r8q"
bitbake petalinux-image-minimal
```

GUI image:

```bash id="2v7x9m"
bitbake core-image-weston
```

---

# 12. Build Outputs

Generated here:

```text id="9x4m2v"
tmp/deploy/images/zcu102-zynqmp/
```

Typical outputs:

```text id="3m8x1q"
BOOT.BIN
Image
system.dtb
rootfs.cpio.gz
core-image-minimal.wic
```

---

# 13. Xilinx Boot Architecture

Very important.

---

# Boot Flow

```text id="6x1m9q"
BootROM
   ↓
FSBL
   ↓
PMUFW
   ↓
Bitstream Load
   ↓
U-Boot
   ↓
Kernel
   ↓
Device Tree
   ↓
RootFS
   ↓
systemd
```

Key difference:

* FPGA bitstream can load during boot.

---

# 14. BOOT.BIN

Critical Xilinx concept.

Contains:

* FSBL
* PMUFW
* FPGA bitstream
* U-Boot

Generated using:

* `bootgen`

---

# 15. Flashing Methods

---

# Method 1 — SD Card Flashing

Most common development workflow.

---

## Flash Image

```bash id="8m2x5r"
sudo dd if=core-image-minimal.wic \
of=/dev/sdb \
bs=4M \
status=progress \
conv=fsync
```

---

# Method 2 — `bmaptool`

Recommended.

```bash id="1q8m4v"
sudo bmaptool copy image.wic /dev/sdb
```

---

# Method 3 — JTAG Flashing (VERY IMPORTANT)

Xilinx boards heavily use:

* JTAG programming

Tools:

* Vivado Hardware Manager
* XSCT

Useful for:

* bring-up
* debugging
* FPGA development

---

# Example XSCT Flash

```bash id="4v7m1x"
xsct
connect
targets
dow u-boot.elf
con
```

---

# Method 4 — QSPI Flashing

Production systems often boot from:

* QSPI NOR

Flash using:

* U-Boot
* XSCT
* bootgen workflows

---

# Method 5 — eMMC Flashing

Industrial systems commonly use:

* eMMC

Workflow:

1. boot from SD/JTAG
2. flash eMMC

---

# Method 6 — TFTP/NFS Boot

Very common for FPGA development.

In U-Boot:

```bash id="9m4x1r"
tftpboot ${kernel_addr_r} Image
booti ${kernel_addr_r} - ${fdt_addr_r}
```

Useful for:

* rapid kernel iteration
* CI systems

---

# 16. Verification Methods

Enterprise FPGA validation is extensive.

---

# Level 1 — UART Boot Verification

MOST IMPORTANT.

Monitor:

```bash id="6x9m2v"
picocom -b 115200 /dev/ttyUSB0
```

Verify:

* FSBL
* PMUFW
* bitstream load
* U-Boot
* kernel logs

---

# Level 2 — U-Boot Verification

Commands:

```bash id="3m1x8q"
printenv
```

```bash id="7v2m5r"
fpga info
```

```bash id="2x4m9v"
mmc list
```

Verify:

* FPGA loaded
* storage
* memory

---

# Level 3 — Linux Verification

Check:

```bash id="5q8m1x"
uname -a
```

```bash id="1v4m7r"
dmesg
```

```bash id="9x2m5q"
cat /etc/os-release
```

---

# Level 4 — FPGA Verification (VERY IMPORTANT)

Verify FPGA loaded correctly.

Check:

```bash id="4m7x1q"
dmesg | grep fpga
```

or:

```bash id="8q2m5v"
fpga_manager
```

Validate:

* bitstream loaded
* PL initialized

---

# Level 5 — Device Tree Verification

Critical in FPGA systems.

Check:

* custom IP blocks
* AXI peripherals
* interrupts

Example:

```bash id="1x5m8r"
ls /proc/device-tree/
```

---

# Level 6 — Hardware Accelerator Verification

Test:

* DMA
* AI accelerators
* video pipelines
* custom FPGA IP

Common tools:

* Vitis
* benchmark apps

---

# Level 7 — PCIe / Networking Validation

Many FPGA systems use:

* high-speed PCIe
* 10G/25G Ethernet

Validate:

* throughput
* latency

---

# Level 8 — AI Verification

For Versal/KV260:

* DPU acceleration
* Vitis AI

Validate:

* inference
* accelerator performance

---

# Level 9 — Thermal + Stress Testing

Use:

```bash id="7m2x5q"
stress-ng
```

Monitor:

* FPGA thermals
* DDR stability
* accelerator load

---

# 17. Secure Boot Verification

Very important in Xilinx systems.

Supports:

* authenticated boot
* encrypted bitstreams
* secure FPGA loading

Validate:

* signed BOOT.BIN
* encrypted PL bitstream

---

# 18. OTA Verification

Common frameworks:

* [RAUC](https://rauc.io/?utm_source=chatgpt.com)
* [Mender](https://mender.io/?utm_source=chatgpt.com)

Special challenge:

* updating FPGA bitstreams safely

Validate:

* rollback
* PL compatibility

---

# 19. CI/CD Validation Pipeline

Enterprise FPGA workflow:

```text id="5m8x2q"
Git Push
   ↓
Vivado Build
   ↓
XSA Export
   ↓
Yocto Build
   ↓
BOOT.BIN Generation
   ↓
Flash Board
   ↓
UART Capture
   ↓
FPGA Validation
   ↓
Accelerator Tests
   ↓
Artifact Publish
```

---

# 20. Factory Flashing Workflow

Manufacturing setup:

```text id="9q4m1x"
Factory PC
   ↓
JTAG/SD/eMMC Flash
   ↓
BOOT.BIN Install
   ↓
Automated Validation
```

---

# 21. Common Xilinx Development Issues

| Problem                | Cause              |
| ---------------------- | ------------------ |
| FPGA not loading       | bitstream mismatch |
| Kernel panic           | DTB mismatch       |
| AXI peripheral missing | device-tree issue  |
| Boot failure           | invalid BOOT.BIN   |
| DDR instability        | Vivado timing      |
| PL driver missing      | kernel config      |

---

# 22. Recommended Enterprise Xilinx Setup

| Area       | Recommendation |
| ---------- | -------------- |
| Build      | kas            |
| BSP        | meta-xilinx    |
| FPGA Tools | Vivado/Vitis   |
| OTA        | RAUC           |
| CI         | GitLab/GitHub  |
| Debug      | UART + JTAG    |
| Storage    | eMMC/NVMe      |
| Validation | labgrid/LAVA   |

---

# 23. Typical Enterprise Layer Structure

```text id="4q1m8x"
meta-company/
├── conf/
├── recipes-fpga/
├── recipes-accelerators/
├── recipes-networking/
├── recipes-security/
└── recipes-ai/
```

---

# 24. Industrial Best Practices

## DO:

* version FPGA + Linux together
* automate XSA export
* validate DTB consistency
* automate BOOT.BIN generation
* isolate FPGA recipes

## DON'T:

* manually patch generated DTBs
* mix FPGA logic into app layers
* ignore secure boot

---

# Useful References

* [meta-xilinx GitHub](https://github.com/Xilinx/meta-xilinx?utm_source=chatgpt.com)
* [AMD Xilinx PetaLinux Tools](https://www.xilinx.com/products/design-tools/embedded-software/petalinux-sdk.html?utm_source=chatgpt.com)
* [AMD Vivado Design Suite](https://www.xilinx.com/products/design-tools/vivado.html?utm_source=chatgpt.com)
* [AMD Vitis Unified Software Platform](https://www.xilinx.com/products/design-tools/vitis.html?utm_source=chatgpt.com)
* [Yocto Project Documentation](https://docs.yoctoproject.org/?utm_source=chatgpt.com)

