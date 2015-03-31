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
    int rc, cnt, dir = 0;

    ctx = modbus_new_tcp("192.168.7.2", 502);
    // modbus_read_bits         addr:0 -> led
    // modbus_read_input_bits   addr:0 -> pulsador
    // modbus_read_registers    addr:0 -> 4 leds

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

    u_int8_t led;
    u_int8_t pushb;
    u_int16_t leds;
    const int address = 0;

    rc = modbus_write_register(ctx, address, 0x1);
    if (rc != 1) {
        //printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
    }

    while(1) {

        rc = modbus_read_bits(ctx, address, 1, &led);
        if (rc != 1) {
            //printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
        } else {
            //printf("Led: %s\n", led? "OFF":"ON");
        }

        rc = modbus_read_input_bits(ctx, address, 1, &pushb);
        if (rc != 1) {
            //printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
        } else {
            rc = modbus_write_bit(ctx, address, pushb);
            //printf("Button: %s\n", pushb? "OFF":"ON");
        }
        rc = modbus_read_registers(ctx, address, 1, &leds);
        if (rc != 1) {
            //printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
        } else {
            //printf("Leds: %d\n", leds);
            if ((dir && leds == 1) || (!dir && leds == 8))
                dir = !dir;
        }
        rc = modbus_write_register(ctx, address, dir ? leds >> 1 : leds << 1);
        if (rc != 1) {
            //printf("FAILED (nb points %d) MSG: %s\n", rc, error_msg[errno - MODBUS_ENOBASE]);
        }

        //for(cnt=0; cnt < 1000; cnt++);
        usleep(100000);
    }
    // Close the connection
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
