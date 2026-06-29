

## docs/architecture/software-stack.md

```markdown
# Software Stack Documentation

## Overview

The BBB AI-64 software stack is built on the Yocto Project, providing a complete embedded Linux distribution tailored for the BeagleBone Black AI-64 platform.

## Software Stack Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                     Application Layer                                │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌───────────┐ │
│  │  Dashboard  │  │   Sensor    │  │   Device    │  │  Updater  │ │
│  │  (Qt6/QML)  │  │  Service    │  │  Manager    │  │ (SWUpdate)│ │
│  └─────────────┘  └─────────────┘  └─────────────┘  └───────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                     Framework Layer                                  │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌───────────┐ │
│  │  Qt6        │  │  D-Bus      │  │  Systemd    │  │  Docker   │ │
│  │  Framework  │  │  IPC        │  │  Services   │  │  Engine   │ │
│  └─────────────┘  └─────────────┘  └─────────────┘  └───────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                     Middleware Layer                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌───────────┐ │
│  │  Config     │  │  Logger     │  │  IPC        │  │  Security │ │
│  │  Manager    │  │  (syslog)   │  │  Manager    │  │  (SELinux)│ │
│  └─────────────┘  └─────────────┘  └─────────────┘  └───────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                     Operating System                                 │
│  ┌─────────────────────────────────────────────────────────────────┐ │
│  │                      Linux Kernel 6.1                           │ │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐         │ │
│  │  │  I2C     │ │  SPI     │ │  UART    │ │  GPIO    │         │ │
│  │  └──────────┘ └──────────┘ └──────────┘ └──────────┘         │ │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐         │ │
│  │  │  USB     │ │  PCIe    │ │  MMC     │ │  CAN     │         │ │
│  │  └──────────┘ └──────────┘ └──────────┘ └──────────┘         │ │
│  └─────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘
```

## Component Descriptions

### 1. Operating System

#### Yocto Distribution

- **Base**: Poky (Yocto Project reference distribution)
- **Customization**: Custom distro with optimized settings
- **Package Manager**: RPM (with support for DEB/IPK)
- **Init System**: Systemd
- **C Library**: glibc

#### Kernel Configuration

```bash
# Kernel Features
- PREEMPT_RT support
- Device tree overlays
- I2C/SPI/UART/GPIO support
- USB gadget support
- Network drivers (Ethernet, WiFi, BT)
- Audio drivers (HDMI, I2S)
- Video drivers (CSI, HDMI)
- AI accelerator support
```

#### Key Kernel Modules

| Module | Purpose |
|--------|---------|
| i2c-omap | I2C bus controller |
| omap-mcspi | SPI bus controller |
| omap-serial | UART controller |
| gpio-omap | GPIO controller |
| ti-sysc | System control |
| pvrsrvkm | GPU driver |
| ti-dsp | DSP driver |
| ti-c7x | C7x AI accelerator |

### 2. Middleware Layer

#### Configuration Manager

```yaml
# config/manager.yaml
features:
  - JSON/YAML support
  - Runtime updates
  - Validation
  - Default values
  - Environment variables
  - Command-line overrides
```

#### Logger

```yaml
# logger/config.yaml
sinks:
  - console:
      level: INFO
      colored: true
  - file:
      path: /var/log/app.log
      level: DEBUG
      rotation: daily
  - syslog:
      facility: local0
      level: INFO
```

#### IPC Manager

```yaml
# ipc/config.yaml
transport:
  type: unix_socket
  address: /tmp/app.sock
  options:
    max_clients: 10
    timeout: 5000
```

### 3. Framework Layer

#### Qt6 Framework

**Qt Components**:
- QtCore: Core non-GUI functionality
- QtGui: GUI foundation
- QtQuick: QML engine
- QtQuickControls2: UI components
- QtSensors: Sensor access
- QtSerialPort: Serial communication
- QtWebSockets: WebSocket support
- QtMultimedia: Audio/Video
- QtCharts: Charting

**QML Imports**:
```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtSensors 5.15
import QtSerialPort 5.15
import QtWebSockets 1.0
import QtMultimedia 5.15
import QtCharts 2.15
```

#### D-Bus IPC

**Interfaces**:
- org.bbbai64.SensorService
- org.bbbai64.DeviceManager
- org.bbbai64.Updater
- org.bbbai64.SettingsManager

#### Systemd Services

| Service | Description |
|---------|-------------|
| sensor-service | Sensor data collection |
| device-manager | Device management |
| gui-app | GUI application |
| docker | Container runtime |
| swupdate | OTA updates |
| sshd | SSH server |
| connman | Network manager |

### 4. Application Layer

#### GUI Application (Dashboard)

**Features**:
- Real-time sensor data display
- Interactive charts
- Device control interface
- Settings management
- Dark/Light theme
- Touch support

**QML Components**:
- SensorWidget: Sensor display
- ChartWidget: Data visualization
- DeviceControl: Device management
- SettingsPanel: Configuration
- StatusBar: System status

