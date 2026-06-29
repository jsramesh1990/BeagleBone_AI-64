/**
 * Sensor Reader Example for BBB AI-64
 * 
 * This program demonstrates reading sensor data
 * from I2C sensors on the BBB AI-64.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

/**
 * I2C device configuration
 */
typedef struct {
    char device[32];
    int address;
    int fd;
} I2CDevice;

/**
 * Open I2C device
 */
int i2c_open(I2CDevice *dev) {
    dev->fd = open(dev->device, O_RDWR);
    if (dev->fd < 0) {
        perror("Failed to open I2C device");
        return -1;
    }
    
    if (ioctl(dev->fd, I2C_SLAVE, dev->address) < 0) {
        perror("Failed to set I2C address");
        close(dev->fd);
        return -1;
    }
    
    return 0;
}

/**
 * Write to I2C device
 */
int i2c_write(I2CDevice *dev, uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    if (write(dev->fd, buffer, 2) != 2) {
        perror("Failed to write to I2C device");
        return -1;
    }
    return 0;
}

/**
 * Read from I2C device
 */
int i2c_read(I2CDevice *dev, uint8_t reg, uint8_t *value) {
    if (write(dev->fd, &reg, 1) != 1) {
        perror("Failed to write register");
        return -1;
    }
    
    if (read(dev->fd, value, 1) != 1) {
        perror("Failed to read from I2C device");
        return -1;
    }
    
    return 0;
}

/**
 * Close I2C device
 */
void i2c_close(I2CDevice *dev) {
    if (dev->fd >= 0) {
        close(dev->fd);
        dev->fd = -1;
    }
}

/**
 * Detect I2C devices
 */
void detect_devices() {
    printf("Scanning I2C bus 0...\n");
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    
    for (int addr = 0x00; addr <= 0x7F; addr++) {
        if (addr % 16 == 0) {
            printf("%02x: ", addr);
        }
        
        I2CDevice dev;
        snprintf(dev.device, sizeof(dev.device), "/dev/i2c-0");
        dev.address = addr;
        dev.fd = -1;
        
        if (i2c_open(&dev) == 0) {
            printf("%02x ", addr);
            i2c_close(&dev);
        } else {
            printf("-- ");
        }
        
        if (addr % 16 == 15) {
            printf("\n");
        }
    }
}

/**
 * Read temperature from TMP102
 */
void read_temperature() {
    I2CDevice dev;
    snprintf(dev.device, sizeof(dev.device), "/dev/i2c-0");
    dev.address = 0x48;  // TMP102 default address
    dev.fd = -1;
    
    if (i2c_open(&dev) < 0) {
        return;
    }
    
    // Read temperature register
    uint8_t msb, lsb;
    if (i2c_read(&dev, 0x00, &msb) == 0 &&
        i2c_read(&dev, 0x01, &lsb) == 0) {
        
        // Convert to temperature (12-bit)
        int16_t temp_raw = (msb << 4) | (lsb >> 4);
        if (temp_raw & 0x0800) {
            temp_raw |= 0xF000;
        }
        float temp = temp_raw * 0.0625;
        
        printf("Temperature: %.2f °C\n", temp);
    }
    
    i2c_close(&dev);
}

/**
 * Main entry point
 */
int main(int argc, char *argv[]) {
    printf("\n");
    printf("========================================\n");
    printf("  BBB AI-64 Sensor Reader\n");
    printf("========================================\n");
    printf("\n");
    
    // Detect devices
    detect_devices();
    printf("\n");
    
    // Read temperature
    printf("Reading temperature...\n");
    read_temperature();
    printf("\n");
    
    printf("Sensor reader complete!\n");
    return 0;
}
