#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <modbus/modbus.h>

#include "mbserver.h"

#define LEDR 60
#define PULSADOR 50
#define LEDG "/sys/class/leds/beaglebone:green:usr%d/brightness"
#define DIRECTION "/sys/class/gpio/gpio%d/direction"
#define VALUE "/sys/class/gpio/gpio%d/value"

void led(int a, int b);
int gpio_export(unsigned int gpio);
int gpio_unexport(unsigned int gpio);
int gpio_set_value(unsigned int gpio, unsigned int value);
int gpio_get_value(unsigned int gpio, unsigned int *value);
int gpio_set_dir(unsigned int gpio, unsigned int out_flag);

int main(int argc, char*argv[]) {
    int socket;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;
    uint8_t *query;
    int header_length;
    int rc, i;
    uint8_t bits;             // ledr
    unsigned int input_bits; // pulsador
    uint16_t registers;  // leds

    printf("Modbus TCP server\n");

    ctx = modbus_new_tcp("0.0.0.0", 502);
    query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
    header_length = modbus_get_header_length(ctx);

    modbus_set_debug(ctx, TRUE);

    mb_mapping = modbus_mapping_new(
        UT_BITS_ADDRESS + UT_BITS_NB,
        UT_INPUT_BITS_ADDRESS + UT_INPUT_BITS_NB,
        UT_REGISTERS_ADDRESS + UT_REGISTERS_NB,
        UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB);

    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    rc = gpio_export(PULSADOR);
    rc |= gpio_set_dir(PULSADOR, 0);
    if (rc) {
        perror("Error configurando pulsador\n");
    }

    rc = gpio_export(LEDR);
    rc |= gpio_set_dir(LEDR, 1);
    if (rc) {
        perror("Error configurando led rojo\n");
    }

    socket = modbus_tcp_listen(ctx, 1);
    while(1) {
        modbus_tcp_accept(ctx, &socket);
        while(1) {
            rc = modbus_receive(ctx, query);
            if (rc == -1) {
                fprintf(stderr, "Connection closed by the client or error\n");
                break;
            }
            rc = modbus_reply(ctx, query, rc, mb_mapping);
            if (rc == -1) {
                break;
            }


            // Pulsador
            rc = gpio_get_value(PULSADOR, &input_bits);

            if(rc == 0) {
               mb_mapping->tab_input_bits[UT_INPUT_BITS_ADDRESS]=input_bits;
            }

            // Led
            bits = mb_mapping->tab_bits[UT_BITS_ADDRESS];
            rc = gpio_set_value(LEDR, bits);

            // Leds
            registers=mb_mapping->tab_registers[UT_REGISTERS_ADDRESS];

            for(i=0; i<4; i++) {
                led(i, registers & (0x1 << i));
            }

        }
    }
    printf("Quit the loop: %s\n", modbus_strerror(errno));

    close(socket);
    modbus_mapping_free(mb_mapping);
    free(query);
    modbus_free(ctx);

    return 0;
}

void led(int a, int b) {
    char buffer [50];
    sprintf(buffer, LEDG, a);
    int fd = open(buffer,O_WRONLY);
    if(fd != 0) {
        write(fd, b ? "1":"0", 1);
        close(fd);
    }
}

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

int gpio_export(unsigned int gpio) {
    int fd, len;
    char buf[MAX_BUF];

    fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
    if (fd < 0) {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);

    return 0;
}

int gpio_unexport(unsigned int gpio) {
    int fd, len;
    char buf[MAX_BUF];

    fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
    if (fd < 0) {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);
    return 0;
}

int gpio_set_dir(unsigned int gpio, unsigned int out_flag) {
    int fd;
    char buf[MAX_BUF];

    snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/direction");
        return fd;
    }

    if (out_flag)
        write(fd, "out", 4);
    else
        write(fd, "in", 3);

    close(fd);
    return 0;
}

int gpio_set_value(unsigned int gpio, unsigned int value) {
    int fd;
    char buf[MAX_BUF];

    snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/set-value");
        return fd;
    }

    if (value)
        write(fd, "1", 2);
    else
        write(fd, "0", 2);

    close(fd);
    return 0;
}

int gpio_get_value(unsigned int gpio, unsigned int *value) {
    int fd, len;
    char buf[MAX_BUF];
    char ch;

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY);
    if (fd < 0) {
        perror("gpio/get-value");
        return fd;
    }

    read(fd, &ch, 1);

    if (ch != '0') {
        *value = 1;
    } else {
        *value = 0;
    }

    close(fd);
    return 0;
}
