
# Boot Flow Documentation

## Overview

This document describes the complete boot flow for the BBB AI-64 system, from power-on to application startup.

## Boot Flow Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                        BOOT FLOW                                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. POWER ON                                                        │
│     └─> ROM Bootloader                                              │
│         └─> Reads boot mode pins                                    │
│             └─> Selects boot device (SD/eMMC/USB)                  │
│                                                                     │
│  2. U-Boot (SPL)                                                    │
│     └─> Initializes DDR memory                                     │
│         └─> Loads U-Boot from boot partition                       │
│             └─> Jumps to U-Boot                                    │
│                                                                     │
│  3. U-Boot (Full)                                                   │
│     └─> Initializes hardware                                        │
│         └─> Reads boot script (boot.scr)                           │
│             └─> Loads kernel Image and DTBs                        │
│                 └─> Boots Linux kernel                             │
│                                                                     │
│  4. Linux Kernel                                                    │
│     └─> Decompresses kernel                                         │
│         └─> Initializes subsystems                                  │
│             └─> Mounts root filesystem                              │
│                 └─> Starts init (systemd)                          │
│                                                                     │
│  5. Systemd                                                         │
│     └─> Starts system services                                      │
│         └─> Mounts filesystems                                     │
│             └─> Starts services                                     │
│                 └─> System ready                                    │
│                                                                     │
│  6. Application Startup                                             │
│     └─> Sensor Service starts                                       │
│         └─> Device Manager starts                                   │
│             └─> GUI Application starts                              │
│                 └─> System fully operational                       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Detailed Boot Stages

### Stage 1: Power-On Reset (POR)

#### ROM Bootloader

The ROM bootloader is hardcoded in the SoC and executed on power-on:

```
1. Hardware Reset
   ├─> CPU wakes up
   ├─> Clocks stabilize
   └─> Boot ROM executes

2. Boot ROM Operations
   ├─> Reads boot mode pins (SYSBOOT[5:0])
   ├─> Selects boot device
   ├─> Loads first stage bootloader (SPL)
   └─> Jumps to SPL

3. Boot Device Priority
   ├─> 1. SD Card (MMC1)
   ├─> 2. eMMC (MMC0)
   ├─> 3. USB
   ├─> 4. UART
   └─> 5. SPI
```

### Stage 2: U-Boot SPL

#### Secondary Program Loader (SPL)

```
1. SPL Initialization
   ├─> Sets up basic hardware
   ├─> Configures DDR memory
   ├─> Enables cache
   └─> Sets up serial console

2. SPL Operations
   ├─> Locates U-Boot on boot device
   ├─> Loads U-Boot to RAM
   ├─> Validates U-Boot (checksum)
   └─> Jumps to U-Boot

3. SPL Location
   ├─> SD Card: /boot/MLO
   ├─> eMMC: /boot/MLO
   └─> Size: ~64KB
```

### Stage 3: U-Boot Full

#### U-Boot Bootloader

```
1. U-Boot Initialization
   ├─> Full hardware initialization
   ├─> Console setup (serial, HDMI)
   ├─> Memory management
   └─> Device tree setup

2. U-Boot Environment
   ├─> Loads environment from storage
   ├─> Sets boot parameters
   ├─> Configures networking
   └─> Prepares boot command

3. Boot Command Sequence
   ├─> Read boot.scr from /boot
   ├─> Execute boot commands
   ├─> Load kernel Image
   ├─> Load device tree blobs
   └─> Boot Linux kernel

4. Boot Script (boot.scr)
   #!/bin/sh
   setenv bootargs console=ttyS0,115200 root=/dev/mmcblk0p2 rootwait
   load mmc 0:1 ${loadaddr} /boot/Image
   load mmc 0:1 ${fdtaddr} /boot/bbbai64.dtb
   booti ${loadaddr} - ${fdtaddr}
```

### Stage 4: Linux Kernel

#### Kernel Boot Process

```
1. Kernel Entry
   ├─> Decompression (self-extracting)
   ├─> Early console setup
   ├─> Memory detection
   └─> CPU identification

2. Kernel Initialization
   ├─> Setup architecture
   ├─> Initialize interrupts
   ├─> Setup scheduling
   ├─> Initialize caches
   └─> Memory management

3. Device Tree Parsing
   ├─> Reads device tree blob
   ├─> Matches compatible devices
   ├─> Initializes devices
   └─> Mounts sysfs, procfs

4. Driver Initialization
   ├─> I2C drivers
   ├─> SPI drivers
   ├─> UART drivers
   ├─> GPIO drivers
   ├─> USB drivers
   ├─> Network drivers
   ├─> Display drivers
   └─> Sensor drivers

5. Root Filesystem
   ├─> Identifies root device
   ├─> Mounts root filesystem
   ├─> Checks filesystem (fsck)
   └─> Switches to root

6. Init System
   ├─> Starts init (systemd)
   ├─> Starts systemd-journald
   └─> Starts systemd-udevd
```

### Stage 5: Systemd Init

#### Systemd Initialization

```
1. Systemd Startup
   ├─> Starts basic.target
   ├─> Starts sysinit.target
   ├─> Starts local-fs.target
   └─> Starts swap.target

2. Service Activation
   ├─> Network services
   ├─> System services
   ├─> User services
   └─> Graphical services

3. Target Units
   ├─> multi-user.target
   ├─> graphical.target
   └─> default.target
```

#### Systemd Service Order

```
graphical.target
    └─> multi-user.target
        ├─> basic.target
        │   ├─> sysinit.target
        │   │   ├─> systemd-journald.service
        │   │   ├─> systemd-udevd.service
        │   │   └─> systemd-modules-load.service
        │   └─> systemd-logind.service
        ├─> dbus.service
        ├─> network.target
        │   ├─> connman.service
        │   └─> networkd.service
        ├─> sensor-service.service
        ├─> device-manager.service
        └─> docker.service
```

### Stage 6: Application Startup

#### Sensor Service

```
1. Sensor Service Initialization
   ├─> Loads configuration
   ├─> Scans for sensors (I2C/SPI)
   ├─> Initializes sensor drivers
   ├─> Starts data collection
   └─> Publishes to IPC

2. Sensor Discovery
   ├─> I2C bus scan
   ├─> SPI device detection
   ├─> GPIO pin detection
   └─> Sensor type identification

3. Data Collection
   ├─> Starts collection thread
   ├─> Reads all sensors
   ├─> Validates data
   └─> Publishes data
```

#### Device Manager

```
1. Device Manager Initialization
   ├─> Loads device configuration
   ├─> Scans for devices
   ├─> Creates device objects
   ├─> Initializes devices
   └─> Starts device monitoring

2. Device Discovery
   ├─> USB device enumeration
   ├─> PCI device scan
   ├─> MMC device detection
   └─> Network device detection
```

