# BBB AI-64 v1.0 Release

## Release Information

| Property | Value |
|----------|-------|
| Version | 1.0.0 |
| Date | 2024-06-26 |
| Status | Stable |
| Type | Initial Release |

## Release Notes

### Features
- Initial Yocto build for BBB AI-64
- Qt6 GUI application
- Sensor service framework
- Device manager
- I2C/SPI/GPIO support
- Basic networking
- Systemd init

### Known Issues
- GUI performance can be improved
- Sensor calibration not persistent
- OTA updates not yet supported
- WiFi driver may be unstable

## Installation Instructions

### 1. Download Files
```
custom-image-bbbai64.wic.gz
custom-image-bbbai64.manifest
```

### 2. Flash to SD Card
```bash
# Decompress and flash
gunzip -c custom-image-bbbai64.wic.gz | sudo dd of=/dev/sdX bs=4M status=progress
sync
```

### 3. Boot the Board
1. Insert SD card
2. Connect power
3. Connect serial console (115200, 8N1)

### 4. First Boot Setup
```bash
# Login as root
login: root

# Configure network
ifconfig eth0 up
udhcpc -i eth0
```

## Component Versions

| Component | Version |
|-----------|---------|
| Linux Kernel | 6.1.30 |
| U-Boot | 2023.04 |
| Qt6 | 6.4.2 |
| Systemd | 251.4 |

## Development

### Build Environment
```bash
# Setup build environment
source /opt/poky/1.0/environment-setup-cortexa72-poky-linux

# Build application
$CC -o app main.c
```

## Known Limitations

1. **Boot Time**: Approximately 8 seconds
2. **Memory Usage**: ~250MB idle
3. **Storage**: ~600MB for rootfs
4. **Sensor Support**: Limited to I2C sensors

## Upgrade Path

### To v1.1
```bash
# Download v1.1 image
wget http://update.server/custom-image-1.1.0.wic.gz

# Flash to SD card
sudo dd if=custom-image-1.1.0.wic.gz of=/dev/mmcblk0 bs=4M status=progress
sync
reboot

