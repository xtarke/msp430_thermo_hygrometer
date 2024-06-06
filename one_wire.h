#ifndef _ONE_WIRE_H
#define _ONE_WIRE_H

#include <stdint.h>

/* Hardware ports  */
#define ONE_WIRE_PORT P2
#define ONE_WIRE_PIN  BIT0

uint8_t reset_1w();
uint8_t read_byte_1w();


#endif
