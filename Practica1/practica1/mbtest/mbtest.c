/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus/modbus.h>

//#include "mbtest.h"

char *error_msg[] = {
    "MODBUS_EXCEPTION_ILLEGAL_FUNCTION",
    "MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS",
    "MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE",
    "MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE",
    "MODBUS_EXCEPTION_ACKNOWLEDGE",
    "MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY",
    "MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE",
    "MODBUS_EXCEPTION_MEMORY_PARITY",
    "MODBUS_EXCEPTION_NOT_DEFINED",
    "MODBUS_EXCEPTION_GATEWAY_PATH",
    "MODBUS_EXCEPTION_GATEWAY_TARGET",
    "MODBUS_EXCEPTION_MAX"
};

int main(int argc, char *argv[]) {
    modbus_t *ctx;
    int rc, i;

    ctx = modbus_new_tcp("192.168.7.2", 502);

    if (ctx == NULL) {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }

    // modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx,
                              MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    u_int8_t tab_rp_bits[1];
    u_int16_t tab_rp_registers[1];

    int tab_rp_bits_size = sizeof(tab_rp_bits)/sizeof(tab_rp_bits[0]);
    int tab_rp_registers_size = sizeof(tab_rp_registers)/sizeof(tab_rp_registers[0]);

    for(i = 0; i< 6; i++) {
        printf("Address: 0x%02d\n\n", i);

        // 0x01 Read Coils
        rc = modbus_read_bits(ctx, i, 1, tab_rp_bits);
        printf("1/6 modbus_read_bits: ");
        if (rc != 1) {
            printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
        } else {
            printf("OK (nb points %d)\n", rc);
        }

        // 0x02 Read Discrete Inputs
        rc = modbus_read_input_bits(ctx, i, tab_rp_bits_size, tab_rp_bits);
        printf("2/6 modbus_read_input_bits: ");
        if (rc != tab_rp_bits_size) {
            printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
        } else {
            printf("OK (nb points %d)\n", rc);
        }

        // 0x03 Read Holding Registers
        rc = modbus_read_registers(ctx, i, tab_rp_registers_size, tab_rp_registers);
        printf("3/6 modbus_read_registers: ");
        if (rc != tab_rp_registers_size) {
            printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
        } else {
            printf("OK (nb points %d)\n", rc);
        }

        // 0x04 Read Input Registers
        rc = modbus_read_input_registers(ctx, i, tab_rp_registers_size, tab_rp_registers);
        printf("4/6 modbus_read_input_registers: ");
        if (rc != tab_rp_registers_size) {
            printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
        } else {
            printf("OK (nb points %d)\n", rc);
        }

        // 0x05 Write Single Coil
        rc = modbus_write_bit(ctx, i, ON);
        printf("5/6 modbus_write_bit: ");
        if (rc != 1) {
            printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
        } else {
            printf("OK\n");
        }

        // 0x06 Write Single Register
        rc = modbus_write_register(ctx, i, 0xff);
        printf("6/6 modbus_write_register: ");
        if (rc != 1) {
            printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
        } else {
            printf("OK\n");
        }
    }

    // Close the connection
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