#### Sensor Service

**Architecture**:
```cpp
class SensorService {
    // Sensor management
    vector<SensorDriver> drivers;
    map<string, SensorData> readings;
    
    // Data collection
    void collectData();
    void publishData();
    
    // Events
    void onDataReceived();
    void onError();
};
```

#### Device Manager

**Features**:
- Device discovery
- Device control
- Status monitoring
- Error handling
- Recovery

### 5. Build System

#### Yocto Layers

| Layer | Purpose |
|-------|---------|
| meta-poky | Base distribution |
| meta-oe | OpenEmbedded recipes |
| meta-qt6 | Qt6 support |
| meta-ti | TI platform support |
| meta-security | Security features |
| meta-virtualization | Docker support |
| meta-swupdate | OTA updates |
| meta-custom | Custom recipes |

#### Build Configuration

```bitbake
# local.conf key settings
MACHINE = "bbbai64"
DISTRO = "poky"
BB_NUMBER_THREADS = "8"
PARALLEL_MAKE = "-j 8"

# Features
DISTRO_FEATURES:append = " systemd wayland opengl vulkan"
EXTRA_IMAGE_FEATURES = "debug-tweaks ssh-server-openssh"
```

## Development Tools

### Toolchain

```bash
# SDK Location
/opt/poky/1.0/

# Environment Setup
source /opt/poky/1.0/environment-setup-cortexa72-poky-linux

# Compiler
$CC --version
arm-poky-linux-gnueabi-gcc (GCC) 12.2.0
```

### Debugging Tools

| Tool | Purpose |
|------|---------|
| GDB | Debugger |
| strace | System call tracer |
| perf | Performance analyzer |
| valgrind | Memory debugger |
| gprof | Profiler |
| lttng | Tracing |

### Profiling Tools

```bash
# Performance profiling
perf record -e cycles -a -- sleep 10
perf report

# Memory profiling
valgrind --tool=massif ./app

# CPU profiling
gprof ./app gmon.out > analysis.txt
```

## Deployment

### Image Types

| Image | Purpose |
|-------|---------|
| custom-image | Full development image |
| production-image | Production image |
| minimal-image | Minimal boot image |
| sdk-image | SDK image |

### Storage Layout

```
/dev/mmcblk0
├── mmcblk0p1 (boot) - FAT32, 256MB
│   ├── MLO
│   ├── u-boot.img
│   ├── Image
│   └── *.dtb
├── mmcblk0p2 (rootfs) - ext4, 4GB
│   ├── /bin
│   ├── /boot
│   ├── /dev
│   ├── /etc
│   ├── /home
│   ├── /lib
│   ├── /mnt
│   ├── /opt
│   ├── /proc
│   ├── /root
│   ├── /sbin
│   ├── /sys
│   ├── /tmp
│   ├── /usr
│   └── /var
└── mmcblk0p3 (data) - ext4, remaining space
    ├── /data/sensors
    ├── /data/logs
    └── /data/apps
```

## Security

### Security Layers

1. **Boot Security**
   - Secure Boot (U-Boot verified boot)
   - Kernel signature verification
   - Filesystem integrity (dm-verity)

2. **System Security**
   - SELinux (enforcing mode)
   - Systemd security features
   - AppArmor profiles

3. **Network Security**
   - TLS/SSL support
   - Firewall (iptables/nftables)
   - SSH hardening

4. **Application Security**
   - Authentication/Authorization
   - Audit logging
   - Secure IPC

## Performance Optimization

### Optimization Techniques

1. **System Level**
   - Boot time optimization
   - Memory management
   - CPU frequency scaling
   - I/O scheduling

2. **Application Level**
   - QML caching
   - Lazy loading
   - Data batching
   - Connection pooling

3. **Sensor Level**
   - Buffered reads
   - Interrupt coalescing
   - Power management

## Compatibility

### Supported Hardware

| Component | Compatibility |
|-----------|--------------|
| SoC | TDA4VM only |
| Memory | 8GB LPDDR4 |
| Storage | eMMC, SD, USB |
| Display | HDMI, LCD |
| Camera | CSI-2 |
| Network | Ethernet, WiFi, BT |

### Software Compatibility

| Software | Version |
|----------|---------|
| Linux Kernel | 6.1+ |
| GCC | 12.2+ |
| Python | 3.10+ |
| Qt | 6.4+ |
| Docker | 20.10+ |
| Systemd | 251+ |

## Maintenance

### Update Process

1. **OTA Updates**
   - SWUpdate client
   - Signed update packages
   - Rollback support
   - Update notifications

2. **Manual Updates**
   - apt-get upgrade
   - RPM package updates
   - Source rebuild

### Monitoring

```bash
# System health
systemctl status sensor-service
journalctl -u sensor-service -f

# Performance monitoring
htop
iostat
netstat

# Sensor health
sensor-service --status
device-manager --list
```
```

