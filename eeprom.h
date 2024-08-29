#ifndef EEPROM_H
#define EEPROM_H

#include <stddef.h>

#define EEPROM_M24512

#if defined(EEPROM_M24512)
#define PAGE_SIZE 128
#define WRITE_DELAY 4000
#define EEPROM_SIZE 65536
#endif

int eeprom_init(const char *i2c_bus, int eeprom_addr);
int eeprom_write(int fd, unsigned int mem_addr, unsigned char *data, size_t len);
int eeprom_read(int fd, unsigned int mem_addr, unsigned char *data, size_t len);
int eeprom_deinit(int fd);

#endif // EEPROM_H