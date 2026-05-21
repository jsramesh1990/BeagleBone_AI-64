# Toradex Boards + Yocto Workflow

Toradex is one of the strongest embedded Linux vendors for:

* industrial HMI
* medical systems
* robotics
* gateways
* automation
* rugged edge devices

Toradex is highly respected because of:

* strong Yocto support
* long-term BSP maintenance
* upstream Linux focus
* production-ready OTA workflows

Their ecosystem is built around:

* Computer-on-Modules (SoMs)
* carrier boards
* Yocto BSP layers
* Torizon OS

Toradex officially maintains production-quality Yocto BSP layers. ([Toradex Developer Center][1])

---

# 1. Main Toradex Product Families

Popular SoMs:

* Verdin iMX8M Plus
* Verdin AM62
* Apalis iMX8
* Colibri iMX6
* Verdin iMX95

Carrier boards:

* Mallow Carrier Board
* Dahlia Carrier Board
* Iris Carrier Board

---

# 2. Toradex Linux Ecosystem

Toradex provides three Linux approaches:

| Platform               | Usage                          |
| ---------------------- | ------------------------------ |
| Torizon OS             | Container-based embedded Linux |
| Yocto Reference Images | Full Yocto customization       |
| BSP Layers             | Complete custom distro         |

Toradex BSPs are based on Yocto/OpenEmbedded. ([Toradex Developer Center][2])

---

# 3. Main Yocto Layers

Toradex maintains:

| Layer                     | Purpose              |
| ------------------------- | -------------------- |
| `meta-toradex-bsp-common` | Common BSP           |
| `meta-toradex-nxp`        | NXP-based modules    |
| `meta-toradex-ti`         | TI-based modules     |
| `meta-toradex-distro`     | Distro configuration |
| `meta-toradex-demos`      | Reference images     |

Toradex officially documents these BSP layers. ([Toradex Developer Center][1])

---

# 4. Typical Yocto Layer Structure

```text
poky/
meta-openembedded/
meta-toradex-bsp-common/
meta-toradex-nxp/
meta-toradex-ti/
meta-company/
```

---

# 5. How Toradex Uses Yocto

Toradex is heavily Yocto-centric.

Unlike some vendors:

* Yocto is NOT secondary
* BSP maintenance is strong
* upstream alignment is good

Toradex provides:

* production BSPs
* OTA integration
* secure boot
* container workflows
* CI-friendly images

---

# 6. Build Environment Setup

---

# Install Host Dependencies

Ubuntu example:

```bash
sudo apt install gawk wget git diffstat unzip texinfo \
gcc build-essential chrpath socat cpio python3 python3-pip \
python3-pexpect xz-utils debianutils iputils-ping \
python3-git python3-jinja2 libsdl1.2-dev xterm \
zstd liblz4-tool
```

---

# 7. Download Toradex BSP

Toradex provides:

* repo manifests
* BSP manifests
* setup scripts

Example:

```bash
repo init -u https://git.toradex.com/toradex-manifest.git
repo sync
```

---

# 8. Initialize Build Environment

Example:

```bash
source setup-environment build
```

Creates:

```text
build/
├── conf/
│   ├── local.conf
│   └── bblayers.conf
```

---

# 9. Select Machine

Examples:

| Module        | MACHINE         |
| ------------- | --------------- |
| Verdin iMX8MP | `verdin-imx8mp` |
| Verdin AM62   | `verdin-am62`   |
| Apalis iMX8   | `apalis-imx8`   |
| Colibri iMX6  | `colibri-imx6`  |

Example:

```conf
MACHINE = "verdin-imx8mp"
```

---

# 10. Build Images

Minimal image:

```bash
bitbake core-image-minimal
```

Toradex reference image:

```bash
bitbake tdx-reference-minimal-image
```

Wayland image:

```bash
bitbake tdx-reference-multimedia-image
```

Torizon image:

```bash
bitbake torizon-core-docker
```

Toradex documents Yocto image building officially. ([Toradex Developer Center][3])

---

# 11. Build Outputs

Generated here:

```text
tmp/deploy/images/verdin-imx8mp/
```

Typical outputs:

```text
Reference-Minimal-Image.wic
Image
imx-boot
u-boot.bin
```

