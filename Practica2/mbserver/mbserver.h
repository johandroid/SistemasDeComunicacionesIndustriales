#ifndef MBSERVER_H
#define MBSERVER_H

#define SERVER_ID         17
#define INVALID_SERVER_ID 18

// modbus bits         addr:0 -> led
// modbus input_bits   addr:0 -> pulsador
// modbus registers    addr:0 -> 4 leds

const uint16_t UT_BITS_ADDRESS = 0x0;
const uint16_t UT_BITS_NB = 0x1;

const uint16_t UT_INPUT_BITS_ADDRESS = 0x0;
const uint16_t UT_INPUT_BITS_NB = 0x1;

const uint16_t UT_REGISTERS_ADDRESS = 0x0;
const uint16_t UT_REGISTERS_NB = 0x1;

const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0x0;
const uint16_t UT_INPUT_REGISTERS_NB = 0x0;

#endif // MBSERVER_H
