# RISC-V Boards + Yocto Workflow

RISC-V platforms are rapidly growing in:

* open hardware systems
* industrial controllers
* edge AI
* academic/research systems
* low-power embedded devices
* Linux SBC ecosystems

Unlike ARM vendors, RISC-V is an ISA ecosystem with many vendors.

Popular RISC-V SoCs:

* StarFive JH7110
* SiFive FU740
* Allwinner D1
* Sophgo CV1800
* SpacemiT K1

Popular boards:

* VisionFive 2
* HiFive Unmatched
* LicheeRV
* Milk-V Duo
* Banana Pi BPI-F3

---

# 1. How RISC-V Uses Yocto

Yocto support for RISC-V comes through:

* upstream Yocto support
* vendor BSP layers
* OpenEmbedded ecosystem

Main layers:

* `meta-riscv`
* vendor-specific BSP layers

Official/open ecosystem:

* OpenSBI
* U-Boot
* Linux kernel

Yocto support:

* [Yocto Project RISC-V Support](https://docs.yoctoproject.org/dev/dev-manual/qemu.html?utm_source=chatgpt.com#qemu-risc-v-32-and-qemu-risc-v-64-configurations)

---

# 2. Important RISC-V Concept

RISC-V systems are highly fragmented.

Each vendor may have:

* custom boot flow
* custom firmware
* different kernel maturity
* vendor BSP patches

Compared to ARM:

* ecosystem is less standardized
* upstreaming matters much more

---

# 3. Typical Yocto Layer Structure

```text id="1m9x4q"
poky/
meta-openembedded/
meta-riscv/
meta-sifive/
meta-starfive/
meta-company/
```

---

# 4. Main BSP Components

| Component       | Purpose                     |
| --------------- | --------------------------- |
| OpenSBI         | Supervisor firmware         |
| U-Boot          | Bootloader                  |
| Linux kernel    | BSP                         |
| Device Tree     | Hardware description        |
| Vendor firmware | SoC-specific initialization |

---

# 5. Build Environment Setup

---

# Install Host Dependencies

Ubuntu example:

```bash id="7x2m5q"
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

```bash id="4m1x8q"
git clone -b scarthgap git://git.yoctoproject.org/poky
```

---

# meta-openembedded

```bash id="9q2m5x"
git clone -b scarthgap https://github.com/openembedded/meta-openembedded
```

---

# Example Vendor Layer

Example:

```bash id="6m4x1v"
git clone https://github.com/starfive-tech/meta-starfive
```

---

# 7. Initialize Build Environment

```bash id="2x7m9q"
source poky/oe-init-build-env
```

---

# 8. Configure Layers

## `bblayers.conf`

```conf id="8m1x5r"
BBLAYERS += " \
../meta-openembedded/meta-oe \
../meta-starfive \
"
```

---

# 9. Select Machine

Examples:

| Board            | MACHINE                |
| ---------------- | ---------------------- |
| VisionFive 2     | `starfive-visionfive2` |
| HiFive Unmatched | `hifive-unmatched`     |

Example:

```conf id="5x2m8q"
MACHINE = "starfive-visionfive2"
```

---

# 10. Build Images

Minimal image:

```bash id="1m7x4q"
bitbake core-image-minimal
```

GUI image:

```bash id="9x1m5v"
bitbake core-image-weston
```

Developer image:

```bash id="4q8m2x"
bitbake core-image-full-cmdline
```

---

# 11. Build Outputs

Generated here:

```text id="7m2x1q"
tmp/deploy/images/starfive-visionfive2/
```

Typical outputs:

```text id="3x8m4q"
core-image-minimal.wic
u-boot.bin
fw_payload.bin
Image
jh7110-starfive-visionfive-2.dtb
```

---

# 12. RISC-V Boot Architecture

VERY IMPORTANT.

---

# Typical Boot Flow

```text id="6m1x9q"
BootROM
   ↓
OpenSBI
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

Key component:

* OpenSBI

Equivalent to:

* ARM Trusted Firmware on ARM systems

---

# 13. Storage Options

RISC-V systems commonly boot from:

* SD card
* eMMC
* SPI NOR
* NVMe

Most development boards primarily use:

* SD card

Industrial systems increasingly use:

* eMMC/NVMe

---

# 14. Flashing Methods

---

# Method 1 — SD Card Flashing

Most common workflow.

---

## Flash Using `dd`

```bash id="2m4x7q"
sudo dd if=core-image-minimal.wic \
of=/dev/sdb \
bs=4M \
status=progress \
conv=fsync
```

---

# Method 2 — `bmaptool`

Recommended.

```bash id="8q1m5x"
sudo bmaptool copy image.wic /dev/sdb
```

---

# Method 3 — SPI NOR Flashing

Some systems boot from:

* SPI NOR

Using:

* flashcp
* U-Boot
* vendor tools

Example:

```bash id="4m9x2q"
flashcp u-boot.bin /dev/mtd0
```

---

# Method 4 — U-Boot Network Flashing

Common for development.

In U-Boot:

```bash id="7x1m4v"
tftpboot ${kernel_addr_r} Image
booti ${kernel_addr_r} - ${fdt_addr_r}
```

Useful for:

* rapid development
* CI systems

---

# Method 5 — eMMC Installation

Production systems:

1. boot from SD
2. flash eMMC

Example:

```bash id="1q8m5x"
dd if=image.wic of=/dev/mmcblk0
```

---

# 15. Verification Methods

Enterprise validation becomes critical because:

* vendor BSP quality varies
* upstream support maturity differs

---

# Level 1 — UART Boot Verification

MOST IMPORTANT.

Use:

* USB-UART adapter

Common baudrates:

* 115200
* 1500000

Monitor:

```bash id="5m2x9q"
picocom -b 115200 /dev/ttyUSB0
```

Verify:

* OpenSBI
* U-Boot
* kernel logs

---

# Level 2 — OpenSBI Verification

VERY IMPORTANT.

Check boot logs:

```text id="9m4x1r"
OpenSBI v1.x
```

Validate:

* hart initialization
* SBI extensions
* timer setup

---

# Level 3 — U-Boot Verification

Commands:

```bash id="3x7m2q"
printenv
```

```bash id="8m1x5v"
mmc list
```

Verify:

* storage
* RAM
* DTB loading

---

# Level 4 — Linux Verification

Check:

```bash id="2q9m4x"
uname -a
```

```bash id="6m1x7q"
dmesg
```

```bash id="1x5m8r"
cat /etc/os-release
```

---

# Level 5 — Peripheral Validation

Test:

* Ethernet
* USB
* I2C
* SPI
* GPIO
* UART

Examples:

```bash id="9q2m4v"
ip a
```

```bash id="4m7x1q"
i2cdetect -y 1
```

---

# Level 6 — Graphics Verification

Some modern RISC-V boards support:

* GPU acceleration
* DRM/KMS
* Wayland

Example:

```bash id="7m1x8q"
weston
```

---

# Level 7 — AI/NPU Verification

Emerging RISC-V SoCs include:

* AI accelerators
* NPUs

Validate:

* inference
* accelerator runtime

---

# Level 8 — Storage Verification

Check:

* SD/eMMC reliability
* NVMe support

Example:

```bash id="1m4x9v"
lsblk
```

Benchmark:

```bash id="5x2m7q"
fio
```

---

# Level 9 — Thermal + Stress Testing

Use:

```bash id="8q1m4x"
stress-ng
```

Monitor:

```bash id="3m7x2q"
cat /sys/class/thermal/thermal_zone0/temp
```

Important because:

* many RISC-V boards are early-stage hardware

---

# 16. Secure Boot Verification

RISC-V secure boot is evolving.

Some platforms support:

* verified boot
* signed firmware
* TPM integration

Validate:

* firmware signatures
* boot chain integrity

---

# 17. OTA Verification

Common frameworks:

* [RAUC](https://rauc.io/?utm_source=chatgpt.com)
* [Mender](https://mender.io/?utm_source=chatgpt.com)

Validate:

* rollback
* image integrity
* partition switching

---

# 18. CI/CD Validation Pipeline

Enterprise RISC-V workflow:

```text id="4x8m1q"
Git Push
   ↓
Yocto Build
   ↓
Static Validation
   ↓
Flash Board
   ↓
UART Capture
   ↓
Peripheral Tests
   ↓
Stress Testing
   ↓
OTA Validation
   ↓
Artifact Publish
```

---

# 19. Factory Flashing Workflow

Manufacturing setup:

```text id="9m1x5q"
Factory PC
   ↓
SD/eMMC/SPI Flash
   ↓
Automated Validation
```

---

# 20. Common RISC-V Development Issues

| Problem             | Cause                       |
| ------------------- | --------------------------- |
| Boot failure        | OpenSBI mismatch            |
| Kernel panic        | DTB incompatibility         |
| Missing peripherals | immature BSP                |
| GPU unavailable     | upstream support incomplete |
| NVMe issues         | PCIe instability            |
| SD corruption       | power problems              |

---

# 21. Recommended Enterprise RISC-V Setup

| Area       | Recommendation                |
| ---------- | ----------------------------- |
| Build      | kas                           |
| BSP        | vendor layer + upstream Yocto |
| OTA        | RAUC                          |
| Storage    | eMMC/NVMe                     |
| CI         | GitLab/GitHub                 |
| Debug      | UART always enabled           |
| Validation | labgrid/LAVA                  |

---

# 22. Typical Enterprise Layer Structure

```text id="6q2m8x"
meta-company/
├── conf/
├── recipes-security/
├── recipes-connectivity/
├── recipes-graphics/
├── recipes-industrial/
└── recipes-ai/
```

---

# 23. Industrial Best Practices

## DO:

* prioritize upstream support
* validate OpenSBI carefully
* automate UART capture
* isolate vendor BSP patches
* stress-test aggressively

## DON'T:

* rely only on vendor kernels
* ignore power stability
* assume ARM workflows directly apply

---

# 24. Why RISC-V Matters for Yocto

RISC-V is becoming important because:

* open ISA
* vendor independence
* growing Linux support
* strong upstream community

Long term:

* industrial adoption will increase significantly

---

# Useful References

* [Yocto Project Documentation](https://docs.yoctoproject.org/?utm_source=chatgpt.com)
* [OpenSBI GitHub](https://github.com/riscv-software-src/opensbi?utm_source=chatgpt.com)
* [U-Boot Project](https://www.denx.de/wiki/U-Boot?utm_source=chatgpt.com)
* [StarFive meta-starfive Layer](https://github.com/starfive-tech/meta-starfive?utm_source=chatgpt.com)