#### GUI Application

```
1. GUI Initialization
   ├─> Qt platform setup
   ├─> QML engine creation
   ├─> C++ components registration
   ├─> QML loading
   └─> Main window show

2. QML Loading Process
   ├─> Loads main.qml
   ├─> Creates QML components
   ├─> Establishes connections
   ├─> Initializes models
   └─> Shows main window
```

## Boot Times

### Boot Time Analysis

| Stage | Time | Description |
|-------|------|-------------|
| Power-on | 0ms | POR |
| ROM Boot | 50ms | Load SPL |
| SPL | 100ms | DDR init, load U-Boot |
| U-Boot | 500ms | Hardware init, load kernel |
| Kernel | 2000ms | Kernel decompress, init |
| Systemd | 3000ms | Service start |
| GUI | 2000ms | QML load, show window |
| **Total** | **~7650ms** | |

### Optimization Opportunities

```
1. U-Boot Optimization
   ├─> Reduce debug output
   ├─> Faster boot device detection
   └─> Parallel initialization

2. Kernel Optimization
   ├─> Minimal kernel config
   ├─> Faster decompression
   └─> Parallel driver init

3. Systemd Optimization
   ├─> Parallel service start
   ├─> Reduce service dependencies
   └─> Optimize service scripts

4. GUI Optimization
   ├─> Preload QML components
   ├─> Lazy initialization
   └─> Caching strategies
```

## Debugging Boot Issues

### Boot Debugging Tools

1. **Serial Console**
   ```bash
   # Connect to serial console
   screen /dev/ttyUSB0 115200
   
   # Or with minicom
   minicom -b 115200 -D /dev/ttyUSB0
   ```

2. **Kernel Debugging**
   ```bash
   # Kernel command line
   setenv bootargs console=ttyS0,115200 earlyprintk debug loglevel=8
   ```

3. **Systemd Debugging**
   ```bash
   # Enable systemd debug
   systemd.log_level=debug
   systemd.log_target=console
   ```

### Common Boot Issues

1. **No Serial Output**
   - Check console settings
   - Verify UART connections
   - Check power supply

2. **Kernel Panic**
   - Verify bootargs
   - Check device tree
   - Validate kernel image

3. **Root Filesystem Errors**
   - Check partition table
   - Verify filesystem
   - Run fsck

## Boot Configuration Files

### U-Boot Environment

```bash
# /boot/uEnv.txt
mmcdev=0
mmcpart=1
kernel_file=Image
dtb_file=bbbai64.dtb
bootargs=console=ttyS0,115200 root=/dev/mmcblk0p2 rootwait
```

### Boot Script

```bash
# /boot/boot.scr
# Generated from boot.cmd

setenv bootargs console=ttyS0,115200 root=/dev/mmcblk0p2 rootwait
load mmc 0:1 ${loadaddr} /boot/Image
load mmc 0:1 ${fdtaddr} /boot/${dtb_file}
booti ${loadaddr} - ${fdtaddr}
```

### Kernel Command Line

```bash
# Full kernel command line
console=ttyS0,115200 root=/dev/mmcblk0p2 rootwait rw
systemd.show_status=true
systemd.log_level=info
```

## Firmware Update

### Update Methods

1. **OTA Updates (SWUpdate)**
   ```bash
   # Update via SWUpdate
   swupdate -i /path/to/update.swu
   ```

2. **SD Card Update**
   ```bash
   # Flash new image to SD card
   sudo dd if=new-image.wic of=/dev/sdX bs=4M
   ```

3. **Network Update**
   ```bash
   # Download and apply update
   wget http://update.server/image.wic
   swupdate -i image.wic
   ```

### Rollback Procedure

1. **U-Boot Fallback**
   ```bash
   # Boot from backup partition
   setenv bootpart 2
   saveenv
   reset
   ```

2. **Manual Recovery**
   ```bash
   # Boot from SD card
   setenv mmcdev 1
   saveenv
   reset
   ```
```

## docs/hardware/sensors.md

```markdown
# Sensor Documentation

## Overview

The BBB AI-64 platform supports a wide range of sensors through various interfaces (I2C, SPI, UART, GPIO). This document details the supported sensors, their connections, and usage.

## Sensor Map

```
┌─────────────────────────────────────────────────────────────────────┐
│                        SENSOR MAP                                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐            │
│  │    IMU      │    │    GPS      │    │ Temperature │            │
│  │  (MPU6050)  │    │  (NMEA)     │    │  (TMP102)   │            │
│  │  I2C:0x68   │    │  UART:0     │    │  I2C:0x48   │            │
│  └─────────────┘    └─────────────┘    └─────────────┘            │
│                                                                     │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐            │
│  │  Pressure   │    │  Humidity   │    │  Light      │            │
│  │  (BMP180)   │    │  (DHT22)    │    │  (BH1750)   │            │
│  │  I2C:0x77   │    │  GPIO:17    │    │  I2C:0x23   │            │
│  └─────────────┘    └─────────────┘    └─────────────┘            │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Supported Sensors

### 1. IMU - MPU6050/MPU9250

#### Specifications

| Parameter | Value |
|-----------|-------|
| Type | Inertial Measurement Unit |
| Interface | I2C |
| Address | 0x68 (MPU6050), 0x68/0x0C (MPU9250) |
| Accelerometer | ±2g, ±4g, ±8g, ±16g |
| Gyroscope | ±250, ±500, ±1000, ±2000 °/s |
| Magnetometer | ±4800 µT (MPU9250 only) |
| Temperature | -40°C to +85°C |
| Sample Rate | Up to 1000 Hz |

#### Pin Connections

```
MPU6050     BBB AI-64
--------    ---------
VCC     ->  3.3V (P9_03)
GND     ->  GND (P9_01)
SCL     ->  SCL (P9_19)
SDA     ->  SDA (P9_20)
ADO     ->  GND (P9_01)
INT     ->  GPIO (P9_15)
```

#### Register Map

| Register | Address | Description |
|----------|---------|-------------|
| WHO_AM_I | 0x75 | Device ID |
| PWR_MGMT_1 | 0x6B | Power management |
| ACCEL_CONFIG | 0x1C | Accelerometer config |
| GYRO_CONFIG | 0x1B | Gyroscope config |
| SMPLRT_DIV | 0x19 | Sample rate divider |
| ACCEL_XOUT_H | 0x3B | Accelerometer X high byte |
| ACCEL_XOUT_L | 0x3C | Accelerometer X low byte |
| GYRO_XOUT_H | 0x43 | Gyroscope X high byte |

#### Data Format

```cpp
// Accelerometer (g)
int16_t accelX = (buffer[0] << 8) | buffer[1];
float accelX_g = accelX / 16384.0;  // ±2g range