---

# 12. Toradex Boot Architecture

Depends on SoC family.

For NXP-based modules:

```text
ROM Boot
   ↓
SPL
   ↓
U-Boot
   ↓
Kernel
   ↓
DTB
   ↓
RootFS
   ↓
systemd
```

For TI-based modules:

```text
ROM
 ↓
SYSFW
 ↓
SPL
 ↓
U-Boot
 ↓
Linux
```

---

# 13. Storage Options

Toradex systems commonly use:

* eMMC
* SD card
* NAND
* NVMe

Production systems almost always use:

* eMMC

High-performance AI systems:

* NVMe SSD

---

# 14. Flashing Methods

---

# Method 1 — Easy Installer (MOST IMPORTANT)

Toradex provides:

* Toradex Easy Installer

Official tool:

* [Toradex Easy Installer](https://developer.toradex.com/easy-installer/toradex-easy-installer/?utm_source=chatgpt.com)

This is the main flashing workflow.

---

# Easy Installer Workflow

```text
Recovery Boot
    ↓
Toradex Easy Installer
    ↓
Select Image
    ↓
Flash eMMC
```

Very professional manufacturing workflow.

---

# Method 2 — SD Card Flashing

Development workflow.

Flash image:

```bash
sudo dd if=image.wic of=/dev/sdb bs=4M
```

or:

```bash
sudo bmaptool copy image.wic /dev/sdb
```

---

# Method 3 — USB Recovery Mode

Toradex modules support:

* USB recovery

Useful for:

* recovery
* manufacturing
* automated flashing

---

# Method 4 — U-Boot Network Boot

Common CI workflow.

Example:

```bash
setenv serverip 192.168.1.10
tftpboot ${loadaddr} Image
booti ${loadaddr} - ${fdtaddr}
```

---

# Method 5 — eMMC Installation

Production workflow:

1. boot temporary installer
2. flash eMMC

Example:

```bash
dd if=image.wic of=/dev/mmcblk0
```

---

# 15. Torizon OS

VERY IMPORTANT Toradex concept.

Torizon OS:

* container-based embedded Linux
* Docker/Podman workflows
* OTA integrated
* built on Yocto BSP layers

Toradex explains that Torizon OS is built on top of their Yocto BSP layers. ([Toradex Developer Center][2])

---

# Torizon Architecture

```text
Yocto BSP Layers
      ↓
Torizon OS
      ↓
Docker Containers
      ↓
Applications
```

---

# 16. Verification Methods

---

# Level 1 — UART Boot Verification

MOST IMPORTANT.

Use:

* USB-UART adapter

Monitor:

```bash
picocom -b 115200 /dev/ttyUSB0
```

Verify:

* SPL
* U-Boot
* kernel logs

---

# Level 2 — U-Boot Verification

Commands:

```bash
printenv
```

```bash
mmc list
```

Verify:

* RAM
* boot source
* storage

---

# Level 3 — Linux Verification

Check:

```bash
uname -a
```

```bash
cat /etc/os-release
```

```bash
dmesg
```

---

# Level 4 — Container Verification (Torizon)

VERY IMPORTANT.

Check:

```bash
docker ps
```

or:

```bash
podman ps
```

Validate:

* container runtime
* networking
* restart behavior

---

# Level 5 — OTA Verification

Toradex strongly supports OTA.

Uses:

* OSTree
* Aktualizr
* Torizon OTA

Validate:

* rollback
* atomic updates
* recovery

Torizon integrates OTA/update frameworks directly. ([Toradex Developer Center][4])

---

# Level 6 — Peripheral Validation

Test:

* Ethernet
* USB
* CAN
* SPI
* I2C
* GPIO
* display

Examples:

```bash
ip a
```

```bash
i2cdetect -y 1
```

---

# Level 7 — GPU / Multimedia Validation

For iMX8 systems:

* VPU
* GPU
* Wayland

Test:

* GStreamer
* Weston
* OpenGL ES

Example:

```bash
weston
```

---

# Level 8 — Secure Boot Verification

Toradex platforms support:

* secure boot
* signed images
* verified boot

Validate:

* signed bootloader
* secure chain

---

# Level 9 — Thermal + Stress Testing

Use:

```bash
stress-ng
```

Monitor:

```bash
cat /sys/class/thermal/thermal_zone0/temp
```

Industrial systems require:

* 24/7 validation
* thermal stability

---

# 17. CI/CD Validation Pipeline

Enterprise Toradex workflow:

```text
Git Push
   ↓
Yocto Build
   ↓
Container Build
   ↓
OTA Package Build
   ↓
Flash Device
   ↓
UART Capture
   ↓
Peripheral Tests
   ↓
OTA Validation
   ↓
Artifact Publish
```

---

# 18. Factory Flashing Workflow

Typical manufacturing:

```text
Factory PC
   ↓
Toradex Easy Installer
   ↓
Flash eMMC
   ↓
Automated Validation
```

---

# 19. Common Toradex Development Issues

| Problem            | Cause                     |
| ------------------ | ------------------------- |
| Boot failure       | BSP mismatch              |
| Display missing    | DTB issue                 |
| OTA rollback issue | OSTree config             |
| Docker problems    | storage space             |
| Peripheral missing | pinmux/DTB                |
| Slow boot          | oversized container stack |

---

# 20. Recommended Enterprise Toradex Setup

| Area       | Recommendation     |
| ---------- | ------------------ |
| Build      | kas                |
| BSP        | Toradex BSP layers |
| OTA        | Torizon OTA        |
| Containers | Docker/Podman      |
| CI         | GitLab/GitHub      |
| Storage    | eMMC/NVMe          |
| Validation | labgrid/LAVA       |

---

# 21. Typical Enterprise Layer Structure

```text
meta-company/
├── conf/
├── recipes-containers/
├── recipes-security/
├── recipes-gui/
├── recipes-industrial/
└── recipes-connectivity/
```

---

# 22. Industrial Best Practices

## DO:

* isolate custom layers
* automate Easy Installer workflows
* validate OTA rollback
* separate application containers
* version BSP carefully

## DON'T:

* modify vendor layers directly
* rely on SD cards in production
* ignore secure boot testing

---

# 23. Why Toradex Is Popular in Industry

Strong points:

* excellent documentation
* long-term support
* Yocto-first mindset
* production OTA
* industrial focus
* stable BSP maintenance

Community discussions also frequently mention strong Yocto support and industrial workflows. ([Reddit][5])

---

# Useful References

* [Toradex Developer Center](https://developer.toradex.com/?utm_source=chatgpt.com)
* [Toradex Yocto Documentation](https://developer.toradex.com/linux-bsp/os-development/build-yocto/yocto-project/?utm_source=chatgpt.com)
* [Build Reference Image with Yocto](https://developer.toradex.com/linux-bsp/os-development/build-yocto/build-a-reference-image-with-yocto-projectopenembedded/?utm_source=chatgpt.com)
* [Torizon OS Technical Overview](https://developer.toradex.com/torizon/torizoncore/torizoncore-technical-overview/?utm_source=chatgpt.com)
* [Toradex Easy Installer](https://developer.toradex.com/easy-installer/toradex-easy-installer/?utm_source=chatgpt.com)

[1]: https://developer.toradex.cn/linux-bsp/os-development/build-yocto/yocto-project/?utm_source=chatgpt.com "Yocto Project | Toradex Developer Center"
[2]: https://developer.toradex.com/torizon/torizoncore/torizon-frequently-asked-questions-faq/?utm_source=chatgpt.com "Torizon Frequently Asked Questions (FAQ) | Toradex Developer Center"
[3]: https://developer.toradex.cn/linux-bsp/os-development/build-yocto/build-a-reference-image-with-yocto-projectopenembedded/?utm_source=chatgpt.com "Build a Reference Image | Toradex Developer Center"
[4]: https://developer.toradex.com/torizon/torizoncore/torizoncore-technical-overview/?utm_source=chatgpt.com "Torizon OS Technical Overview | Toradex Developer Center"
[5]: https://www.reddit.com/r/embedded/comments/1rp6d3w/architecture_yocto_setup_for_an_imx8mp_data_logger/?utm_source=chatgpt.com "Architecture & Yocto Setup for an i.MX8MP Data Logger"

