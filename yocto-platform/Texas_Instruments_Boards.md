# Texas Instruments (TI) Boards + Yocto Workflow

Texas Instruments boards are heavily used in:

* industrial automation
* robotics
* PLCs
* automotive gateways
* motor control
* real-time systems

Common TI SoCs:

* AM335x
* AM437x
* AM62x
* AM64x
* J721E
* Jacinto family

Popular boards:

* BeagleBone Black
* BeaglePlay
* SK-AM62
* SK-AM64
* J721E EVM

---

# 1. How TI Uses Yocto

TI officially supports Yocto through:

* `meta-ti`

This layer provides:

* BSP
* U-Boot
* Linux kernel
* RT patches
* graphics
* DSP/AI integration
* firmware packages

Official TI SDK:

* Processor SDK Linux

Official docs:

* [TI Processor SDK Linux](https://www.ti.com/tool/PROCESSOR-SDK-LINUX?utm_source=chatgpt.com)

---

# 2. Typical Yocto Layer Structure

```text id="m5z6y5"
poky/
meta-openembedded/
meta-ti/
meta-arm/
meta-company/
```

---

# 3. Main TI BSP Layers

| Layer               | Purpose             |
| ------------------- | ------------------- |
| `meta-ti`           | TI BSP support      |
| `meta-arm`          | ARM common support  |
| `meta-openembedded` | Additional packages |
| `poky`              | Yocto core          |

---

# 4. Build Environment Setup

---

# Install Dependencies

Ubuntu example:

```bash id="jlwmwq"
sudo apt install gawk wget git diffstat unzip texinfo \
gcc build-essential chrpath socat cpio python3 python3-pip \
python3-pexpect xz-utils debianutils iputils-ping \
python3-git python3-jinja2 libsdl1.2-dev xterm \
zstd liblz4-tool
```

---

# 5. Download TI Yocto SDK

TI provides:

* setup scripts
* manifests
* integrated BSPs

Example:

```bash id="6i63mf"
git clone https://git.ti.com/git/arago-project/oe-layersetup.git
```

---

# 6. Setup Environment

Example for AM62:

```bash id="cv2s5q"
./oe-layertool-setup.sh -f configs/processor-sdk-linux-am62xx-evm-09.02.00-config.txt
```

This fetches:

* poky
* meta-ti
* meta-openembedded
* meta-arm

---

# 7. Initialize Build Environment

```bash id="h5m38r"
source oe-init-build-env build
```

Creates:

```text id="em7ixx"
build/
├── conf/
│   ├── local.conf
│   └── bblayers.conf
```

---

# 8. Select TI Machine

Examples:

| Board            | MACHINE            |
| ---------------- | ------------------ |
| AM62 SK          | `am62xx-evm`       |
| AM64 SK          | `am64xx-evm`       |
| BeagleBone Black | `beaglebone-yocto` |
| J721E            | `j721e-evm`        |

Example:

```conf id="8whg3m"
MACHINE = "am62xx-evm"
```

---

# 9. Build Images

Minimal image:

```bash id="qdlcik"
bitbake core-image-minimal
```

TI SDK image:

```bash id="gcrv8f"
bitbake tisdk-default-image
```

Multimedia image:

```bash id="q2bjlwm"
bitbake tisdk-base-image
```

---

# 10. Build Outputs

Generated in:

```text id="ovxd0x"
tmp/deploy/images/am62xx-evm/
```

Typical outputs:

```text id="4bfjlwm"
tisdk-default-image-am62xx-evm.wic.xz
Image
tispl.bin
u-boot.img
sysfw.itb
```

---

# 11. TI Boot Architecture

Understanding TI boot is very important.

---

# Boot Flow

```text id="z0m14p"
ROM Boot
   ↓
SYSFW (System Firmware)
   ↓
SPL (tispl.bin)
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

Modern TI SoCs use:

* TI System Firmware
* secure boot chain
* multiple cores (A53/R5/M4)

---

# 12. Storage Options

TI boards commonly boot from:

* SD card
* eMMC
* OSPI NOR
* NAND
* USB DFU
* Ethernet/TFTP

---

# 13. Flashing Methods

---

# Method 1 — SD Card Flashing

Most common for development.

---

## Decompress Image

```bash id="5x2k9r"
unxz tisdk-default-image-am62xx-evm.wic.xz
```

---

## Flash Using `dd`

```bash id="0j5w7h"
sudo dd if=tisdk-default-image-am62xx-evm.wic \
of=/dev/sdb \
bs=4M \
status=progress \
conv=fsync
```

---

# Method 2 — `bmaptool` (Recommended)

Install:

```bash id="dljdpa"
sudo apt install bmap-tools
```

Flash:

```bash id="3vjlwm"
sudo bmaptool copy \
tisdk-default-image-am62xx-evm.wic.xz \
/dev/sdb
```

Benefits:

* integrity checking
* sparse flashing
* faster writes

---

# Method 3 — USB DFU Flashing

TI boards often support:

* USB Device Firmware Upgrade (DFU)

Useful for:

* manufacturing
* recovery

---

## U-Boot DFU Example

Board enters DFU mode.

Flash:

```bash id="8xyjlwm"
sudo dfu-util -a boot -D tispl.bin
```

---

# Method 4 — U-Boot Network Flashing

Common industrial workflow.

---

## TFTP Boot

In U-Boot:

```bash id="khxjlwm"
setenv serverip 192.168.1.10
setenv ipaddr 192.168.1.20
tftpboot ${loadaddr} Image
booti ${loadaddr} - ${fdtaddr}
```

Very common in:

* labs
* CI systems
* bring-up

---

# Method 5 — eMMC Flashing

Production systems usually use:

* eMMC

Typical workflow:

1. boot from SD
2. flash internal eMMC

Example:

```bash id="99jjlwm"
dd if=image.wic of=/dev/mmcblk0
```

---

# 14. Verification Methods

This is where embedded validation becomes serious.

---

# Level 1 — UART Boot Verification

MOST IMPORTANT.

Use:

* USB-UART adapter

Typical baudrate:

```text id="jlwm2r"
115200 8N1
```

Monitor:

```bash id="jlwmxq"
picocom -b 115200 /dev/ttyUSB0
```

Verify:

* ROM boot
* SYSFW
* SPL
* U-Boot
* kernel logs

---

# Level 2 — U-Boot Verification

Interrupt boot:

```text id="jlwm8u"
Hit any key to stop autoboot
```

Commands:

```bash id="jlwm9d"
printenv
```

```bash id="jlwmjp"
mmc list
```

```bash id="jlwmmi"
bdinfo
```

Verify:

* RAM detection
* boot source
* storage
* network

---

# Level 3 — Linux Verification

Check:

```bash id="jlwmw3"
uname -a
```

```bash id="jlwm7s"
cat /etc/os-release
```

```bash id="jlwmde"
dmesg
```

---

# Level 4 — Peripheral Validation

Test:

* Ethernet
* USB
* CAN
* SPI
* I2C
* GPIO
* UART
* PCIe
* audio

Examples:

```bash id="jlwm5v"
ip a
```

```bash id="jlwmqq"
i2cdetect -y 1
```

```bash id="jlwm1x"
candump can0
```

---

# Level 5 — Real-Time Verification

TI systems are often RT-focused.

Check:

* PREEMPT_RT
* latency
* deterministic scheduling

Example:

```bash id="jlwmhf"
cyclictest
```

---

# Level 6 — DSP/AI Verification

For AM62A/Jacinto platforms:

Validate:

* TIDL
* DSP offload
* AI acceleration

---

# Level 7 — Graphics Verification

Test:

* Weston
* DRM/KMS
* OpenGL ES

Example:

```bash id="jlwm4o"
weston
```

---

# Level 8 — Thermal + Stress Testing

Use:

```bash id="jlwmz6"
stress-ng
```

Monitor:

```bash id="jlwm6a"
cat /sys/class/thermal/thermal_zone0/temp
```

Industrial systems require:

* long-duration testing
* thermal stability

---

# 15. Secure Boot Verification

TI platforms support:

* secure boot
* signed binaries
* encrypted boot

Verify:

* authenticated boot chain
* key provisioning

---

# 16. OTA Verification

Common frameworks:

* [RAUC](https://rauc.io/?utm_source=chatgpt.com)
* [Mender](https://mender.io/?utm_source=chatgpt.com)

Validate:

* rollback
* A/B boot
* update recovery

---

# 17. CI/CD Validation Pipeline

Enterprise workflow:

```text id="jlwm7f"
Git Push
   ↓
Yocto Build
   ↓
Static Validation
   ↓
QEMU Tests
   ↓
Flash Board
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

# 18. Factory Flashing Workflow

Manufacturing setup:

```text id="jlwm8j"
Factory PC
   ↓
USB DFU/TFTP
   ↓
Flash eMMC
   ↓
Automated Validation
```

---

# 19. Typical Industrial TI Workflow

## Development

```text id="jlwmzi"
SD card boot
```

## Production

```text id="jlwm3r"
eMMC + secure boot
```

## CI

```text id="jlwm2z"
network boot + UART automation
```

---

# 20. Recommended Enterprise TI Stack

| Area       | Recommendation      |
| ---------- | ------------------- |
| Build      | kas                 |
| BSP        | meta-ti             |
| OTA        | RAUC                |
| Debug      | UART always enabled |
| Storage    | eMMC                |
| CI         | GitLab/GitHub       |
| Validation | labgrid/LAVA        |
| Containers | Podman              |

---

# 21. Common TI Development Issues

| Problem            | Cause                 |
| ------------------ | --------------------- |
| Boot hangs         | SYSFW mismatch        |
| No UART output     | wrong boot mode       |
| Kernel panic       | DTB mismatch          |
| Peripheral missing | pinmux config         |
| DFU fails          | USB cable/power issue |

---

# 22. Typical Enterprise Layer Structure

```text id="jlwm5m"
meta-company/
├── conf/
├── recipes-core/
├── recipes-kernel/
├── recipes-rt/
├── recipes-connectivity/
└── recipes-security/
```

---

# 23. Industrial Best Practices

## DO:

* isolate BSP changes
* maintain custom layers
* automate flashing
* automate UART capture
* validate RT latency

## DON'T:

* modify vendor layers directly
* hardcode board assumptions
* skip bootloader validation

---

# Useful References

* [TI Processor SDK Linux](https://www.ti.com/tool/PROCESSOR-SDK-LINUX?utm_source=chatgpt.com)
* [meta-ti Git Repository](https://git.yoctoproject.org/meta-ti/?utm_source=chatgpt.com)
* [Yocto Project Documentation](https://docs.yoctoproject.org/?utm_source=chatgpt.com)
* [TI Processor SDK Documentation](https://software-dl.ti.com/processor-sdk-linux/esd/docs/latest/linux/index.html?utm_source=chatgpt.com)

