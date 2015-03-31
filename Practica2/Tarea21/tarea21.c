#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus/modbus.h>
#include "tarea21.h"
#include "gpio.h"

#define LEDR 60
#define PULSADOR 50
#define LEDG "/sys/class/leds/beaglebone:green:usr%d/brightness"
#define EXPORT "/sys/class/gpio/export"
#define DIRECTION "/sys/class/gpio/gpio%d/direction" //60
#define VALUE "/sys/class/gpio/gpio%d/value"
#define UNEXPORT "/sys/class/gpio/unexport"

/*
void led(int a,int b)
{
    char buffer [50];
    sprintf(buffer,LEDG,a);
    int fd=open(buffer,O_WRONLY);
    if(fd!=0)
    {
        write(fd,b?"1":"0",1);
        close(fd);
    }
}
*/

int main(int argc, char*argv[])
{
    int *valor;
    int socket;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;
    int rc;
    int i;
    uint8_t *query;
    int header_length;

    /* By default */
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

    /* Examples from PI_MODBUS_300.pdf.
       Only the read-only input values are assigned. */

    /** INPUT STATUS **/
    modbus_set_bits_from_bytes(mb_mapping->tab_input_bits,
                               UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB,
                               UT_INPUT_BITS_TAB);

    /** INPUT REGISTERS **/
    for (i=0; i < UT_INPUT_REGISTERS_NB; i++) {
        mb_mapping->tab_input_registers[UT_INPUT_REGISTERS_ADDRESS+i] =
            UT_INPUT_REGISTERS_TAB[i];;
    }

    socket = modbus_tcp_listen(ctx, 1);
    modbus_tcp_accept(ctx, &socket);


    for (;;) {
        rc = modbus_receive(ctx, query);
        if (rc == -1) {
            /* Connection closed by the client or error */
            break;
        }

        /* Read holding registers */
        if (query[header_length] == 0x03) {
            if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 3)
                == UT_REGISTERS_NB_SPECIAL) {
                printf("Set an incorrect number of values\n");
                MODBUS_SET_INT16_TO_INT8(query, header_length + 3,
                                         UT_REGISTERS_NB_SPECIAL - 1);
            } else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
                == UT_REGISTERS_ADDRESS_SPECIAL) {
                printf("Reply to this special register address by an exception\n");
                modbus_reply_exception(ctx, query,
                                       MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY);
                continue;
            }
        }

        rc = gpio_export(PULSADOR);
        rc=gpio_set_dir(PULSADOR,1);
        rc=gpio_get_value(PULSADOR,&valor);

        //if ((dir && leds == 1) || (!dir && leds == 8))
        //    dir = !dir;


        //rc = modbus_write_register(ctx, address, dir ? leds >> 1 : leds << 1);
        rc = gpio_unexport(PULSADOR);

        rc = gpio_export(LEDR);
        rc=gpio_set_dir(LEDR,0);
        rc=gpio_set_value(LEDR,&valor);
        rc = gpio_unexport(LEDR);
    }
    printf("Quit the loop: %s\n", modbus_strerror(errno));
    close(socket);
    modbus_mapping_free(mb_mapping);
    free(query);
    modbus_free(ctx);
    return 0;
}
