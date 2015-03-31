#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define LEDR 60
#define PULSADOR 50
#define LEDG "/sys/class/leds/beaglebone:green:usr%d/brightness"
#define DIRECTION "/sys/class/gpio/gpio%d/direction"
#define VALUE "/sys/class/gpio/gpio%d/value"

void led(int a, int b);
int gpio_export(unsigned int gpio);
int gpio_unexport(unsigned int gpio);
int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
int gpio_set_value(unsigned int gpio, unsigned int value);
int gpio_get_value(unsigned int gpio, unsigned int *value);
int gpio_set_edge(unsigned int gpio, char *edge);
int gpio_fd_open(unsigned int gpio);
int gpio_fd_close(int fd);

int main(int argc, char*argv[]) {
    int rc;
    unsigned int led_idx, dir, valor;

    led_idx = 0;

    rc = gpio_export(LEDR);
    rc |= gpio_set_dir(LEDR, 1);
    rc = gpio_export(PULSADOR);
    rc |= gpio_set_dir(PULSADOR, 0);
    while (1) {
        rc |= gpio_get_value(PULSADOR, &valor);
        //printf("El valor del pulsador es %d",valor);
        if (rc) {
            printf("READ ERROR: ");
            printf(VALUE, PULSADOR);
            printf("\n");
        }

        rc |= gpio_set_value(LEDR, valor);
        if (rc) {
            printf("WRITE ERROR: ");
            printf(VALUE, PULSADOR);
            printf("\n");
        }

        led(led_idx, 0);
        if ((dir && led_idx <= 0) || (!dir && led_idx >= 3))
            dir = !dir;

        led_idx = dir ? led_idx - 1 : led_idx + 1;
        led(led_idx, 1);
        printf("led_idx: %02d\n", led_idx);

        usleep(250000);
    }

    rc |= gpio_unexport(PULSADOR);
    rc |= gpio_unexport(LEDR);

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
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
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
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

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
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

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

int gpio_set_edge(unsigned int gpio, char *edge) {
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/set-edge");
        return fd;
    }

    write(fd, edge, strlen(edge) + 1);
    close(fd);
    return 0;
}

int gpio_fd_open(unsigned int gpio) {
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY | O_NONBLOCK );
    if (fd < 0) {
        perror("gpio/fd_open");
    }
    return fd;
}

int gpio_fd_close(int fd) {
    return close(fd);
}
