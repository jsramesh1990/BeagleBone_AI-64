# DTS and DTB in Embedded Linux

## Overview

DTS and DTB are fundamental concepts in:
- Embedded Linux
- ARM-based systems
- SoC platforms
- Board Support Packages (BSP)

They are used to describe:
```text
hardware configuration to the Linux kernel
```

Without Device Tree:
- kernel would need hardcoded hardware information
- portability becomes difficult
- maintaining board support becomes complex

Device Tree enables:
```text
hardware description separate from kernel code
```

---

# 1. What is Device Tree?

Device Tree is:
```text
a data structure describing hardware components
```

used by:
- Linux kernel
- bootloader
- firmware

---

# 2. Why Device Tree is Needed

Modern SoCs contain:
- CPUs
- UARTs
- SPI
- I2C
- GPIO
- DMA
- timers
- interrupt controllers

Kernel must know:
- hardware addresses
- interrupts
- clocks
- memory layout

---

# 3. Traditional Hardware Description

Earlier systems used:
```text
board-specific hardcoded kernel files
```

Problems:
- poor scalability
- difficult maintenance
- kernel recompilation needed

---

# 4. Device Tree Solution

Device Tree separates:
```text
hardware description
```

from:
```text
kernel source code
```

---

# 5. Device Tree Components

| Component | Description |
|-----------|-------------|
| DTS | Device Tree Source |
| DTB | Device Tree Blob |
| DTC | Device Tree Compiler |

---

# 6. DTS Definition

DTS =
```text
Device Tree Source
```

Human-readable text file.

Extension:
```text
.dts
```

---

# 7. DTB Definition

DTB =
```text
Device Tree Blob
```

Binary compiled version of DTS.

Extension:
```text
.dtb
```

---

# 8. DTC Definition

DTC =
```text
Device Tree Compiler
```

Converts:
```text
DTS → DTB
```

---

# 9. High-Level Device Tree Flow

```text
DTS Source File
       ↓
DTC Compiler
       ↓
DTB Binary
       ↓
Bootloader Loads DTB
       ↓
Kernel Parses DTB
``` id="flow1"

---

# 10. Device Tree Architecture

```text
+----------------------+
| Linux Kernel         |
+----------------------+
          ↑
+----------------------+
| DTB (Binary Blob)    |
+----------------------+
          ↑
+----------------------+
| DTS Source File      |
+----------------------+
``` id="arch1"

---

# 11. DTS File Structure

DTS uses:
```text
tree-like hierarchical structure
```

---

# 12. Basic DTS Example

```dts
/dts-v1/;

/ {
    model = "MyBoard";

    memory {
        reg = <0x80000000 0x4000000>;
    };
};
``` id="dts1"

---

# 13. Root Node

```dts
/
```

represents:
```text
entire hardware system
```

---

# 14. Nodes in Device Tree

Each hardware component represented as:
```text
node
```

Examples:
- cpu node
- uart node
- gpio node

---

# 15. Device Tree Node Example

```dts
uart0 {
    compatible = "ns16550";
};
``` id="node1"

---

# 16. Properties

Node attributes called:
```text
properties
```

Example:
```dts
compatible = "vendor,device";
``` id="prop1"

---

# 17. Important Properties

| Property | Purpose |
|----------|---------|
| compatible | Driver matching |
| reg | Register addresses |
| interrupts | IRQ numbers |
| clocks | Clock references |
| status | Device enabled/disabled |

---

# 18. compatible Property

Most important property.

Used for:
```text
driver matching
```

Example:

```dts
compatible = "arm,pl011";
``` id="comp1"

---

# 19. reg Property

Defines:
```text
memory-mapped register addresses
```

Example:

```dts
reg = <0x101f1000 0x1000>;
``` id="reg1"

Meaning:
```text
base address + size
```

---

# 20. interrupts Property

Defines:
```text
IRQ number
```

Example:

```dts
interrupts = <5>;
``` id="irq1"

---

# 21. status Property

Controls whether device enabled.

Example:

```dts
status = "okay";
``` id="status1"

---

# 22. Disabled Device Example

```dts
status = "disabled";
``` id="status2"

---

# 23. Device Tree Hierarchy

```text
/
├── cpus
├── memory
├── soc
│   ├── uart0
│   ├── spi0
│   └── i2c0
``` id="tree1"

---

# 24. CPU Node Example

```dts
cpus {
    cpu@0 {
        device_type = "cpu";
    };
};
``` id="cpu1"

---

# 25. UART Node Example

```dts
uart0: serial@101f1000 {
    compatible = "arm,pl011";
    reg = <0x101f1000 0x1000>;
    interrupts = <5>;
};
``` id="uart1"

---

# 26. GPIO Node Example

```dts
gpio0 {
    compatible = "vendor,gpio";
};
``` id="gpio1"

---

# 27. Device Tree Include Files

Reusable files:
```text
.dtsi
```

---

# 28. DTS vs DTSI

| File | Purpose |
|------|---------|
| .dts | Board-specific |
| .dtsi | Shared SoC description |

