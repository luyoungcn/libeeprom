#include "eeprom.h"
#include "platform_linux.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>

int eeprom_init(const char *i2c_bus, int eeprom_addr)
{
    int fd = open(i2c_bus, O_RDWR);
    if (fd < 0) {
        perror("Open I2C bus failed");
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, eeprom_addr) < 0) {
        perror("Set I2C address failed");
        close(fd);
        return -1;
    }

    return fd;
}

int eeprom_write(int fd, unsigned int mem_addr, unsigned char *data, size_t len) {
    size_t remaining = len;
    size_t offset = 0;

    while (remaining > 0) {
        size_t to_write = PAGE_SIZE - (mem_addr % PAGE_SIZE);
        if (to_write > remaining) {
            to_write = remaining;
        }

        unsigned char buf[to_write + 2];
        buf[0] = (mem_addr >> 8) & 0xFF;
        buf[1] = mem_addr & 0xFF;
        memcpy(&buf[2], &data[offset], to_write);

        if (write(fd, buf, to_write + 2) != to_write + 2) {
            perror("Write to EEPROM failed");
            return -1;
        }

        usleep(WRITE_DELAY);

        mem_addr += to_write;
        offset += to_write;
        remaining -= to_write;
    }

    return 0;
}
int eeprom_read(int fd, unsigned int mem_addr, unsigned char *data, size_t len) {
    unsigned char addr[2];
    addr[0] = (mem_addr >> 8) & 0xFF;
    addr[1] = mem_addr & 0xFF;

    size_t remaining = len;
    size_t offset = 0;

    while (remaining > 0) {
        size_t to_read = remaining > SINGLE_READ_LIMIT ? SINGLE_READ_LIMIT : remaining;

        if (write(fd, addr, 2) != 2) {
            perror("Write address to EEPROM failed");
            return -1;
        }

        if (read(fd, data + offset, to_read) != to_read) {
            perror("Read from EEPROM failed");
            return -1;
        }

        mem_addr += to_read;
        addr[0] = (mem_addr >> 8) & 0xFF;
        addr[1] = mem_addr & 0xFF;
        offset += to_read;
        remaining -= to_read;
    }

    return 0;
}

int eeprom_deinit(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
    return 0;
}