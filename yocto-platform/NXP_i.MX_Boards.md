# NXP i.MX Boards + Yocto Workflow

NXP i.MX boards are among the most widely used industrial Yocto platforms.

Typical use cases:

* industrial gateways
* medical devices
* automotive systems
* robotics
* HMI displays
* edge AI

Common SoCs:

* i.MX6
* i.MX7
* i.MX8M Mini
* i.MX8M Plus
* i.MX93

Common boards:

* i.MX 8M Plus EVK
* i.MX 8M Mini EVK
* Verdin iMX8M Plus
* Apalis iMX8

---

# 1. How NXP Uses Yocto

NXP officially supports Yocto through:

* `meta-freescale`
* `meta-imx`

These layers provide:

* BSP
* kernel
* U-Boot
* multimedia
* GPU acceleration
* AI/ISP support

Official BSP source:

* [NXP Yocto BSP Documentation](https://www.nxp.com/design/software/embedded-software/i-mx-software-and-development-tools/i-mx-yocto-project-bsp:I-MX-YP?utm_source=chatgpt.com)

---

# 2. Yocto Architecture for i.MX

Typical layer structure:

```text id="l0m3g7"
poky/
meta-openembedded/
meta-freescale/
meta-imx/
meta-company/
```

---

# 3. Main BSP Layers

| Layer               | Purpose                 |
| ------------------- | ----------------------- |
| `meta-freescale`    | Community BSP           |
| `meta-imx`          | NXP official extensions |
| `meta-openembedded` | Extra packages          |
| `poky`              | Yocto core              |

---

# 4. Build Environment Setup

---

# Install Host Dependencies

Ubuntu example:

```bash id="8bq7c8"
sudo apt install gawk wget git diffstat unzip texinfo \
gcc build-essential chrpath socat cpio python3 python3-pip \
python3-pexpect xz-utils debianutils iputils-ping \
python3-git python3-jinja2 libsdl1.2-dev xterm \
zstd liblz4-tool
```

---

# 5. Download NXP Yocto BSP

NXP provides:

* `imx-setup-release.sh`
* repo manifests
* BSP integration

Example:

```bash id="7vny44"
repo init -u https://github.com/nxp-imx/imx-manifest \
-b imx-linux-scarthgap \
-m imx-6.6.3-1.0.0.xml
```

Then:

```bash id="jdx4si"
repo sync
```

This pulls:

* poky
* meta-imx
* meta-freescale
* required dependencies

---

# 6. Setup Build Environment

```bash id="n7j2z5"
DISTRO=fsl-imx-wayland MACHINE=imx8mpevk \
source imx-setup-release.sh -b build-wayland
```

This creates:

```text id="ydceq0"
build-wayland/
├── conf/
│   ├── local.conf
│   └── bblayers.conf
```

---

# 7. Select Machine

Examples:

| Board       | MACHINE     |
| ----------- | ----------- |
| i.MX8MP EVK | `imx8mpevk` |
| i.MX8MM EVK | `imx8mmevk` |
| i.MX93 EVK  | `imx93evk`  |

Example:

```conf id="p3gbtv"
MACHINE = "imx8mpevk"
```

---

# 8. Build Images

Minimal:

```bash id="hjmow5"
bitbake core-image-minimal
```

GUI image:

```bash id="o1i9dy"
bitbake imx-image-full
```

Multimedia image:

```bash id="t9ivk9"
bitbake imx-image-multimedia
```

---

# 9. Build Outputs

Generated here:

```text id="1yzrhv"
tmp/deploy/images/imx8mpevk/
```

Typical files:

```text id="s08gzm"
imx-image-full-imx8mpevk.rootfs.wic.zst
Image
imx8mp-evk.dtb
imx-boot
u-boot.bin
```

---

# 10. i.MX Boot Architecture

Understanding this is critical.

---

# Boot Flow

```text id="p7z2eq"
ROM Boot
   ↓
SPL
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

NXP SoCs have:

* internal ROM bootloader
* secure boot support
* eMMC/SD/NAND/QSPI boot

---

# 11. Flashing Methods

Industrial i.MX systems usually boot from:

* SD card
* eMMC
* NAND
* NVMe
* QSPI NOR

---

# Method 1 — SD Card Flashing

Most common for EVKs.

---

## Decompress Image

```bash id="d7e2ic"
unzstd imx-image-full-imx8mpevk.rootfs.wic.zst
```

---

## Flash Using `dd`

```bash id="n8q77t"
sudo dd if=imx-image-full-imx8mpevk.rootfs.wic \
of=/dev/sdb \
bs=4M \
status=progress \
conv=fsync
```

---

# Method 2 — `bmaptool` (Recommended)

Install:

```bash id="mrrmlt"
sudo apt install bmap-tools
```

Flash:

```bash id="x9c0y8"
sudo bmaptool copy \
imx-image-full-imx8mpevk.rootfs.wic.zst \
/dev/sdb
```

Advantages:

* sparse flashing
* faster
* integrity verification

---

# Method 3 — UUU Tool (Most Important for i.MX)

NXP production flashing tool.

Called:

* Universal Update Utility (UUU)

Official:

* [NXP UUU Tool](https://github.com/nxp-imx/mfgtools?utm_source=chatgpt.com)

---

# How UUU Works

Board enters:

* USB recovery mode

Host PC flashes:

* bootloader
* kernel
* rootfs

via USB OTG.

---

# Example UUU Flash

```bash id="6w4v8r"
sudo ./uuu flash.bin
```

or:

```bash id="2wn0k4"
sudo ./uuu -b emmc_all imx-image-full.rootfs.wic
```

This is heavily used in:

* factories
* manufacturing
* mass production

---

# Method 4 — Fastboot

Some i.MX BSPs support Android-style fastboot.

Example:

```bash id="zov6e1"
fastboot flash boot boot.img
```

---

# Method 5 — Network Flashing

Industrial systems may use:

* TFTP
* NFS boot
* PXE-like workflows

Especially useful for:

* development labs
* CI automation

---

# 12. Verification Methods

This is where enterprise embedded validation becomes important.

---

# Level 1 — UART Boot Verification

MOST IMPORTANT.

Use:

* USB-UART adapter

Typical baudrate:

```text id="6tlf9g"
115200 8N1
```

Monitor:

```bash id="w3dnyv"
picocom -b 115200 /dev/ttyUSB0
```

Check:

* SPL boot
* U-Boot
* kernel logs

---

# Level 2 — U-Boot Verification

Interrupt boot:

```text id="6pkcv8"
Hit any key to stop autoboot
```

Commands:

```bash id="i89z9j"
printenv
```

```bash id="v61qse"
mmc list
```

```bash id="gmjlwm"
bdinfo
```

Verify:

* RAM
* boot source
* partitions

---

# Level 3 — Linux Boot Verification

Check:

```bash id="apz9nq"
uname -a
```

```bash id="tkh3fu"
cat /etc/os-release
```

```bash id="7ix2hi"
dmesg
```

---

# Level 4 — Peripheral Validation

Test:

* Ethernet
* WiFi
* Bluetooth
* CAN
* USB
* HDMI
* LVDS
* MIPI CSI camera
* GPIO
* SPI
* I2C

Examples:

```bash id="r9p5l2"
ip a
```

```bash id="40wlmf"
i2cdetect -y 1
```

```bash id="n9x8vx"
gst-launch-1.0
```

---

# Level 5 — Multimedia Verification

Very important on i.MX8.

Test:

* VPU
* GPU
* ISP
* camera pipelines

Common tools:

* GStreamer
* Weston
* OpenGL ES

---

# Level 6 — AI Verification

For i.MX8MP:

* NPU support

Validate:

* TensorFlow Lite
* NNStreamer

---

# Level 7 — Storage Validation

Check:

* eMMC
* SD
* NVMe

Example:

```bash id="5t38gk"
lsblk
```

Benchmark:

```bash id="p3a0u8"
fio
```

---

# Level 8 — Thermal + Stress Testing

Use:

```bash id="yc0k7n"
stress-ng
```

Monitor:

```bash id="7slzhv"
cat /sys/class/thermal/thermal_zone0/temp
```

Industrial systems require:

* thermal stability
* long-duration validation

---

# 13. Secure Boot Verification

NXP platforms commonly use:

* HAB (High Assurance Boot)
* AHAB (Advanced HAB)

Verify:

* signed bootloader
* signed kernel

Critical in:

* automotive
* medical
* industrial

---

# 14. OTA Verification

Common frameworks:

* [RAUC](https://rauc.io/?utm_source=chatgpt.com)
* [Mender](https://mender.io/?utm_source=chatgpt.com)

Verify:

* rollback
* A/B boot
* update integrity

---

# 15. CI/CD Validation Pipeline

Enterprise workflow:

```text id="0wn4kh"
Git Push
   ↓
Yocto Build
   ↓
Static Analysis
   ↓
QEMU Boot
   ↓
UUU Flash
   ↓
UART Capture
   ↓
SSH Tests
   ↓
Peripheral Tests
   ↓
Artifact Publish
```

---

# 16. Factory Flashing Workflow

Production systems usually use:

```text id="3n3zwm"
Factory PC
   ↓ USB OTG
i.MX Board Recovery Mode
   ↓
UUU Flash
   ↓
Automated Verification
```

---

# 17. Common Industrial Storage Choices

| Storage | Usage                    |
| ------- | ------------------------ |
| SD Card | Development              |
| eMMC    | Production               |
| NAND    | Legacy systems           |
| NVMe    | High-performance edge AI |

---

# 18. Recommended Enterprise i.MX Setup

| Area       | Recommendation |
| ---------- | -------------- |
| Boot       | eMMC           |
| OTA        | RAUC           |
| Debug      | UART always    |
| Build      | kas            |
| CI         | GitLab/GitHub  |
| Flashing   | UUU            |
| Security   | HAB/AHAB       |
| Containers | Podman         |

---

# 19. Typical Enterprise Folder Structure

```text id="f5e7o9"
meta-company/
├── conf/
├── recipes-core/
├── recipes-kernel/
├── recipes-security/
├── recipes-multimedia/
└── recipes-connectivity/
```

---

# 20. Real Industrial Best Practices

## DO:

* isolate BSP changes
* maintain custom layer
* automate flashing
* automate UART capture
* use reproducible builds

## DON'T:

* modify vendor layers directly
* hardcode board assumptions
* mix product logic with BSP

---

# Useful References

* [NXP Yocto BSP Documentation](https://www.nxp.com/design/software/embedded-software/i-mx-software-and-development-tools/i-mx-yocto-project-bsp:I-MX-YP?utm_source=chatgpt.com)
* [meta-freescale GitHub](https://github.com/Freescale/meta-freescale?utm_source=chatgpt.com)
* [meta-imx GitHub](https://github.com/nxp-imx/meta-imx?utm_source=chatgpt.com)
* [NXP UUU Tool](https://github.com/nxp-imx/mfgtools?utm_source=chatgpt.com)
* [Yocto Project Documentation](https://docs.yoctoproject.org/?utm_source=chatgpt.com)