---

# 29. Include Example

```dts
#include "soc.dtsi"
``` id="inc1"

---

# 30. Device Tree Compilation

Compile command:

```bash
dtc -I dts -O dtb board.dts -o board.dtb
``` id="dtc1"

---

# 31. Bootloader Role

Bootloader:
- loads kernel
- loads DTB
- passes DTB address to kernel

---

# 32. Boot Flow with DTB

```text
Power ON
    ↓
ROM Code
    ↓
Bootloader
    ↓
Load Kernel
    ↓
Load DTB
    ↓
Kernel Parses Hardware Info
``` id="boot1"

---

# 33. Kernel DTB Parsing

Kernel reads:
- memory map
- devices
- interrupts
- clocks

from DTB.

---

# 34. Device Driver Matching

Kernel matches drivers using:
```text
compatible property
```

---

# 35. Driver Matching Flow

```text
Kernel Reads compatible
         ↓
Search Matching Driver
         ↓
Load Driver
``` id="match1"

---

# 36. Example Driver Match

DTS:

```dts
compatible = "vendor,myuart";
```

Driver:

```c
.of_match_table = my_uart_ids
``` id="drv1"

---

# 37. Device Tree in ARM Systems

Extensively used in:
- ARM
- ARM64
- RISC-V

---

# 38. x86 and Device Tree

x86 mainly uses:
```text
ACPI
```

instead of Device Tree.

---

# 39. Device Tree Aliases

Aliases simplify device naming.

Example:

```dts
aliases {
    serial0 = &uart0;
};
``` id="alias1"

---

# 40. Chosen Node

Used for:
- kernel arguments
- console settings

Example:

```dts
chosen {
    bootargs = "console=ttyS0";
};
``` id="chosen1"

---

# 41. Memory Node

Defines RAM.

Example:

```dts
memory@80000000 {
    reg = <0x80000000 0x4000000>;
};
``` id="mem1"

---

# 42. Reserved Memory

Memory reserved for:
- DMA
- firmware
- GPU

---

# 43. Clock Nodes

Describes:
```text
hardware clocks
```

---

# 44. Interrupt Controller Node

Defines:
```text
IRQ controller hardware
```

---

# 45. Pin Control (Pinctrl)

Configures:
- pin multiplexing
- pull-ups
- drive strength

---

# 46. Pinctrl Example

```dts
pinctrl_uart0 {
    pins = "PA0", "PA1";
};
``` id="pin1"

---

# 47. Device Tree Overlay

Overlay dynamically modifies:
```text
base device tree
```

Used in:
- Raspberry Pi
- FPGA systems

---

# 48. Overlay Flow

```text
Base DTB
    ↓
Apply Overlay
    ↓
Modified Hardware Description
``` id="overlay1"

---

# 49. Common DTB Locations

```text
/boot/*.dtb
```

---

# 50. View Device Tree in Linux

```bash
ls /proc/device-tree
``` id="proc1"

---

# 51. Dump Device Tree

```bash
dtc -I dtb -O dts board.dtb
``` id="dump1"

---

# 52. Debugging Device Tree

---

## Kernel Logs

```bash
dmesg
``` id="dbg1"

---

## Check Loaded Device Tree

```bash
cat /proc/device-tree/model
``` id="dbg2"

---

# 53. Common Device Tree Problems

| Problem | Description |
|---------|-------------|
| Wrong compatible | Driver not loaded |
| Wrong IRQ | Interrupt failure |
| Incorrect reg | MMIO access failure |
| Missing clocks | Peripheral failure |

---

# 54. Advantages of Device Tree

| Advantage | Description |
|-----------|-------------|
| Hardware abstraction | Yes |
| Reusable kernel | Yes |
| Easier maintenance | Yes |
| Portable | Yes |

---

# 55. Disadvantages

| Issue | Description |
|------|-------------|
| Complexity | Large trees |
| Debugging difficulty | Possible |
| Platform-specific syntax | Yes |

---

# 56. Embedded Linux Device Tree Workflow

```text
Board Hardware Designed
         ↓
Engineer Writes DTS
         ↓
DTC Compiles DTB
         ↓
Bootloader Loads DTB
         ↓
Kernel Parses DTB
         ↓
Drivers Initialized
``` id="final1"

---

# 57. DTS vs DTB Summary

| Feature | DTS | DTB |
|---------|-----|-----|
| Format | Text | Binary |
| Editable | Yes | No |
| Human-readable | Yes | No |
| Used by kernel directly | No | Yes |

---

# 58. Real Embedded Linux Example

```text
UART Hardware Exists
        ↓
UART Node Added in DTS
        ↓
DTB Generated
        ↓
Kernel Matches UART Driver
        ↓
/dev/ttyS0 Created
``` id="real1"

---

# 59. Summary

- Device Tree describes hardware to Linux kernel
- DTS is source format
- DTB is compiled binary format
- Bootloader passes DTB to kernel
- Kernel uses DTB for driver initialization
- Essential in modern embedded Linux systems

---

````
