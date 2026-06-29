// sensor-reader.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

#define I2C_DEVICE "/dev/i2c-0"

// I2C device structure
typedef struct {
    int fd;
    int address;
} I2CDevice;

// Open I2C device
int i2c_open(I2CDevice *dev, int address) {
    dev->fd = open(I2C_DEVICE, O_RDWR);
    if (dev->fd < 0) {
        perror("Failed to open I2C device");
        return -1;
    }
    
    if (ioctl(dev->fd, I2C_SLAVE, address) < 0) {
        perror("Failed to set I2C address");
        close(dev->fd);
        return -1;
    }
    
    dev->address = address;
    return 0;
}

// Close I2C device
void i2c_close(I2CDevice *dev) {
    if (dev->fd >= 0) {
        close(dev->fd);
        dev->fd = -1;
    }
}

// Read from I2C device
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

// Read TMP102 temperature sensor
float read_tmp102(I2CDevice *dev) {
    uint8_t msb, lsb;
    
    if (i2c_read(dev, 0x00, &msb) < 0) {
        return -1;
    }
    if (i2c_read(dev, 0x01, &lsb) < 0) {
        return -1;
    }
    
    // Convert to temperature (12-bit)
    int16_t temp_raw = (msb << 4) | (lsb >> 4);
    if (temp_raw & 0x0800) {
        temp_raw |= 0xF000;
    }
    
    return temp_raw * 0.0625;
}

// Scan I2C bus for devices
void scan_i2c_bus() {
    printf("Scanning I2C bus...\n");
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    
    I2CDevice dev;
    dev.fd = -1;
    
    for (int addr = 0x03; addr <= 0x77; addr++) {
        if (addr % 16 == 0) {
            printf("%02x: ", addr);
        }
        
        if (i2c_open(&dev, addr) == 0) {
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

// Main function
int main(int argc, char *argv[]) {
    printf("\n");
    printf("========================================\n");
    printf("  BBB AI-64 Sensor Reader\n");
    printf("========================================\n");
    printf("\n");
    
    // Scan for devices
    scan_i2c_bus();
    printf("\n");
    
    // Read temperature from TMP102 (0x48)
    I2CDevice dev;
    if (i2c_open(&dev, 0x48) == 0) {
        float temp = read_tmp102(&dev);
        if (temp >= 0) {
            printf("Temperature: %.2f °C\n", temp);
            printf("Temperature: %.2f °F\n", temp * 9.0/5.0 + 32.0);
        }
        i2c_close(&dev);
    }
    
    printf("\n");
    printf("Sensor reader complete!\n");
    return 0;
}
