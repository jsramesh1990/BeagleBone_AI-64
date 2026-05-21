# STM32 MPU Boards + Yocto Workflow

STM32 MPU platforms from STMicroelectronics are widely used in:

* industrial HMI
* low-power edge devices
* secure embedded systems
* medical devices
* gateways
* automation systems

Main MPU family:

* STM32MP1
* STM32MP13

Popular boards:

* STM32MP157C-DK2
* STM32MP135F-DK
* STM32MP157F-EV1

---

# 1. How STM32 Uses Yocto

ST officially supports Yocto through:

* `meta-st-stm32mp`

Integrated into:

* OpenSTLinux distribution

Official ecosystem:

* STM32CubeMX
* OpenSTLinux
* Trusted Firmware-A
* OP-TEE

Official docs:

* [STM32 MPU OpenSTLinux Distribution](https://www.st.com/en/embedded-software/stm32mpu-embedded-software.html?utm_source=chatgpt.com)

---

# 2. Typical Yocto Layer Structure

```text id="u4j8n2"
poky/
meta-openembedded/
meta-st-stm32mp/
meta-arm/
meta-security/
meta-company/
```

---

# 3. Main BSP Components

| Component         | Purpose            |
| ----------------- | ------------------ |
| `meta-st-stm32mp` | STM32 MPU BSP      |
| TF-A              | Trusted Firmware-A |
| OP-TEE            | Trusted execution  |
| U-Boot            | Bootloader         |
| Linux kernel      | BSP kernel         |

---

# 4. STM32MP Architecture

STM32MP is unique because it combines:

* Cortex-A Linux cores
* Cortex-M microcontroller core

Example:

```text id="q9m3x1"
Cortex-A7 → Linux
Cortex-M4 → RTOS/baremetal
```

Very common in:

* industrial systems
* deterministic control applications

---

# 5. Build Environment Setup

---

# Install Host Dependencies

Ubuntu example:

```bash id="x5n1r8"
sudo apt install gawk wget git diffstat unzip texinfo \
gcc build-essential chrpath socat cpio python3 python3-pip \
python3-pexpect xz-utils debianutils iputils-ping \
python3-git python3-jinja2 libsdl1.2-dev xterm \
zstd liblz4-tool
```

---

# 6. Download STM32 Yocto BSP

ST provides:

* repo manifests
* setup scripts
* OpenSTLinux

Example:

```bash id="m2v7x4"
repo init -u https://github.com/STMicroelectronics/oe-manifest \
-b refs/tags/openstlinux-6.6-yocto-scarthgap-mpu-v24.06.26
```

Then:

```bash id="p8q1m5"
repo sync
```

---

# 7. Initialize Build Environment

```bash id="r4x9m2"
DISTRO=openstlinux-weston \
MACHINE=stm32mp1-disco \
source layers/meta-st/scripts/envsetup.sh
```

Creates:

```text id="n7m1x5"
build-openstlinuxweston-stm32mp1-disco/
```

---

# 8. Select Machine

Examples:

| Board          | MACHINE           |
| -------------- | ----------------- |
| STM32MP157 DK2 | `stm32mp1-disco`  |
| STM32MP135 DK  | `stm32mp13-disco` |
| EV1 Board      | `stm32mp15-eval`  |

Example:

```conf id="6m2x9q"
MACHINE = "stm32mp1-disco"
```

---

# 9. Build Images

Minimal image:

```bash id="3x8m1v"
bitbake core-image-minimal
```

OpenSTLinux image:

```bash id="9m4x2r"
bitbake st-image-weston
```

Qt image:

```bash id="5q1m8x"
bitbake st-image-qt
```

---

# 10. Build Outputs

Generated here:

```text id="1m7x5q"
tmp-glibc/deploy/images/stm32mp1-disco/
```

Typical outputs:

```text id="4x2m9v"
st-image-weston-openstlinux-weston-stm32mp1-disco.wic.gz
tf-a-stm32mp157c.bin
u-boot.stm32
Image
stm32mp157c-dk2.dtb
```

---

# 11. STM32MP Boot Architecture

VERY IMPORTANT.

---

# Boot Flow

```text id="8m1x4q"
BootROM
   ↓
TF-A
   ↓
OP-TEE
   ↓
U-Boot
   ↓
Linux Kernel
   ↓
Device Tree
   ↓
RootFS
   ↓
systemd
```

STM32MP platforms commonly use:

* secure boot
* trusted execution
* OP-TEE

---

# 12. Storage Options

STM32 MPU systems commonly boot from:

* SD card
* eMMC
* NAND
* NOR flash

Industrial systems usually prefer:

* eMMC

---

# 13. Flashing Methods

---

# Method 1 — SD Card Flashing

Most common development workflow.

---

## Decompress

```bash id="7q2m5x"
gunzip image.wic.gz
```

---

## Flash Using `dd`

```bash id="2m8x1v"
sudo dd if=image.wic \
of=/dev/sdb \
bs=4M \
status=progress \
conv=fsync
```

---

# Method 2 — `bmaptool` (Recommended)

```bash id="5x1m7q"
sudo bmaptool copy image.wic.gz /dev/sdb
```

Benefits:

* sparse flashing
* integrity verification
* faster writes

---

# Method 3 — STM32CubeProgrammer (VERY IMPORTANT)

Main STM32 flashing tool.

Official tool:

* [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html?utm_source=chatgpt.com)

Supports:

* USB flashing
* UART flashing
* JTAG/SWD
* eMMC/NAND programming

---

# USB Recovery Flashing

Board enters:

* USB DFU mode

Flash using GUI or CLI.

Example:

```bash id="9x4m1r"
STM32_Programmer_CLI -c port=usb1 \
-w flashlayout.tsv
```

This is heavily used in:

* manufacturing
* board recovery
* production flashing

---

# Method 4 — U-Boot Flashing

Industrial systems often:

1. boot from SD
2. flash eMMC

Example:

```bash id="1v8m2x"
mmc write
```

---

# Method 5 — Network Boot

STM32 supports:

* TFTP
* NFS rootfs

Useful for:

* development
* CI systems

---

# 14. Flash Layout Concept

STM32 uses:

* `FlashLayout.tsv`

Defines:

* TF-A
* OP-TEE
* U-Boot
* kernel
* rootfs partitions

VERY important for production.

---

# 15. Verification Methods

Enterprise embedded validation becomes important here.

---

# Level 1 — UART Boot Verification

MOST IMPORTANT.

Use:

* USB-UART adapter

Typical baudrate:

```text id="4m9x1q"
115200 8N1
```

Monitor:

```bash id="8x2m5v"
picocom -b 115200 /dev/ttyUSB0
```

Verify:

* TF-A
* OP-TEE
* U-Boot
* kernel logs

---

# Level 2 — U-Boot Verification

Commands:

```bash id="2q7m1x"
printenv
```

```bash id="5m8x4r"
mmc list
```

Verify:

* boot source
* storage
* environment

---

# Level 3 — Linux Verification

Check:

```bash id="1x4m8q"
uname -a
```

```bash id="7m2x5v"
dmesg
```

```bash id="9q1m4x"
cat /etc/os-release
```

---

# Level 4 — Peripheral Validation

Test:

* Ethernet
* USB
* SPI
* I2C
* CAN
* UART
* GPIO
* display

Examples:

```bash id="3m7x1q"
ip a
```

```bash id="6x2m8v"
i2cdetect -y 1
```

---

# Level 5 — Display/GUI Validation

STM32MP boards are often GUI-oriented.

Test:

* Weston
* DRM/KMS
* touchscreen
* LVDS/MIPI display

Example:

```bash id="8m5x1r"
weston
```

---

# Level 6 — Cortex-M4 Verification

VERY IMPORTANT.

STM32MP supports:

* remoteproc
* RPMsg

Validate:

* M4 firmware loading
* interprocessor communication

Example:

```bash id="2x9m4v"
echo firmware.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state
```

---

# Level 7 — OP-TEE Verification

Verify secure world:

```bash id="5v1m8x"
dmesg | grep optee
```

Important for:

* secure storage
* crypto
* trusted execution

---

# Level 8 — Storage Verification

Check:

* eMMC
* NAND
* SD

Example:

```bash id="1m4x9q"
lsblk
```

Benchmark:

```bash id="7x2m5v"
fio
```

---

# Level 9 — Thermal + Stress Testing

Use:

```bash id="3q8m1x"
stress-ng
```

Monitor:

```bash id="6m5x2v"
cat /sys/class/thermal/thermal_zone0/temp
```

Industrial systems require:

* long-duration stability
* power validation

---

# 16. Secure Boot Verification

STM32MP strongly focuses on:

* secure boot
* trusted firmware
* OP-TEE

Validate:

* signed boot chain
* secure storage
* trusted OS

---

# 17. OTA Verification

Common frameworks:

* [RAUC](https://rauc.io/?utm_source=chatgpt.com)
* [Mender](https://mender.io/?utm_source=chatgpt.com)

Validate:

* rollback
* A/B updates
* partition switching

---

# 18. CI/CD Validation Pipeline

Enterprise STM32 workflow:

```text id="9m2x5q"
Git Push
   ↓
Yocto Build
   ↓
Static Validation
   ↓
STM32CubeProgrammer Flash
   ↓
UART Capture
   ↓
Peripheral Tests
   ↓
M4 Firmware Tests
   ↓
OP-TEE Validation
   ↓
Artifact Publish
```

---

# 19. Factory Flashing Workflow

Manufacturing setup:

```text id="4x1m8q"
Factory PC
   ↓ USB DFU
STM32CubeProgrammer
   ↓
Flash eMMC/NAND
   ↓
Automated Validation
```

---

# 20. Common STM32MP Development Issues

| Problem                 | Cause                 |
| ----------------------- | --------------------- |
| Boot failure            | TF-A mismatch         |
| OP-TEE failure          | secure config issue   |
| Display not working     | DTB/display timing    |
| M4 firmware not loading | remoteproc config     |
| USB DFU not detected    | boot mode issue       |
| NAND problems           | flash layout mismatch |

---

# 21. Recommended Enterprise STM32 Setup

| Area       | Recommendation      |
| ---------- | ------------------- |
| Build      | kas                 |
| BSP        | meta-st-stm32mp     |
| Security   | OP-TEE              |
| OTA        | RAUC                |
| Storage    | eMMC                |
| CI         | GitLab/GitHub       |
| Debug      | UART always enabled |
| Validation | labgrid/LAVA        |

---

# 22. Typical Enterprise Layer Structure

```text id="7m1x4q"
meta-company/
├── conf/
├── recipes-security/
├── recipes-m4/
├── recipes-connectivity/
├── recipes-gui/
└── recipes-industrial/
```

---

# 23. Industrial Best Practices

## DO:

* isolate BSP changes
* automate DFU flashing
* validate OP-TEE
* separate M4 firmware from Linux apps
* use eMMC for production

## DON'T:

* modify vendor BSP directly
* hardcode flash layouts
* ignore secure boot testing

---

# Useful References

* [STM32 MPU OpenSTLinux Distribution](https://www.st.com/en/embedded-software/stm32mpu-embedded-software.html?utm_source=chatgpt.com)
* [meta-st-stm32mp GitHub](https://github.com/STMicroelectronics/meta-st-stm32mp?utm_source=chatgpt.com)
* [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html?utm_source=chatgpt.com)
* [Yocto Project Documentation](https://docs.yoctoproject.org/?utm_source=chatgpt.com)