// Gyroscope (°/s)
int16_t gyroX = (buffer[8] << 8) | buffer[9];
float gyroX_dps = gyroX / 131.0;    // ±250°/s range

// Temperature (°C)
int16_t temp = (buffer[6] << 8) | buffer[7];
float temp_c = (temp / 340.0) + 36.53;
```

#### Usage Example

```cpp
#include "IMUDriver.h"

IMUDriver imu("imu-001");

// Configure sensor
Json::Value config;
config["accel_range"] = 2;      // ±8g
config["gyro_range"] = 1;       // ±500°/s
config["sample_rate"] = 100;    // 100 Hz

imu.initialize(config);
imu.start();

// Read data
SensorReading reading;
if (imu.readSample(reading)) {
    double accelX = reading.values["accel_x"];
    double gyroX = reading.values["gyro_x"];
    double temp = reading.values["temperature"];
}
```

### 2. GPS - NMEA Receiver

#### Specifications

| Parameter | Value |
|-----------|-------|
| Type | GPS Receiver |
| Interface | UART |
| Baud Rate | 9600 (default) |
| Protocol | NMEA 0183 |
| Channels | 22 (GPS) / 72 (GPS+GLONASS) |
| Accuracy | 2.5m (horizontal) |
| Update Rate | 1-10 Hz |
| Sensitivity | -165 dBm |

#### Pin Connections

```
GPS Module     BBB AI-64
-----------    ---------
VCC        ->  5V (P9_05)
GND        ->  GND (P9_01)
TX         ->  RX (P9_24)
RX         ->  TX (P9_26)
PPS        ->  GPIO (P9_15)
```

#### NMEA Messages

| Message | Description |
|---------|-------------|
| $GPGGA | GPS fix data |
| $GPGLL | Geographic position |
| $GPRMC | Recommended minimum data |
| $GPVTG | Course and speed |
| $GPGSA | Dilution of precision |
| $GPGSV | Satellites in view |

#### Data Parsing Example

```cpp
void parseGGA(const std::string& data) {
    // $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
    
    std::vector<std::string> fields = split(data, ',');
    
    if (fields.size() < 10) return;
    
    // Time: 123519 (12:35:19 UTC)
    std::string time = fields[1];
    
    // Latitude: 4807.038,N
    double lat = parseNMEACoordinate(fields[2], fields[3]);
    
    // Longitude: 01131.000,E
    double lon = parseNMEACoordinate(fields[4], fields[5]);
    
    // Quality: 1 = GPS fix
    int quality = std::stoi(fields[6]);
    
    // Satellites: 8
    int satellites = std::stoi(fields[7]);
    
    // Altitude: 545.4 meters
    double altitude = std::stod(fields[9]);
}
```

### 3. Temperature - TMP102

#### Specifications

| Parameter | Value |
|-----------|-------|
| Type | Temperature Sensor |
| Interface | I2C |
| Address | 0x48-0x4F |
| Range | -40°C to +125°C |
| Accuracy | ±0.5°C (typ) |
| Resolution | 12-bit (0.0625°C) |
| Sample Rate | Up to 4 Hz |

#### Pin Connections

```
TMP102       BBB AI-64
-------      ---------
VCC      ->  3.3V (P9_03)
GND      ->  GND (P9_01)
SCL      ->  SCL (P9_19)
SDA      ->  SDA (P9_20)
ADDR     ->  GND (P9_01)  // Address 0x48
ALERT    ->  GPIO (P9_15)
```

#### Configuration

```cpp
// Initialize TMP102
void initTMP102() {
    // Set configuration register
    uint8_t config = 0x00;
    // Resolution: 12-bit
    config |= (3 << 6);
    // Normal operation
    config &= ~(1 << 4);
    // Alert: active low
    config &= ~(1 << 3);
    // Comparator mode
    config &= ~(1 << 2);
    // Default threshold: 0°C
    config &= ~(1 << 1);
    
    writeRegister(0x01, config);
}

// Read temperature
double readTMP102() {
    uint16_t value;
    readRegister(0x00, value);
    
    // 12-bit temperature data
    int16_t tempRaw = (value >> 4) & 0x0FFF;
    
    // Check sign
    if (tempRaw & 0x0800) {
        tempRaw |= 0xF000;
    }
    
    return tempRaw * 0.0625;
}
```

### 4. Pressure - BMP180

#### Specifications

| Parameter | Value |
|-----------|-------|
| Type | Barometric Pressure Sensor |
| Interface | I2C |
| Address | 0x77 |
| Pressure Range | 300-1100 hPa |
| Accuracy | ±1 hPa |
| Temperature Range | -40°C to +85°C |
| Resolution | 0.1 hPa |
| Sample Rate | Up to 10 Hz |

#### Pin Connections

```
BMP180       BBB AI-64
-------      ---------
VCC      ->  3.3V (P9_03)
GND      ->  GND (P9_01)
SCL      ->  SCL (P9_19)
SDA      ->  SDA (P9_20)
```

#### Calibration Data

```cpp
struct BMP180Calibration {
    int16_t AC1, AC2, AC3;
    uint16_t AC4, AC5, AC6;
    int16_t B1, B2;
    int16_t MB, MC, MD;
};

