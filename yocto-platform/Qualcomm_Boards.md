# Qualcomm Boards + Yocto Workflow

Qualcomm platforms are widely used in:

* automotive systems
* AI edge devices
* 5G gateways
* robotics
* smart cameras
* XR/AR devices
* mobile-class embedded systems

Common Qualcomm SoCs:

* Snapdragon 410/600/800 series
* QCS6490
* RB5
* SA8155P
* SC8280XP

Popular development boards:

* Qualcomm Robotics RB5
* DragonBoard 410c
* QCS6490 RB3 Gen 2

---

# 1. How Qualcomm Uses Yocto

Qualcomm Yocto support usually comes through:

* `meta-qcom`
* vendor BSPs
* Linaro/OpenEmbedded support

Official/open ecosystem:

* Qualcomm Linux SDK
* Linaro BSPs
* Qualcomm AI Stack

Main BSP layer:

* [meta-qcom GitHub](https://github.com/qualcomm-linux/meta-qcom?utm_source=chatgpt.com)

---

# 2. Qualcomm Ecosystem Complexity

Qualcomm systems are more complex because of:

* proprietary firmware
* DSP subsystems
* modem integration
* GPU/NPU acceleration
* Android heritage

Boot architecture is closer to:

* smartphones
* automotive systems

than traditional SBCs.

---

# 3. Typical Yocto Layer Structure

```text id="m9q2x5"
poky/
meta-openembedded/
meta-qcom/
meta-clang/
meta-python/
meta-company/
```

---

# 4. Main BSP Components

| Component    | Purpose              |
| ------------ | -------------------- |
| `meta-qcom`  | Qualcomm BSP         |
| LK/ABL       | Bootloader           |
| Linux kernel | SoC support          |
| DSP firmware | Hexagon acceleration |
| GPU drivers  | Adreno graphics      |
| AI SDK       | NPU acceleration     |

---

# 5. Build Environment Setup

---

# Install Host Dependencies

Ubuntu example:

```bash id="7x1m4q"
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

```bash id="4m8x2v"
git clone -b scarthgap git://git.yoctoproject.org/poky
```

---

# meta-qcom

```bash id="1q7m5x"
git clone https://github.com/qualcomm-linux/meta-qcom
```

---

# meta-openembedded

```bash id="9x2m4r"
git clone -b scarthgap https://github.com/openembedded/meta-openembedded
```

---

# 7. Initialize Build Environment

```bash id="5m1x8q"
source poky/oe-init-build-env
```

---

# 8. Configure Layers

## `bblayers.conf`

```conf id="2x7m4v"
BBLAYERS += " \
../meta-qcom \
../meta-openembedded/meta-oe \
../meta-openembedded/meta-python \
"
```

---

# 9. Select Machine

Examples:

| Board            | MACHINE            |
| ---------------- | ------------------ |
| DragonBoard 410c | `dragonboard-410c` |
| RB5              | `qrb5165-rb5`      |
| RB3 Gen 2        | `qcs6490-rb3gen2`  |

Example:

```conf id="8m2x5q"
MACHINE = "qrb5165-rb5"
```

---

# 10. Build Images

Minimal image:

```bash id="3x1m7v"
bitbake core-image-minimal
```

Wayland image:

```bash id="6m4x2q"
bitbake core-image-weston
```

AI/multimedia image:

```bash id="1v8m5x"
bitbake qcom-multimedia-image
```

---

# 11. Build Outputs

Generated here:

```text id="9q1m4x"
tmp/deploy/images/qrb5165-rb5/
```

Typical outputs:

```text id="7m2x8q"
boot.img
vendor.img
system.img
dtbo.img
Image
```

Qualcomm platforms often use:

* Android-style partitions
* fastboot flashing
* boot/vendor separation

---

# 12. Qualcomm Boot Architecture

VERY IMPORTANT.

---

# Boot Flow

```text id="4x9m2q"
BootROM
   ↓
XBL
   ↓
ABL/LK
   ↓
UEFI/U-Boot
   ↓
Kernel
   ↓
DTB/DTBO
   ↓
RootFS
   ↓
systemd
```

Modern Qualcomm systems heavily use:

* UEFI
* Android-derived boot flows
* A/B partitioning

---

# 13. Storage Options

Qualcomm systems commonly boot from:

* eMMC
* UFS
* NVMe

Production systems strongly prefer:

* UFS
* NVMe

---

# 14. Flashing Methods

---

# Method 1 — Fastboot Flashing (MOST IMPORTANT)

Most Qualcomm boards use:

* fastboot mode

Very similar to Android devices.

---

# Enter Fastboot Mode

Typical process:

1. hold boot/recovery button
2. connect USB
3. power board

Verify:

```bash id="5x2m7q"
fastboot devices
```

---

# Flash Images

```bash id="1m8x4v"
fastboot flash boot boot.img
```

```bash id="6q2m5x"
fastboot flash system system.img
```

```bash id="8x1m7r"
fastboot flash vendor vendor.img
```

Reboot:

```bash id="3m9x2q"
fastboot reboot
```

This is the dominant Qualcomm flashing workflow.

---

# Method 2 — QDL / EDL Flashing

Qualcomm supports:

* Emergency Download Mode (EDL)

Used for:

* board recovery
* manufacturing
* low-level flashing

---

# Tools

Common tools:

* `qdl`
* QFIL
* vendor flash tools

Open-source tool:

* [qdl GitHub](https://github.com/linux-msm/qdl?utm_source=chatgpt.com)

---

# Example

```bash id="2m5x8q"
sudo qdl prog_firehose_ddr.elf rawprogram.xml
```

---

# Method 3 — SD Card Flashing

Some development boards support:

* SD boot

Example:

```bash id="9x1m4v"
sudo dd if=image.wic of=/dev/sdb bs=4M
```

Less common in production.

---

# Method 4 — Network Boot

Advanced setups use:

* TFTP
* NFS rootfs

Useful for:

* CI
* bring-up

---

# 15. Partition Layout

Qualcomm systems commonly use:

* GPT layouts
* Android-style partitions

Examples:

```text id="4m2x7q"
boot
vendor
system
userdata
dtbo
modem
```

Very important for OTA systems.

---

# 16. Verification Methods

Enterprise Qualcomm validation is extensive.

---

# Level 1 — UART Boot Verification

MOST IMPORTANT.

Monitor:

```bash id="8m1x5q"
picocom -b 115200 /dev/ttyUSB0
```

Verify:

* XBL
* ABL
* kernel logs
* boot chain

---

# Level 2 — Bootloader Verification

Check:

* fastboot
* partitions
* A/B slots

Commands:

```bash id="2q8m4x"
fastboot getvar all
```

Verify:

* active slot
* partition integrity

---

# Level 3 — Linux Verification

Check:

```bash id="7m1x2q"
uname -a
```

```bash id="1x5m8v"
dmesg
```

```bash id="5q2m7x"
cat /etc/os-release
```

---

# Level 4 — GPU Verification

Qualcomm platforms use:

* Adreno GPU

Test:

* OpenGL ES
* Vulkan
* Wayland

Example:

```bash id="9m4x1q"
glmark2-es2
```

---

# Level 5 — DSP / AI Verification

VERY IMPORTANT.

Qualcomm platforms often include:

* Hexagon DSP
* NPU acceleration

Validate:

* AI inference
* DSP offload

---

# Level 6 — Camera Verification

Critical for robotics/vision systems.

Test:

* CSI camera
* ISP pipelines

Example:

```bash id="3x7m1q"
v4l2-ctl --list-devices
```

---

# Level 7 — Multimedia Verification

Test:

* hardware encode/decode
* display pipelines
* audio

Example:

```bash id="8q2m5v"
gst-launch-1.0
```

---

# Level 8 — Connectivity Verification

Test:

* Wi-Fi
* Bluetooth
* 5G/modem
* GPS

Examples:

```bash id="1m4x9v"
nmcli
```

```bash id="6x2m7q"
bluetoothctl
```

---

# Level 9 — Thermal + Stress Testing

Use:

```bash id="4q8m1x"
stress-ng
```

Monitor:

* CPU
* GPU
* DSP thermals

Qualcomm SoCs are highly power-managed.

---

# 17. Secure Boot Verification

VERY IMPORTANT.

Qualcomm strongly emphasizes:

* secure boot
* trusted execution
* rollback protection

Validate:

* signed images
* boot chain integrity
* secure partitions

---

# 18. OTA Verification

Common frameworks:

* [RAUC](https://rauc.io/?utm_source=chatgpt.com)
* [Mender](https://mender.io/?utm_source=chatgpt.com)

Qualcomm systems often use:

* A/B updates

Validate:

* slot switching
* rollback
* partition updates

---

# 19. CI/CD Validation Pipeline

Enterprise Qualcomm workflow:

```text id="5x1m8q"
Git Push
   ↓
Yocto Build
   ↓
Image Packaging
   ↓
Fastboot/QDL Flash
   ↓
UART Capture
   ↓
GPU Tests
   ↓
Camera Tests
   ↓
AI Validation
   ↓
OTA Tests
   ↓
Artifact Publish
```

---

# 20. Factory Flashing Workflow

Manufacturing setup:

```text id="2m7x4q"
Factory PC
   ↓ USB
Fastboot/QDL
   ↓
Flash UFS/eMMC
   ↓
Automated Validation
```

---

# 21. Common Qualcomm Development Issues

| Problem               | Cause               |
| --------------------- | ------------------- |
| Fastboot not detected | USB/cable issue     |
| Boot loop             | partition mismatch  |
| GPU missing           | userspace mismatch  |
| Camera failure        | ISP firmware        |
| DSP unavailable       | firmware loading    |
| OTA failure           | slot metadata issue |

---

# 22. Recommended Enterprise Qualcomm Setup

| Area       | Recommendation      |
| ---------- | ------------------- |
| Build      | kas                 |
| BSP        | meta-qcom           |
| OTA        | RAUC                |
| Storage    | UFS/NVMe            |
| CI         | GitLab/GitHub       |
| Debug      | UART always enabled |
| Validation | labgrid/LAVA        |
| Containers | Docker/Podman       |

---

# 23. Typical Enterprise Layer Structure

```text id="9m1x4q"
meta-company/
├── conf/
├── recipes-ai/
├── recipes-camera/
├── recipes-connectivity/
├── recipes-security/
└── recipes-multimedia/
```

---

# 24. Industrial Best Practices

## DO:

* automate fastboot flashing
* validate A/B updates
* isolate proprietary firmware
* benchmark thermal behavior
* version DSP firmware carefully

## DON'T:

* modify vendor BSP directly
* ignore partition layouts
* skip secure boot validation

---

# Useful References

* [meta-qcom GitHub](https://github.com/qualcomm-linux/meta-qcom?utm_source=chatgpt.com)
* [Qualcomm Robotics RB5 Platform](https://www.qualcomm.com/developer/hardware/rb5-platform?utm_source=chatgpt.com)
* [qdl GitHub](https://github.com/linux-msm/qdl?utm_source=chatgpt.com)
* [Yocto Project Documentation](https://docs.yoctoproject.org/?utm_source=chatgpt.com)

