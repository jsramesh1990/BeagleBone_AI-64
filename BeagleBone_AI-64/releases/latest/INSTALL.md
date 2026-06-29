# Installation Guide - BBB AI-64 v1.1.0

## Prerequisites

### Hardware
- BBB AI-64 board
- 8GB+ microSD card
- USB-C power supply (5V, 3A)
- USB-to-serial adapter
- Ethernet cable or WiFi

### Software
- Linux (recommended) or macOS
- balenaEtcher or dd command
- Screen or minicom for serial console

## Step 1: Download Files

Download the following files from the release:
```
custom-image-bbbai64.wic.gz
custom-image-bbbai64.wic.bmap
custom-image-bbbai64.manifest
checksums.sha256
```

## Step 2: Verify Checksums

```bash
# Download checksums.sha256
wget https://github.com/yourusername/BeagleBone_AI-64/releases/latest/checksums.sha256

# Verify
sha256sum -c checksums.sha256

# Should output:
# custom-image-bbbai64.wic.gz: OK
```

## Step 3: Prepare SD Card

### Option A: Using bmaptool (Recommended)
```bash
# Install bmaptool
sudo apt-get install bmap-tools

# Flash to SD card
sudo bmaptool copy custom-image-bbbai64.wic.gz /dev/sdX
```

### Option B: Using dd
```bash
# Decompress and flash
gunzip -c custom-image-bbbai64.wic.gz | sudo dd of=/dev/sdX bs=4M status=progress
sync
```

### Option C: Using balenaEtcher
1. Download balenaEtcher
2. Select the image file
3. Select the SD card
4. Click Flash!

## Step 4: Boot the Board

### Connect Serial Console
```bash
# Find serial device
ls /dev/ttyUSB*

# Connect with screen
screen /dev/ttyUSB0 115200

# Or with minicom
minicom -b 115200 -D /dev/ttyUSB0
```

### Power On
1. Insert SD card into BBB AI-64
2. Connect USB-C power
3. Watch serial console for boot messages

## Step 5: Initial Setup

### First Boot
```bash
# Login as root
login: root
password: (none)

# Set root password
passwd
New password: <your-password>
Retype new password: <your-password>

# Set hostname
echo "bbb-ai64" > /etc/hostname
hostname bbb-ai64
```

### Configure Network

#### WiFi
```bash
# Enable WiFi
connmanctl enable wifi

# Scan for networks
connmanctl scan wifi

# List networks
connmanctl services

# Connect (replace SSID)
connmanctl connect wifi_<SSID>_<hash>
Enter passphrase: <your-password>

# Verify connection
connmanctl state
```

#### Ethernet
```bash
# DHCP (automatic)
connmanctl enable ethernet

# Or manual IP
ifconfig eth0 192.168.1.100 netmask 255.255.255.0 up
route add default gw 192.168.1.1
echo "nameserver 8.8.8.8" > /etc/resolv.conf
```

### Install Updates (Optional)
```bash
# Check for updates
swupdate --check

# Apply updates
swupdate -d -i update.swu

# Reboot
reboot
```

## Step 6: Start Services

### Enable Services
```bash
# Enable services
systemctl enable sensor-service
systemctl enable device-manager
systemctl enable gui-app

# Start services
systemctl start sensor-service
systemctl start device-manager
systemctl start gui-app
```

### Verify Services
```bash
# Check status
systemctl status sensor-service
systemctl status device-manager
systemctl status gui-app

# Check logs
journalctl -u sensor-service -f
journalctl -u device-manager -f
journalctl -u gui-app -f
```

## Step 7: Test Sensors

### I2C Sensors
```bash
# Scan I2C bus
i2cdetect -y 0

# Test IMU
sensor-service --test imu

# Read IMU data
sensor-service --read imu

# Calibrate IMU
sensor-service --calibrate imu
```

### GPS
```bash
# Start GPS daemon
gpsd /dev/ttyS0 -F /var/run/gpsd.sock

# Read GPS data
cgps -s

# Or use sensor service
sensor-service --read gps
```

## Step 8: GUI Setup

### HDMI Display
1. Connect HDMI cable
2. HDMI output should be active
3. GUI starts automatically

### LCD Display
1. Connect LCD to display connector
2. Configure display in /boot/uEnv.txt
3. Reboot

### Touchscreen
1. Connect touchscreen
2. Calibrate with tslib:
```bash
ts_calibrate
```

## Step 9: Development Setup

### Install SDK (Optional)
```bash
# Copy SDK to target
scp poky-sdk-installer.sh root@<ip>:/root/

# Install SDK on target
./poky-sdk-installer.sh
```

### Build Applications
```bash
# On development machine
source /opt/poky/1.0/environment-setup-cortexa72-poky-linux
qmake project.pro
make

# Deploy to target
scp app root@<ip>:/usr/bin/
```

## Troubleshooting

### Boot Issues
| Symptom | Solution |
|---------|----------|
| No serial output | Check cable, baud rate, power |
| Kernel panic | Re-flash SD card |
| Root filesystem errors | fsck /dev/mmcblk0p2 |
| Boot loop | Check power supply |

### Network Issues
| Symptom | Solution |
|---------|----------|
| No WiFi | Check module, driver |
| No Ethernet | Check cable, IP config |
| DNS issues | Check /etc/resolv.conf |
| DHCP fails | Manual IP configuration |

### Sensor Issues
| Symptom | Solution |
|---------|----------|
| No I2C devices | Check wiring, power |
| Invalid data | Run calibration |
| Service fails | Check logs, configuration |

## Security

### Change Default Passwords
```bash
# Change root password
passwd

# Change other user passwords
useradd -m admin
passwd admin
```

### SSH Hardening
```bash
# Edit /etc/ssh/sshd_config
PermitRootLogin no
PasswordAuthentication no
PubkeyAuthentication yes

# Restart SSH
systemctl restart sshd
```

### Firewall
```bash
# Install nftables
apt-get install nftables

# Basic rules
nft add table inet filter
nft add chain inet filter input { type filter hook input priority 0; }
nft add rule inet filter input tcp dport 22 accept
nft add rule inet filter input tcp dport 80 accept
nft add rule inet filter input drop
```

## Next Steps

1. Install applications
2. Configure sensors
3. Set up OTA updates
4. Deploy to production

