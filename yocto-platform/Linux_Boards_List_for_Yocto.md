If you're building a scalable Yocto platform, it helps to classify boards by:

* architecture,
* vendor ecosystem,
* upstream Yocto support,
* long-term maintenance,
* industrial adoption,
* AI/media capabilities.

Below is a practical board/vendor list commonly used in professional Yocto ecosystems.

# ARM64 / ARM Boards

## Raspberry Pi Family

Very popular for:

* prototyping
* edge devices
* education
* lightweight products

Supported via:

* `meta-raspberrypi`

Boards:

* Raspberry Pi 5
* Raspberry Pi 4 Model B
* Compute Module 4
* Raspberry Pi Zero 2 W

Useful for:

* camera products
* IoT gateways
* edge AI
* kiosk systems

---

# NXP i.MX Boards

Industrial-grade ecosystem.

Supported via:

* `meta-freescale`

Popular SoCs:

* i.MX6
* i.MX7
* i.MX8
* i.MX93

Boards:

* i.MX 8M Mini EVK
* i.MX 8M Plus EVK
* Verdin iMX8M Plus
* Apalis iMX8

Excellent for:

* industrial HMI
* medical
* robotics
* multimedia
* secure devices

---

# Texas Instruments Boards

Supported via:

* `meta-ti`

Boards:

* BeagleBone Black
* BeaglePlay
* SK-AM62
* SK-AM64

Good for:

* industrial control
* real-time systems
* motor control

---

# NVIDIA Jetson Boards

Supported via:

* `meta-tegra`

Boards:

* Jetson Nano
* Jetson Xavier NX
* Jetson Orin Nano
* Jetson AGX Orin

Best for:

* AI inference
* CUDA
* video analytics
* robotics

---

# Rockchip Boards

Supported via:

* `meta-rockchip`

Boards:

* Rock 5B
* Orange Pi 5
* Quartz64

Great for:

* multimedia
* ARM desktop
* AI edge devices

---

# Xilinx / AMD FPGA Boards

Supported via:

* `meta-xilinx`

Boards:

* ZCU102
* KV260 Vision AI Starter Kit
* PYNQ-Z2

Used in:

* FPGA acceleration
* AI pipelines
* telecom
* SDR

---

# STM32 MPU Boards

Supported via:

* `meta-st-stm32mp`

Boards:

* STM32MP157C-DK2
* STM32MP135F-DK

Good for:

* low-power industrial devices
* HMI
* secure embedded systems

---

# Intel x86 Boards

Supported directly in Poky/OpenEmbedded.

Boards:

* Intel NUC
* UP Squared
* LattePanda Sigma

Best for:

* gateways
* edge servers
* virtualization
* container workloads

---

# Qualcomm Boards

Support improving via:

* `meta-qcom`

Boards:

* DragonBoard 410c
* RB5 Robotics Platform

Useful for:

* robotics
* AI
* 5G edge

---

# SiFive / RISC-V Boards

Emerging ecosystem.

Boards:

* HiFive Unmatched
* VisionFive 2

Good for:

* experimentation
* future RISC-V support

---

# Popular Industrial SOM Vendors

These vendors provide better BSP stability than raw SBC vendors.

## Toradex

* [Toradex](https://www.toradex.com/?utm_source=chatgpt.com)

Modules:

* Verdin
* Colibri
* Apalis

Excellent Yocto support.

---

## Variscite

* [Variscite](https://www.variscite.com/?utm_source=chatgpt.com)

Strong NXP ecosystem.

---

## Digi Embedded

* [Digi Embedded](https://www.digi.com/?utm_source=chatgpt.com)

Industrial gateways and SOMs.

---

## PHYTEC

* [PHYTEC](https://www.phytec.com/?utm_source=chatgpt.com)

Strong industrial BSP support.

---

# Recommended Board Categories for Your Platform

## Tier 1 (Must Support)

These give maximum ecosystem coverage.

| Vendor       | Board            |
| ------------ | ---------------- |
| Raspberry Pi | RPi 5            |
| NXP          | i.MX8M Plus      |
| Intel        | Intel NUC        |
| NVIDIA       | Jetson Orin Nano |

---

## Tier 2 (Industrial)

| Vendor | Board    |
| ------ | -------- |
| TI     | AM62     |
| STM    | STM32MP1 |
| Xilinx | KV260    |

---

## Tier 3 (Future/Experimental)

| Vendor   | Board        |
| -------- | ------------ |
| Qualcomm | RB5          |
| RISC-V   | VisionFive 2 |

---

# Yocto BSP Layers You Should Know

| Layer               | Purpose                 |
| ------------------- | ----------------------- |
| `poky`              | Core Yocto              |
| `meta-openembedded` | Community packages      |
| `meta-raspberrypi`  | Raspberry Pi BSP        |
| `meta-freescale`    | NXP BSP                 |
| `meta-ti`           | TI BSP                  |
| `meta-tegra`        | NVIDIA BSP              |
| `meta-rockchip`     | Rockchip BSP            |
| `meta-xilinx`       | AMD/Xilinx BSP          |
| `meta-arm`          | ARM reference platforms |

---

# Suggested Enterprise Support Matrix

## Start Small

### ARM64

* Raspberry Pi 5
* i.MX8M Plus
* Jetson Orin Nano

### x86_64

* Intel NUC

This covers:

* edge AI
* industrial
* multimedia
* gateway/server
* development systems

---

# Long-Term Platform Strategy

## Standardize:

| Area         | Recommendation       |
| ------------ | -------------------- |
| Architecture | ARM64 first          |
| Bootloader   | U-Boot               |
| Init system  | systemd              |
| OTA          | RAUC                 |
| Security     | dm-verity            |
| Containers   | Podman               |
| CI           | kas + GitHub Actions |

---

# Best Sources

* [Yocto Project Compatible Layers](https://layers.openembedded.org/layerindex/branch/master/layers/?utm_source=chatgpt.com)
* [Yocto Project Documentation](https://docs.yoctoproject.org/?utm_source=chatgpt.com)
* [meta-raspberrypi](https://github.com/agherzan/meta-raspberrypi?utm_source=chatgpt.com)
* [meta-freescale](https://github.com/Freescale/meta-freescale?utm_source=chatgpt.com)
* [meta-tegra](https://github.com/OE4T/meta-tegra?utm_source=chatgpt.com)


