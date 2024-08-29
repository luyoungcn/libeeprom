#include "eeprom.h"
#include "board_info.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>

unsigned char verbose = 0;

void print_data(const char *label, unsigned char *data, size_t len) {
    if (verbose == 0) {
        return;
    }
    printf("%s:\n", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void compare_data(unsigned char *write_data, unsigned char *read_data, size_t len) {
    int match = 1;
    for (size_t i = 0; i < len; i++) {
        if (write_data[i] != read_data[i]) {
            match = 0;
            break;
        }
    }
    if (match) {
        printf("Data match: SUCCESS\n");
    } else {
        printf("Data match: FAILURE\n");
    }
}

void measure_time(const char *label, int (*func)(int, unsigned int, unsigned char *, size_t), int fd, unsigned int mem_addr, unsigned char *data, size_t len) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    func(fd, mem_addr, data, len);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) * 1000 * 1000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("[%s] %d bytes: %f micro seconds\n", label, len, elapsed);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }

    int fd = eeprom_init(I2C_BUS, EEPROM_ADDR);
    if (fd < 0) {
        printf("eeprom_init failed\n");
        return 1;
    }

    printf("\n\n==========================================\n");
    printf("Test 1: Single Page Read/Write\n");
    printf("==========================================\n");
    unsigned char write_data_page[PAGE_SIZE];
    for (int i = 0; i < PAGE_SIZE; i++) {
        write_data_page[i] = i;
    }
    measure_time("Single Page Write Time", eeprom_write, fd, 0x0000, write_data_page, sizeof(write_data_page));
    unsigned char read_data_page[PAGE_SIZE];
    if (eeprom_read(fd, 0x0000, read_data_page, sizeof(read_data_page)) < 0) {
        close(fd);
        return 1;
    }
    print_data("Single Page Write Data", write_data_page, sizeof(write_data_page));
    print_data("Single Page Read Data", read_data_page, sizeof(read_data_page));
    compare_data(write_data_page, read_data_page, sizeof(write_data_page));

    printf("\n\n==========================================\n");
    printf("Test 2: Cross Page Read/Write\n");
    printf("==========================================\n");
    unsigned char write_data_cross_page[PAGE_SIZE * 2];
    for (int i = 0; i < PAGE_SIZE * 2; i++) {
        write_data_cross_page[i] = i;
    }
    measure_time("Cross Page Write Time", eeprom_write, fd, PAGE_SIZE - 10, write_data_cross_page, sizeof(write_data_cross_page));
    unsigned char read_data_cross_page[PAGE_SIZE * 2];
    if (eeprom_read(fd, PAGE_SIZE - 10, read_data_cross_page, sizeof(read_data_cross_page)) < 0) {
        close(fd);
        return 1;
    }
    print_data("Cross Page Write Data", write_data_cross_page, sizeof(write_data_cross_page));
    print_data("Cross Page Read Data", read_data_cross_page, sizeof(read_data_cross_page));
    compare_data(write_data_cross_page, read_data_cross_page, sizeof(read_data_cross_page));

    printf("\n\n==========================================\n");
    printf("Test 3: Full EEPROM Read/Write\n");
    printf("==========================================\n");
    unsigned char write_data_full[EEPROM_SIZE];
    for (int i = 0; i < EEPROM_SIZE; i++) {
        write_data_full[i] = i % 256;
    }
    measure_time("Full EEPROM Write Time", eeprom_write, fd, 0x0000, write_data_full, sizeof(write_data_full));
    unsigned char read_data_full[EEPROM_SIZE];
    if (eeprom_read(fd, 0x0000, read_data_full, sizeof(read_data_full)) < 0) {
        close(fd);
        return 1;
    }
    print_data("Full EEPROM Write Data", write_data_full, sizeof(write_data_full));
    print_data("Full EEPROM Read Data", read_data_full, sizeof(read_data_full));
    compare_data(write_data_full, read_data_full, sizeof(write_data_full));

    printf("\n\n==========================================\n");
    printf("Test 4: Full EEPROM Erase\n");
    printf("==========================================\n");
    unsigned char erase_data[EEPROM_SIZE];
    memset(erase_data, 0xFF, sizeof(erase_data));
    measure_time("Full EEPROM Erase Time", eeprom_write, fd, 0x0000, erase_data, sizeof(erase_data));
    unsigned char read_erase_data[EEPROM_SIZE];
    if (eeprom_read(fd, 0x0000, read_erase_data, sizeof(read_erase_data)) < 0) {
        close(fd);
        return 1;
    }
    print_data("Erase EEPROM Write Data", erase_data, sizeof(erase_data));
    print_data("Erase EEPROM Read Data", read_erase_data, sizeof(read_erase_data));
    compare_data(erase_data, read_erase_data, sizeof(erase_data));

    eeprom_deinit(fd);
    return 0;
}
