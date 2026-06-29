## Overview

This guide covers upgrading from v1.0 to v1.1. The upgrade can be done via OTA or manual flashing.

## Pre-Upgrade Checks

### 1. Backup Data
```bash
# Backup configuration
tar -czf config-backup.tar.gz /etc /var/lib

# Backup user data
tar -czf data-backup.tar.gz /home /data

# Copy backups to external storage
scp *.tar.gz user@backup-server:/backups/
```

### 2. Check Current Version
```bash
# Check version
cat /etc/version

# Should output:
# v1.0.0
```

### 3. Verify Resources
```bash
# Check free space
df -h /

# Should have at least 1GB free
```

## Option A: OTA Upgrade

### 1. Install SWUpdate
```bash
# Check if SWUpdate is installed
which swupdate

# If not installed, install it
apt-get install swupdate
```

### 2. Download Update
```bash
# Download update package
wget http://update.server/update-v1.1.0.swu

# Verify signature (optional)
wget http://update.server/update-v1.1.0.swu.sig
openssl dgst -sha256 -verify public.pem -signature update-v1.1.0.swu.sig update-v1.1.0.swu
```

### 3. Apply Update
```bash
# Apply update
swupdate -d -i update-v1.1.0.swu

# Monitor progress
tail -f /var/log/swupdate.log
```

### 4. Reboot
```bash
# Reboot to apply update
reboot

# Wait for boot to complete (approx. 1-2 minutes)
```

### 5. Verify Upgrade
```bash
# Check version
cat /etc/version

# Should output:
# v1.1.0

# Check services
systemctl status sensor-service
systemctl status device-manager
systemctl status gui-app
```

## Option B: Manual Upgrade

### 1. Download Image
```bash
# Download v1.1 image
wget http://update.server/custom-image-1.1.0.wic.gz
wget http://update.server/custom-image-1.1.0.wic.bmap
```

### 2. Flash to SD Card
```bash
# Flash with bmaptool
sudo bmaptool copy custom-image-1.1.0.wic.gz /dev/sdX# Or with dd
gunzip -c custom-image-1.1.0.wic.gz | sudo dd of=/dev/sdX bs=4M status=progress
sync
```

### 3. Boot New Image
1. Insert new SD card
2. Power on
3. Complete initial setup

### 4. Restore Data
```bash
# Copy backup to new system
scp config-backup.tar.gz root@<new-ip>:/tmp/
ssh root@<new-ip>

# Restore
tar -xzf /tmp/config-backup.tar.gz -C /
```

## Post-Upgrade Steps

### 1. Verify Services
```bash
# Check all services are running
systemctl status --all

# Check for errors
journalctl -p err -b
```

### 2. Test Sensors
```bash
# Test all sensors
sensor-service --test-all

# Read sensor data
sensor-service --read-all
```

### 3. Test GUI
```bash
# Start GUI
systemctl start gui-app

# Check display
echo $DISPLAY
```

### 4. Test OTA
```bash
# Check OTA status
swupdate --status

# Check for future updates
swupdate --check
```

## Rollback

### If Upgrade Fails

#### Using Backup SD Card
1. Insert previous SD card
2. Boot from it
3. Restore to previous version

#### Using Backup Image
```bash
# Flash previous image
sudo dd if=backup-image.wic of=/dev/mmcblk0 bs=4M status=progress
sync
reboot
```

#### Using SWUpdate Rollback
```bash
# Check available backups
swupdate --list-backups

# Rollback to previous version
swupdate --rollback
```

## Troubleshooting

### OTA Upgrade Issues

| Issue | Solution |
|-------|----------|
| Update package not found | Check URL and connectivity |
| Signature verification failed | Download public key |
| Insufficient space | Free up space |
| Network timeout | Use manual upgrade |

### Manual Upgrade Issues

| Issue | Solution |
|-------|----------|
| SD card write error | Check card, use different card |
| Image corruption | Re-download, verify checksum |
| Boot fails | Check U-Boot configuration |
| Services not starting | Check logs, configuration |

### Post-Upgrade Issues

| Issue | Solution |
|-------|----------|
| Services not starting | Check logs, start manually |
| Sensors not working | Check configuration, drivers |
| GUI not starting | Check display, X11/Wayland |
| Network issues | Check configuration, restart |

## Verification Checklist

- [ ] Version shows v1.1.0
- [ ] All services running
- [ ] Sensors working
- [ ] GUI operational
- [ ] Network connectivity
- [ ] OTA updates working
- [ ] Data restored
- [ ] Backups available

## Support

If you encounter issues:

1. Check logs: `journalctl -xe`
2. Check documentation
3. Search known issues
4. Contact community

## Success!

You have successfully upgraded to v1.1.0!

New features available:
- IMU sensor support
- GPS support
- OTA updates
- Docker support
- WiFi/Bluetooth
- Enhanced GUI
```