void readCalibration(BMP180Calibration& cal) {
    readRegister(0xAA, cal.AC1);
    readRegister(0xAC, cal.AC2);
    readRegister(0xAE, cal.AC3);
    readRegister(0xB0, cal.AC4);
    readRegister(0xB2, cal.AC5);
    readRegister(0xB4, cal.AC6);
    readRegister(0xB6, cal.B1);
    readRegister(0xB8, cal.B2);
    readRegister(0xBA, cal.MB);
    readRegister(0xBC, cal.MC);
    readRegister(0xBE, cal.MD);
}
```

### 5. Humidity - DHT22

#### Specifications

| Parameter | Value |
|-----------|-------|
| Type | Humidity and Temperature |
| Interface | GPIO (1-Wire) |
| Humidity Range | 0-100% RH |
| Accuracy | ±2% RH |
| Temperature Range | -40°C to +80°C |
| Accuracy | ±0.5°C |
| Sampling Rate | 0.5 Hz |
| Supply Voltage | 3-5V |

#### Pin Connections

```
DHT22        BBB AI-64
-------      ---------
VCC      ->  5V (P9_05)
GND      ->  GND (P9_01)
DATA     ->  GPIO (P9_12)
```

#### Communication Protocol

```cpp
// DHT22 communication timing
bool readDHT22(int& humidity, int& temperature) {
    uint8_t data[5] = {0};
    
    // Start signal
    gpio_direction_output(GPIO_PIN, 0);
    usleep(18000);
    gpio_direction_input(GPIO_PIN);
    
    // Wait for response
    if (!waitForPinState(0, 80)) return false;
    if (!waitForPinState(1, 80)) return false;
    
    // Read 40 bits of data
    for (int i = 0; i < 40; i++) {
        if (!waitForPinState(0, 50)) return false;
        if (gpio_read(GPIO_PIN) == 0) {
            // 0 bit
            if (!waitForPinState(1, 28)) return false;
        } else {
            // 1 bit
            if (!waitForPinState(1, 70)) return false;
            data[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    
    // Verify checksum
    if ((data[0] + data[1] + data[2] + data[3]) != data[4]) {
        return false;
    }
    
    // Calculate values
    humidity = data[0] * 256 + data[1];
    temperature = ((data[2] & 0x7F) * 256 + data[3]);
    
    // Check sign
    if (data[2] & 0x80) {
        temperature = -temperature;
    }
    
    return true;
}
```

## Sensor Configuration

### Device Tree Overlays

```dts
// sensors-overlay.dts
/dts-v1/;
/plugin/;

/ {
    compatible = "ti,j721e";
    
    fragment@0 {
        target = <&i2c0>;
        __overlay__ {
            status = "okay";
            
            // IMU
            imu@68 {
                compatible = "invensense,mpu6050";
                reg = <0x68>;
                interrupt-parent = <&gpio0>;
                interrupts = <14 0>;
            };
            
            // Temperature
            temp@48 {
                compatible = "ti,tmp102";
                reg = <0x48>;
            };
            
            // Pressure
            pressure@77 {
                compatible = "bosch,bmp180";
                reg = <0x77>;
            };
        };
    };
    
    fragment@1 {
        target = <&uart0>;
        __overlay__ {
            status = "okay";
            // GPS
            gps {
                compatible = "u-blox,neo-6m";
                current-speed = <9600>;
            };
        };
    };
};
```

### Sensor Configuration File

```json
{
  "sensors": [
    {
      "id": "imu-001",
      "type": "imu",
      "enabled": true,
      "interface": {
        "type": "i2c",
        "bus": 0,
        "address": 0x68
      },
      "config": {
        "accel_range": 2,
        "gyro_range": 1,
        "sample_rate": 100
      }
    },
    {
      "id": "gps-001",
      "type": "gps",
      "enabled": true,
      "interface": {
        "type": "uart",
        "device": "/dev/ttyS0",
        "baud_rate": 9600
      }
    },
    {
      "id": "temp-001",
      "type": "temperature",
      "enabled": true,
      "interface": {
        "type": "i2c",
        "bus": 0,
        "address": 0x48
      }
    },
    {
      "id": "pressure-001",
      "type": "pressure",
      "enabled": true,
      "interface": {
        "type": "i2c",
        "bus": 0,
        "address": 0x77
      }
    },
    {
      "id": "humidity-001",
      "type": "humidity",
      "enabled": true,
      "interface": {
        "type": "gpio",
        "pin": 17
      }
    }
  ]
}
```

## Data Processing

### Sensor Data Pipeline

```
1. Raw Data
   ├─> Sensor reading (raw values)
   └─> Timestamp

2. Calibration
   ├─> Offset correction
   ├─> Scale correction
   └─> Filtering

3. Conversion
   ├─> Raw to physical units
   └─> Unit conversion

4. Validation
   ├─> Range checking
   ├─> Rate checking
   └─> Consistency checking

5. Output
   ├─> JSON format
   ├─> CSV format
   └─> Binary format
```

### Calibration Procedures

```cpp
// IMU Calibration
void calibrateIMU() {
    const int SAMPLES = 100;
    double sumAccelX = 0, sumAccelY = 0, sumAccelZ = 0;
    double sumGyroX = 0, sumGyroY = 0, sumGyroZ = 0;
    
    for (int i = 0; i < SAMPLES; i++) {
        IMUData data = readIMU();
        sumAccelX += data.accel.x;
        sumAccelY += data.accel.y;
        sumAccelZ += data.accel.z;
        sumGyroX += data.gyro.x;
        sumGyroY += data.gyro.y;
        sumGyroZ += data.gyro.z;
        usleep(10000);
    }
    
    // Calculate offsets
    offsets.accelX = -sumAccelX / SAMPLES;
    offsets.accelY = -sumAccelY / SAMPLES;
    offsets.accelZ = 1.0 - (sumAccelZ / SAMPLES);  // Gravity = 1g
    offsets.gyroX = -sumGyroX / SAMPLES;
    offsets.gyroY = -sumGyroY / SAMPLES;
    offsets.gyroZ = -sumGyroZ / SAMPLES;
}
```

## Troubleshooting

### Common Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| No sensor detected | Wiring issue | Check connections |
| | Wrong address | Verify I2C address |
| | Power issue | Check voltage |
| | Driver not loaded | Check kernel modules |
| Invalid data | Calibration needed | Run calibration |
| | Interference | Check environment |
| | Sampling rate | Adjust rate |
| Slow response | Sample rate too low | Increase rate |
| | Buffer size | Increase buffer |

### Debugging Commands

```bash
# I2C detection
i2cdetect -y 0

# SPI detection
ls -la /dev/spidev*

# GPIO status
cat /sys/class/gpio/gpio*/value

# Sensor service status
systemctl status sensor-service
journalctl -u sensor-service -f

# Test sensors
sensor-service --test
device-manager --list
```
```

## docs/hardware/gpio-map.md

```markdown
# GPIO Map for BBB AI-64

## GPIO Pin Mapping

### 40-Pin Header (P8/P9)

```
┌─────────────────────────────────────────────────────────────┐
│                     BBB AI-64 GPIO Map                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  P9 Header (Top)                                            │
│  ┌───────┬───────┐  ┌───────┬───────┐                     │
│  │  P9_01 │ VCC   │  │ P9_02 │ VCC   │                     │
│  │  P9_03 │ 3.3V  │  │ P9_04 │ 3.3V  │                     │
│  │  P9_05 │ 5V    │  │ P9_06 │ GND   │                     │
│  │  P9_07 │ GPIO  │  │ P9_08 │ GPIO  │                     │
│  │  P9_09 │ GND   │  │ P9_10 │ GND   │                     │
│  │  P9_11 │ GPIO  │  │ P9_12 │ GPIO  │                     │
│  │  P9_13 │ GPIO  │  │ P9_14 │ GPIO  │                     │
│  │  P9_15 │ GPIO  │  │ P9_16 │ GPIO  │                     │
│  │  P9_17 │ GPIO  │  │ P9_18 │ GPIO  │                     │
│  │  P9_19 │ I2C   │  │ P9_20 │ I2C   │                     │
│  │  P9_21 │ SPI   │  │ P9_22 │ SPI   │                     │
│  │  P9_23 │ GPIO  │  │ P9_24 │ UART  │                     │
│  │  P9_25 │ GPIO  │  │ P9_26 │ UART  │                     │
│  │  P9_27 │ GPIO  │  │ P9_28 │ SPI   │                     │
│  │  P9_29 │ GPIO  │  │ P9_30 │ GND   │                     │
│  │  P9_31 │ SPI   │  │ P9_32 │ VCC   │                     │
│  │  P9_33 │ GPIO  │  │ P9_34 │ GND   │                     │
│  │  P9_35 │ GPIO  │  │ P9_36 │ GPIO  │                     │
│  │  P9_37 │ GPIO  │  │ P9_38 │ GPIO  │                     │
│  │  P9_39 │ GPIO  │  │ P9_40 │ GPIO  │                     │
│  └───────┴───────┘  └───────┴───────┘                     │
│                                                             │
│  P8 Header (Bottom)                                         │
│  ┌───────┬───────┐  ┌───────┬───────┐                     │
│  │  P8_01 │ GND   │  │ P8_02 │ GND   │                     │
│  │  P8_03 │ GPIO  │  │ P8_04 │ GPIO  │                     │
│  │  P8_05 │ GPIO  │  │ P8_06 │ GPIO  │                     │
│  │  P8_07 │ GPIO  │  │ P8_08 │ GPIO  │                     │
│  │  P8_09 │ GPIO  │  │ P8_10 │ GPIO  │                     │
│  │  P8_11 │ GPIO  │  │ P8_12 │ GPIO  │                     │
│  │  P8_13 │ GPIO  │  │ P8_14 │ GPIO  │                     │
│  │  P8_15 │ GPIO  │  │ P8_16 │ GPIO  │                     │
│  │  P8_17 │ GPIO  │  │ P8_18 │ GPIO  │                     │
│  │  P8_19 │ GPIO  │  │ P8_20 │ GPIO  │                     │
│  │  P8_21 │ GPIO  │  │ P8_22 │ GPIO  │                     │
│  │  P8_23 │ GPIO  │  │ P8_24 │ GPIO  │                     │
│  │  P8_25 │ GPIO  │  │ P8_26 │ GPIO  │                     │
│  │  P8_27 │ GPIO  │  │ P8_28 │ GPIO  │                     │
│  │  P8_29 │ GPIO  │  │ P8_30 │ GPIO  │                     │
│  │  P8_31 │ GPIO  │  │ P8_32 │ GPIO  │                     │
│  │  P8_33 │ GPIO  │  │ P8_34 │ GPIO  │                     │
│  │  P8_35 │ GPIO  │  │ P8_36 │ GPIO  │                     │
│  │  P8_37 │ GPIO  │  │ P8_38 │ GPIO  │                     │
│  │  P8_39 │ GPIO  │  │ P8_40 │ GPIO  │                     │
│  └───────┴───────┘  └───────┴───────┘                     │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## Detailed Pin Mapping

### P9 Header (Top)

| Pin | Name | Type | Function | Linux GPIO | Notes |
|-----|------|------|----------|------------|-------|
| 1 | GND | Power | Ground | - | |
| 2 | GND | Power | Ground | - | |
| 3 | 3.3V | Power | 3.3V | - | |
| 4 | 3.3V | Power | 3.3V | - | |
| 5 | 5V | Power | 5V | - | |
| 6 | GND | Power | Ground | - | |
| 7 | SDA | I2C | I2C Data | - | I2C0_SDA |
| 8 | SCL | I2C | I2C Clock | - | I2C0_SCL |
| 9 | GND | Power | Ground | - | |
| 10 | GND | Power | Ground | - | |
| 11 | UART4_RX | UART | UART RX | 30 | |
| 12 | UART4_TX | UART | UART TX | 31 | |
| 13 | GPIO | GPIO | General Purpose | 32 | |
| 14 | GPIO | GPIO | General Purpose | 33 | |
| 15 | GPIO | GPIO | General Purpose | 34 | |
| 16 | GPIO | GPIO | General Purpose | 35 | |
| 17 | SPI | SPI | SPI Data | - | SPI0_CS0 |
| 18 | SPI | SPI | SPI Clock | - | SPI0_SCLK |
| 19 | I2C | I2C | I2C Data | - | I2C1_SDA |
| 20 | I2C | I2C | I2C Clock | - | I2C1_SCL |
| 21 | SPI | SPI | SPI Data | - | SPI1_SCLK |
| 22 | SPI | SPI | SPI Data | - | SPI1_MISO |
| 23 | GPIO | GPIO | General Purpose | 36 | |
| 24 | GPIO | GPIO | General Purpose | 37 | |
| 25 | GPIO | GPIO | General Purpose | 38 | |
| 26 | GPIO | GPIO | General Purpose | 39 | |
| 27 | GPIO | GPIO | General Purpose | 40 | |
| 28 | SPI | SPI | SPI Data | - | SPI1_CS1 |
| 29 | GPIO | GPIO | General Purpose | 41 | |
| 30 | GND | Power | Ground | - | |
| 31 | SPI | SPI | SPI Data | - | SPI1_CS0 |
| 32 | VDD_ADC | Power | Analog | - | |
| 33 | GPIO | GPIO | General Purpose | 42 | |
| 34 | GND | Power | Ground | - | |
| 35 | GPIO | GPIO | General Purpose | 43 | |
| 36 | GPIO | GPIO | General Purpose | 44 | |
| 37 | GPIO | GPIO | General Purpose | 45 | |
| 38 | GPIO | GPIO | General Purpose | 46 | |
| 39 | GPIO | GPIO | General Purpose | 47 | |
| 40 | GPIO | GPIO | General Purpose | 48 | |

### P8 Header (Bottom)

| Pin | Name | Type | Function | Linux GPIO | Notes |
|-----|------|------|----------|------------|-------|
| 1 | GND | Power | Ground | - | |
| 2 | GND | Power | Ground | - | |
| 3 | GPIO | GPIO | General Purpose | 38 | |
| 4 | GPIO | GPIO | General Purpose | 39 | |
| 5 | GPIO | GPIO | General Purpose | 34 | |
| 6 | GPIO | GPIO | General Purpose | 35 | |
| 7 | GPIO | GPIO | General Purpose | 66 | |
| 8 | GPIO | GPIO | General Purpose | 67 | |
| 9 | GPIO | GPIO | General Purpose | 69 | |
| 10 | GPIO | GPIO | General Purpose | 68 | |
| 11 | GPIO | GPIO | General Purpose | 45 | |
| 12 | GPIO | GPIO | General Purpose | 44 | |
| 13 | GPIO | GPIO | General Purpose | 23 | |
| 14 | GPIO | GPIO | General Purpose | 26 | |
| 15 | GPIO | GPIO | General Purpose | 47 | |
| 16 | GPIO | GPIO | General Purpose | 46 | |
| 17 | GPIO | GPIO | General Purpose | 27 | |
| 18 | GPIO | GPIO | General Purpose | 65 | |
| 19 | GPIO | GPIO | General Purpose | 22 | |
| 20 | GPIO | GPIO | General Purpose | 63 | |
| 21 | GPIO | GPIO | General Purpose | 62 | |
| 22 | GPIO | GPIO | General Purpose | 37 | |
| 23 | GPIO | GPIO | General Purpose | 36 | |
| 24 | GPIO | GPIO | General Purpose | 33 | |
| 25 | GPIO | GPIO | General Purpose | 32 | |
| 26 | GPIO | GPIO | General Purpose | 61 | |
| 27 | GPIO | GPIO | General Purpose | 86 | |
| 28 | GPIO | GPIO | General Purpose | 88 | |
| 29 | GPIO | GPIO | General Purpose | 87 | |
| 30 | GPIO | GPIO | General Purpose | 89 | |
| 31 | GPIO | GPIO | General Purpose | 10 | |
| 32 | GPIO | GPIO | General Purpose | 11 | |
| 33 | GPIO | GPIO | General Purpose | 9 | |
| 34 | GPIO | GPIO | General Purpose | 8 | |
| 35 | GPIO | GPIO | General Purpose | 7 | |
| 36 | GPIO | GPIO | General Purpose | 6 | |
| 37 | GPIO | GPIO | General Purpose | 5 | |
| 38 | GPIO | GPIO | General Purpose | 4 | |
| 39 | GPIO | GPIO | General Purpose | 3 | |
| 40 | GPIO | GPIO | General Purpose | 2 | |

## GPIO Pin Usage Map

### Sensor Pin Allocations

| Sensor | Pin | GPIO | Function | Notes |
|--------|-----|------|----------|-------|
| IMU (MPU6050) | P9_19 | - | I2C1_SDA | I2C Data |
| | P9_20 | - | I2C1_SCL | I2C Clock |
| | P9_15 | 34 | INT | Interrupt |
| GPS | P9_24 | 37 | UART RX | Serial RX |
| | P9_26 | 39 | UART TX | Serial TX |
| | P9_15 | 34 | PPS | Pulse Per Second |
| Temperature (TMP102) | P9_19 | - | I2C1_SDA | I2C Data |
| | P9_20 | - | I2C1_SCL | I2C Clock |
| Pressure (BMP180) | P9_19 | - | I2C1_SDA | I2C Data |
| | P9_20 | - | I2C1_SCL | I2C Clock |
| Humidity (DHT22) | P9_12 | 31 | DATA | 1-Wire Data |
| Light (BH1750) | P9_19 | - | I2C1_SDA | I2C Data |
| | P9_20 | - | I2C1_SCL | I2C Clock |

### Actuator Allocations

| Actuator | Pin | GPIO | Function | Notes |
|----------|-----|------|----------|-------|
| LED | P8_12 | 44 | OUTPUT | Red LED |
| | P8_11 | 45 | OUTPUT | Green LED |
| | P8_16 | 46 | OUTPUT | Blue LED |
| Motor | P8_13 | 23 | PWM | PWM0 |
| | P8_19 | 22 | PWM | PWM1 |
| Relay | P8_14 | 26 | OUTPUT | Relay 1 |
| | P8_17 | 27 | OUTPUT | Relay 2 |
| Buzzer | P8_15 | 47 | PWM | PWM2 |
| Servo | P9_14 | 33 | PWM | PWM3 |

## GPIO Configuration

### Sysfs Interface

```bash
# Export GPIO
echo 44 > /sys/class/gpio/export

# Set direction
echo out > /sys/class/gpio/gpio44/direction

# Set value (0/1)
echo 1 > /sys/class/gpio/gpio44/value

# Read value
cat /sys/class/gpio/gpio44/value

# Unexport GPIO
echo 44 > /sys/class/gpio/unexport
```

### Device Tree Overlay

```dts
// gpio-overlay.dts
/dts-v1/;
/plugin/;

/ {
    compatible = "ti,j721e";
    
    fragment@0 {
        target = <&gpio0>;
        __overlay__ {
            // GPIO configuration
            pinctrl-names = "default";
            pinctrl-0 = <&gpio_pins>;
        };
    };
    
    fragment@1 {
        target = <&pmx0>;
        __overlay__ {
            gpio_pins: gpio_pins {
                pinctrl-single,pins = <
                    // P8_12 (GPIO0_44)
                    0x1c 0x07  // Output
                    // P8_11 (GPIO0_45)
                    0x18 0x07  // Output
                    // P9_15 (GPIO0_34)
                    0x14 0x17  // Input with pull-up
                >;
            };
        };
    };
};
```

## GPIO Programming

### C++ Example

```cpp
#include <fcntl.h>
#include <unistd.h>
#include <string>

class GPIO {
private:
    int fd;
    int pin;
    std::string path;
    
public:
    GPIO(int pin) : pin(pin) {
        path = "/sys/class/gpio/gpio" + std::to_string(pin);
    }
    
    bool exportPin() {
        int fd = open("/sys/class/gpio/export", O_WRONLY);
        if (fd < 0) return false;
        
        std::string pinStr = std::to_string(pin);
        write(fd, pinStr.c_str(), pinStr.length());
        close(fd);
        return true;
    }
    
    void setDirection(bool output) {
        std::string dirPath = path + "/direction";
        int fd = open(dirPath.c_str(), O_WRONLY);
        if (fd < 0) return;
        
        std::string dir = output ? "out" : "in";
        write(fd, dir.c_str(), dir.length());
        close(fd);
    }
    
    void setValue(bool value) {
        std::string valPath = path + "/value";
        int fd = open(valPath.c_str(), O_WRONLY);
        if (fd < 0) return;
        
        std::string val = value ? "1" : "0";
        write(fd, val.c_str(), val.length());
        close(fd);
    }
    
    bool getValue() {
        std::string valPath = path + "/value";
        int fd = open(valPath.c_str(), O_RDONLY);
        if (fd < 0) return false;
        
        char buffer[2];
        read(fd, buffer, 2);
        close(fd);
        return buffer[0] == '1';
    }
};
```

## Safety Guidelines

### Electrical Specifications

| Parameter | Min | Max | Unit |
|-----------|-----|-----|------|
| Input Voltage | 0 | 3.3 | V |
| Output Voltage | 0 | 3.3 | V |
| Input Current | - | 1 | mA |
| Output Current | - | 8 | mA |
| Pull-up Resistor | - | 10 | kΩ |

### Precautions

1. **Voltage Levels**: All GPIO pins are 3.3V logic level. Do not connect 5V signals directly.
2. **Current Limitations**: Maximum 8mA per pin, 100mA total.
3. **ESD Protection**: Handle with care to avoid ESD damage.
4. **Short Circuits**: Avoid shorting pins to GND or VCC.
5. **Pull-ups**: Some pins have internal pull-ups, check the datasheet.

## Troubleshooting

### Common GPIO Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| Pin not responding | Not exported | Export GPIO first |
| | Wrong direction | Check direction setting |
| | Conflict with other uses | Check pin usage |
| Unexpected values | Floating input | Add pull-up/pull-down |
| | Voltage level issue | Check voltage levels |
| Interference | Noise on line | Add shielding |
| | Long wires | Use shorter wires |
| Damage | Over voltage | Use level shifter |
| | Over current | Add current limiting |

### Debugging Commands

```bash
# List exported GPIOs
ls -la /sys/class/gpio/gpio*/

# Show GPIO status
cat /sys/class/gpio/gpio*/value

# Test with simple commands
echo 44 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio44/direction
echo 1 > /sys/class/gpio/gpio44/value
```

## docs/yocto/build-guide.md

```markdown
# Yocto Build Guide for BBB AI-64

## Overview

This guide covers the complete build process for the BBB AI-64 Yocto distribution.

## Prerequisites

### System Requirements

| Component | Requirement |
|-----------|-------------|
| CPU | 4+ cores |
| RAM | 8+ GB |
| Storage | 100+ GB free |
| OS | Ubuntu 22.04 LTS (recommended) |
| Network | Fast internet (10+ Mbps) |

### Required Packages (Ubuntu/Debian)

```bash
# Install essential packages
sudo apt-get update
sudo apt-get install -y \
    gawk wget git diffstat unzip texinfo gcc build-essential \
    chrpath socat cpio python3 python3-pip python3-pexpect \
    xz-utils debianutils iputils-ping python3-git python3-jinja2 \
    libegl1-mesa libsdl1.2-dev xterm python3-subunit mesa-common-dev \
    zstd liblz4-tool

# Install repo tool
mkdir -p ~/.local/bin
curl https://storage.googleapis.com/git-repo-downloads/repo > ~/.local/bin/repo
chmod a+x ~/.local/bin/repo
export PATH="$HOME/.local/bin:$PATH"
```

### Required Packages (Fedora/RHEL)

```bash
sudo dnf install -y \
    gawk wget git diffstat unzip texinfo gcc gcc-c++ \
    chrpath socat cpio python3 python3-pip python3-pexpect \
    xz-devel debianutils iputils python3-git python3-jinja2 \
    mesa-libEGL libSDL-devel xterm python3-subunit mesa-libGL-devel \
    zstd lz4
```

## Directory Setup

```bash
# Create project directory
mkdir -p ~/yocto/bbb-ai64
cd ~/yocto/bbb-ai64

# Directory structure
mkdir -p sources
mkdir -p build
mkdir -p downloads
mkdir -p sstate-cache
```

## Layer Setup

### Clone Layers

```bash
cd sources

# Poky (base)
git clone git://git.yoctoproject.org/poky

# OpenEmbedded
git clone git://git.openembedded.org/meta-openembedded

# Qt6
git clone https://github.com/meta-qt6/meta-qt6.git

# TI Platform
git clone https://git.ti.com/git/processor-sdk/meta-ti.git

# Security
git clone git://git.yoctoproject.org/meta-security

# Virtualization
git clone https://github.com/meta-virtualization/meta-virtualization.git

# SWUpdate
git clone https://github.com/sbabic/meta-swupdate.git

# Custom layer
git clone https://github.com/yourusername/meta-custom.git
```

### Setup Build Environment

```bash
cd poky
source oe-init-build-env ../../build
```

### Configure Layers

```bash
# Add layers
bitbake-layers add-layer ../meta-openembedded/meta-oe
bitbake-layers add-layer ../meta-openembedded/meta-python
bitbake-layers add-layer ../meta-openembedded/meta-networking
bitbake-layers add-layer ../meta-openembedded/meta-filesystems
bitbake-layers add-layer ../meta-qt6
bitbake-layers add-layer ../meta-ti
bitbake-layers add-layer ../meta-security
bitbake-layers add-layer ../meta-virtualization
bitbake-layers add-layer ../meta-swupdate
bitbake-layers add-layer ../meta-custom
```

## Configuration

### local.conf

```bitbake
# Machine selection
MACHINE = "bbbai64"

# Distribution
DISTRO = "poky"

# Build threads
BB_NUMBER_THREADS = "8"
PARALLEL_MAKE = "-j 8"

# Download and sstate cache
DL_DIR = "${TOPDIR}/../downloads"
SSTATE_DIR = "${TOPDIR}/../sstate-cache"

# Package management
PACKAGE_CLASSES = "package_rpm"

# Features
EXTRA_IMAGE_FEATURES = "debug-tweaks ssh-server-openssh tools-debug"
DISTRO_FEATURES:append = " systemd wayland opengl vulkan bluetooth wifi"

# Init system
VIRTUAL-RUNTIME_init_manager = "systemd"

# Additional packages
IMAGE_INSTALL:append = " \
    packagegroup-gui \
    packagegroup-sensors \
    sensor-service \
    device-manager \
    gui-app \
    docker \
    docker-compose \
    "
```

### bblayers.conf

```bitbake
BBPATH = "${TOPDIR}"
BBFILES ?= ""

BBLAYERS ?= " \
    ${TOPDIR}/../sources/poky/meta \
    ${TOPDIR}/../sources/poky/meta-poky \
    ${TOPDIR}/../sources/poky/meta-yocto-bsp \
    ${TOPDIR}/../sources/meta-openembedded/meta-oe \
    ${TOPDIR}/../sources/meta-openembedded/meta-python \
    ${TOPDIR}/../sources/meta-openembedded/meta-networking \
    ${TOPDIR}/../sources/meta-openembedded/meta-filesystems \
    ${TOPDIR}/../sources/meta-qt6 \
    ${TOPDIR}/../sources/meta-ti \
    ${TOPDIR}/../sources/meta-security \
    ${TOPDIR}/../sources/meta-virtualization \
    ${TOPDIR}/../sources/meta-swupdate \
    ${TOPDIR}/../sources/meta-custom \
    "
```

## Build Process

### Fetch Sources

```bash
# Fetch all sources
bitbake -c fetchall custom-image

# Or fetch specific recipe
bitbake -c fetchall linux-ti
```

### Build Image

```bash
# Build the image
bitbake custom-image

# Build with verbose output
bitbake -v custom-image

# Build specific tasks
bitbake -c compile linux-ti
bitbake -c deploy linux-ti
```

### Build SDK

```bash
# Build SDK
bitbake -c populate_sdk custom-image

# SDK location
# build/deploy/sdk/poky-glibc-x86_64-custom-image-core-image-sdk-1.0.sh
```

### Clean Build

```bash
# Clean specific recipe
bitbake -c cleanall custom-image

# Clean specific task
bitbake -c clean linux-ti

# Complete clean
rm -rf build/tmp
```

## Build Optimization

### ccache Configuration

```bash
# Install ccache
sudo apt-get install ccache

# Configure
export CCACHE_DIR=~/yocto/.ccache
ccache -M 20G
ccache -s

# Add to local.conf
INHERIT += "ccache"
CCACHE_MAXSIZE = "20G"
```

### Shared State Cache

```bash
# Use sstate-cache
SSTATE_DIR = "${TOPDIR}/../sstate-cache"

# Share with other builds
SSTATE_MIRRORS = "file://.* file:///path/to/common/sstate-cache/PATH"

# Enable sstate-cache validation
SSTATE_HASHEQUIV_REPORT_TASKDATA = "1"
```

### Parallel Builds

```bitbake
# local.conf
BB_NUMBER_THREADS = "8"
PARALLEL_MAKE = "-j 8"

# For specific recipes
PARALLEL_MAKE_pn-linux-ti = "-j 4"
```

## Debugging Builds

### Build Logs

```bash
# View build log
bitbake -e custom-image

# View task logs
less tmp/work/bbbai64-poky-linux/custom-image/1.0-r0/temp/log.do_*.log

# View bitbake output
bitbake -v custom-image 2>&1 | tee build.log
```

### Task Debugging

```bash
# List tasks for recipe
bitbake -c listtasks linux-ti

# Force task rerun
bitbake -C configure linux-ti

# Debug specific task
bitbake -D -v -c compile linux-ti
```

### Environment Inspection

```bash
# View environment
bitbake -e custom-image > build.env

# Show recipe versions
bitbake -s | grep linux

# Show package dependencies
bitbake -g custom-image
```

## Image Customization

### Add Packages

```bitbake
# In local.conf
IMAGE_INSTALL:append = " package1 package2"

# In custom-image.bb
IMAGE_INSTALL += " package3 package4"
```

### Remove Packages

```bitbake
# Remove packages
IMAGE_INSTALL:remove = " unwanted-package"

# Remove features
EXTRA_IMAGE_FEATURES:remove = "tools-debug"
```

### Custom Configuration

```bash
# Create custom layer
bitbake-layers create-layer meta-custom
bitbake-layers add-layer meta-custom

# Create custom recipe
mkdir -p meta-custom/recipes-custom/hello
echo "DESCRIPTION = 'Hello World'" > meta-custom/recipes-custom/hello/hello_1.0.bb
```

## Development Workflow

### Interactive Development

```bash
# Build SDK
bitbake -c populate_sdk custom-image

# Install SDK
./build/deploy/sdk/poky-glibc-x86_64-custom-image-core-image-sdk-1.0.sh

# Source SDK
source /opt/poky/1.0/environment-setup-cortexa72-poky-linux

# Develop application
$CC -o app main.c
```

### Recipe Development

```bash
# Create new recipe
recipetool create -o meta-custom/recipes-utils/helloworld helloworld

# Edit recipe
vi meta-custom/recipes-utils/helloworld/helloworld_1.0.bb

# Build recipe
bitbake helloworld

# Test recipe
bitbake -c devshell helloworld
```

## Common Issues

### Build Failures

| Issue | Solution |
|-------|----------|
| Out of memory | Increase RAM or swap |
| | Reduce threads: BB_NUMBER_THREADS = "4" |
| Missing dependencies | Check package installation |
| Network issues | Check proxy settings |
| | Use DL_DIR for downloads |
| Version conflicts | Check layer priorities |
| | Lock specific versions |
| Toolchain issues | Clean and rebuild |
| | Check SDK installation |

### Performance Issues

| Issue | Solution |
|-------|----------|
| Slow builds | Enable ccache |
| | Use sstate-cache |
| | Increase threads |
| Large downloads | Use DL_DIR |
| | Use local mirror |
| Disk space | Clean tmp dir |
| | Use sstate-cache |

## Release Build

### Production Build

```bash
# Configure for production
cat >> conf/local.conf << EOF
# Production settings
DISTRO = "poky"
DISTRO_FEATURES:remove = "debug-tweaks"
EXTRA_IMAGE_FEATURES:remove = "tools-debug tools-profile"
IMAGE_FEATURES:remove = "debug-tweaks"
EOF

# Build
bitbake custom-image

# Create release
./scripts/release.sh v1.0.0
```

### Image Signing

```bash
# Sign kernel
sign-kernel Image Image.signed

# Sign image
swupdate-sign -k private.pem -i custom-image.wic.gz

# Verify
swupdate-verify -k public.pem -i custom-image.wic.gz.signed
```

## Documentation

### Build Summary

```bash
# Show build information
bitbake -e | grep ^PN=
bitbake -e | grep ^PV=
bitbake -e | grep ^PR=

# Show installed packages
bitbake -s | grep ^${PN}

# Show image contents
bitbake -g custom-image
```

### Build Statistics

```bash
# Build time
bitbake -e | grep ^BB_VERSION
bitbake -e | grep ^BUILD_SYS
bitbake -e | grep ^TARGET_SYS

# Package statistics
bitbake -s | wc -l
```

## Advanced Topics

### Custom Kernel

```bash
# Configure kernel
bitbake -c menuconfig linux-ti

# Kernel config fragment
echo "CONFIG_FEATURE=y" > meta-custom/recipes-kernel/linux/linux-ti/feature.cfg

# Apply config fragment
KERNEL_CONFIG_FRAGMENTS:append = " ${WORKDIR}/feature.cfg"
```

### Device Tree Overlays

```dts
// Create overlay
/dts-v1/;
/plugin/;

/ {
    compatible = "ti,j721e";
    
    fragment@0 {
        target = <&i2c0>;
        __overlay__ {
            status = "okay";
            // Device configuration
        };
    };
};

// Build overlay
dtc -@ -I dts -O dtb -o custom.dtbo custom.dts
```

### OTA Updates

```bash
# Create SWUpdate image
swupdate -c swupdate.cfg -i update.swu

# Update
swupdate -i update.swu

# Check status
swupdate --status
```

## Resources

### Documentation

- [Yocto Project Reference Manual](https://docs.yoctoproject.org/ref-manual/index.html)
- [BitBake User Manual](https://docs.yoctoproject.org/bitbake/index.html)
- [TI Processor SDK Documentation](https://software-dl.ti.com/processor-sdk-linux/esd/docs/latest/linux/index.html)

### Community

- [Yocto Project Mailing List](https://lists.yoctoproject.org/g/yocto)
- [TI E2E Forum](https://e2e.ti.com/support/processors/f/processors-forum)
- [BeagleBoard Community](https://beagleboard.org/community)
```
