# Rockchip Boards + Yocto Workflow

Rockchip boards are increasingly popular for:

* multimedia systems
* edge AI
* ARM desktops
* kiosks
* smart displays
* low-cost industrial systems

Popular Rockchip SoCs:

* RK3399
* RK3566
* RK3568
* RK3588

Popular boards:

* Rock 5B
* Orange Pi 5
* Quartz64
* ROCK 3A

---

# 1. How Rockchip Uses Yocto

Yocto support for Rockchip is mainly through:

* `meta-rockchip`

Official/open BSP ecosystem:

* Rockchip Linux SDK
* community BSP layers
* upstream kernels

Main layer:

* [meta-rockchip GitHub](https://github.com/JeffyCN/meta-rockchip?utm_source=chatgpt.com)

---

# 2. Typical Yocto Layer Structure

```text id="e8jqk3"
poky/
meta-openembedded/
meta-rockchip/
meta-python/
meta-networking/
meta-company/
```

---

# 3. Main BSP Components

| Component       | Purpose                 |
| --------------- | ----------------------- |
| `meta-rockchip` | BSP integration         |
| U-Boot          | Bootloader              |
| Linux kernel    | Board support           |
| MPP             | Multimedia acceleration |
| Mali GPU        | Graphics acceleration   |

---

# 4. Build Environment Setup

---

# Install Host Dependencies

Ubuntu example:

```bash id="hz71ow"
sudo apt install gawk wget git diffstat unzip texinfo \
gcc build-essential chrpath socat cpio python3 python3-pip \
python3-pexpect xz-utils debianutils iputils-ping \
python3-git python3-jinja2 libsdl1.2-dev xterm \
zstd liblz4-tool
```

---

# 5. Clone Yocto Layers

---

# Poky

```bash id="n0sx6w"
git clone -b scarthgap git://git.yoctoproject.org/poky
```

---

# meta-rockchip

```bash id="6v0r7g"
git clone https://github.com/JeffyCN/meta-rockchip
```

---

# meta-openembedded

```bash id="7ql4tr"
git clone -b scarthgap https://github.com/openembedded/meta-openembedded
```

---

# 6. Initialize Build Environment

```bash id="m1f2sw"
source poky/oe-init-build-env
```

---

# 7. Configure Layers

## `bblayers.conf`

```conf id="1txlkl"
BBLAYERS += " \
../meta-rockchip \
../meta-openembedded/meta-oe \
../meta-openembedded/meta-python \
../meta-openembedded/meta-networking \
"
```

---

# 8. Select Machine

Examples:

| Board       | MACHINE              |
| ----------- | -------------------- |
| Rock 5B     | `rock-5b-rk3588`     |
| Orange Pi 5 | `orangepi-5-rk3588s` |
| Quartz64    | `quartz64-a`         |

Example:

```conf id="xq5m0l"
MACHINE = "rock-5b-rk3588"
```

---

# 9. Build Images

Minimal image:

```bash id="n9f0m4"
bitbake core-image-minimal
```

Weston/GUI image:

```bash id="9b2z0x"
bitbake core-image-weston
```

Multimedia image:

```bash id="q4s9h8"
bitbake core-image-full-cmdline
```

---

# 10. Build Outputs

Generated here:

```text id="m7l2wd"
tmp/deploy/images/rock-5b-rk3588/
```

Typical files:

```text id="x4r8nt"
core-image-minimal.rootfs.wic
Image
u-boot.img
idbloader.img
rk3588-rock-5b.dtb
```

---

# 11. Rockchip Boot Architecture

Understanding Rockchip boot flow is important.

---

# Boot Flow

```text id="3l4r1m"
BootROM
   ↓
idbloader
   ↓
U-Boot SPL
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

Rockchip platforms often use:

* SPL
* ATF (ARM Trusted Firmware)
* U-Boot

---

# 12. Storage Options

Rockchip boards commonly boot from:

* SD card
* eMMC
* NVMe SSD
* SPI NOR
* USB

RK3588 systems often use:

* NVMe for production

---

# 13. Flashing Methods

---

# Method 1 — SD Card Flashing

Most common development workflow.

---

## Flash Using `dd`

```bash id="f7w4qp"
sudo dd if=core-image-minimal.rootfs.wic \
of=/dev/sdb \
bs=4M \
status=progress \
conv=fsync
```

---

# Method 2 — `bmaptool` (Recommended)

Install:

```bash id="9f0l2s"
sudo apt install bmap-tools
```

Flash:

```bash id="n6m5q8"
sudo bmaptool copy \
core-image-minimal.rootfs.wic \
/dev/sdb
```

Benefits:

* sparse flashing
* integrity checks
* faster writes

---

# Method 3 — Rockchip USB Loader Mode

VERY IMPORTANT.

Rockchip SoCs support:

* USB recovery mode
* maskrom mode

Used heavily for:

* production flashing
* board recovery

---

# Enter MaskROM Mode

Typical process:

1. hold recovery button
2. power/reset board
3. connect USB OTG

Verify:

```bash id="8v5k2y"
lsusb
```

---

# Method 4 — `rkdeveloptool`

Main Rockchip flashing utility.

Official/open tool:

* [rkdeveloptool GitHub](https://github.com/rockchip-linux/rkdeveloptool?utm_source=chatgpt.com)

---

# Flash Loader

```bash id="0q8m3j"
rkdeveloptool db loader.bin
```

---

# Flash Image

```bash id="1w6s7h"
rkdeveloptool wl 0 image.img
```

Reboot:

```bash id="x9c4e7"
rkdeveloptool rd
```

This is the most common professional flashing workflow.

---

# Method 5 — eMMC Flashing

Production systems usually:

1. boot from SD
2. flash eMMC

Example:

```bash id="3n5x1f"
dd if=image.wic of=/dev/mmcblk0
```

---

# Method 6 — NVMe Installation

Common on RK3588 boards.

Install rootfs onto:

* NVMe SSD

Advantages:

* faster boot
* better reliability
* AI workloads

---

# Method 7 — Network Boot

Advanced setups may use:

* TFTP
* NFS rootfs

Useful for:

* CI labs
* automated testing

---

# 14. Verification Methods

This is where enterprise embedded validation becomes important.

---

# Level 1 — UART Boot Verification

MOST IMPORTANT.

Use:

* USB-UART adapter

Typical baudrate:

```text id="6q7w4t"
1500000
```

(Some boards use 115200)

Monitor:

```bash id="8j2v5s"
picocom -b 1500000 /dev/ttyUSB0
```

Verify:

* SPL boot
* U-Boot
* kernel logs

---

# Level 2 — U-Boot Verification

Interrupt boot:

```text id="5n3r9w"
Hit any key to stop autoboot
```

Commands:

```bash id="4x0s2m"
printenv
```

```bash id="7z1l8f"
mmc list
```

```bash id="2y4c6v"
nvme scan
```

Verify:

* storage detection
* RAM
* boot source

---

# Level 3 — Linux Verification

Check:

```bash id="3j8m2w"
uname -a
```

```bash id="5v1x7n"
cat /etc/os-release
```

```bash id="1p4r8q"
dmesg
```

---

# Level 4 — Multimedia Verification

Rockchip platforms are heavily multimedia-oriented.

Test:

* video decode
* HDMI
* GPU
* VPU

Tools:

* GStreamer
* ffmpeg
* MPP

Example:

```bash id="9t3y5l"
gst-launch-1.0
```

---

# Level 5 — GPU Verification

Rockchip uses:

* Mali GPU

Test:

* OpenGL ES
* Weston
* DRM/KMS

Example:

```bash id="2f8j0v"
glmark2-es2
```

---

# Level 6 — Camera Verification

Important for RK3588 AI systems.

Check:

* CSI camera
* USB camera

Examples:

```bash id="7s4x2n"
v4l2-ctl --list-devices
```

---

# Level 7 — AI/NPU Verification

RK3588 includes:

* NPU acceleration

Validate:

* RKNN toolkit
* inference performance

---

# Level 8 — Storage Verification

Check:

* eMMC
* SD
* NVMe

Example:

```bash id="4d2m9s"
lsblk
```

Benchmark:

```bash id="6p8x1r"
fio
```

---

# Level 9 — Thermal + Stress Testing

Use:

```bash id="1z4q8n"
stress-ng
```

Monitor:

```bash id="8m2x5j"
cat /sys/class/thermal/thermal_zone0/temp
```

RK3588 systems can become very hot under:

* GPU
* NPU
* multimedia workloads

Active cooling is strongly recommended.

---

# 15. Secure Boot Verification

Some Rockchip platforms support:

* secure boot
* trusted boot chains

Validate:

* signed bootloader
* verified kernel

---

# 16. OTA Verification

Common frameworks:

* [RAUC](https://rauc.io/?utm_source=chatgpt.com)
* [Mender](https://mender.io/?utm_source=chatgpt.com)

Validate:

* A/B rootfs
* rollback
* update integrity

---

# 17. CI/CD Validation Pipeline

Enterprise workflow:

```text id="9n7x5v"
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
GPU Tests
   ↓
Multimedia Tests
   ↓
AI Validation
   ↓
Artifact Publish
```

---

# 18. Factory Flashing Workflow

Manufacturing setup:

```text id="2x5m7q"
Factory PC
   ↓ USB OTG
MaskROM Mode
   ↓
rkdeveloptool
   ↓
Automated Validation
```

---

# 19. Common Rockchip Development Issues

| Problem                   | Cause                   |
| ------------------------- | ----------------------- |
| No USB recovery detection | cable/power             |
| HDMI missing              | DTB mismatch            |
| GPU not working           | Mali userspace mismatch |
| Boot failure              | wrong idbloader         |
| Thermal throttling        | poor cooling            |
| NVMe boot issues          | U-Boot config           |

---

# 20. Recommended Enterprise Rockchip Setup

| Area       | Recommendation |
| ---------- | -------------- |
| Storage    | NVMe SSD       |
| Cooling    | Active cooling |
| Build      | kas            |
| BSP        | meta-rockchip  |
| OTA        | RAUC           |
| CI         | GitLab/GitHub  |
| Validation | labgrid/LAVA   |
| Containers | Docker/Podman  |

---

# 21. Typical Enterprise Layer Structure

```text id="4r7n2x"
meta-company/
├── conf/
├── recipes-gpu/
├── recipes-ai/
├── recipes-multimedia/
├── recipes-connectivity/
└── recipes-security/
```

---

# 22. Industrial Best Practices

## DO:

* isolate BSP changes
* validate GPU userspace compatibility
* automate USB flashing
* benchmark thermal behavior
* prefer NVMe storage

## DON'T:

* modify vendor BSP directly
* rely on SD cards in production
* ignore DTB compatibility

---

# Useful References

* [meta-rockchip GitHub](https://github.com/JeffyCN/meta-rockchip?utm_source=chatgpt.com)
* [rkdeveloptool GitHub](https://github.com/rockchip-linux/rkdeveloptool?utm_source=chatgpt.com)
* [Rockchip Linux SDK Wiki](https://opensource.rock-chips.com/wiki_Main_Page?utm_source=chatgpt.com)
* [Yocto Project Documentation](https://docs.yoctoproject.org/?utm_source=chatgpt.com)

