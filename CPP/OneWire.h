/*
 * OneWire.h
 *
 *  Created on: Jun 18, 2024
 *      Author: Renan Augusto Starke
 */

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include <stdint.h>

/* Hardware ports  */
#define ONE_WIRE_PORT P2
#define ONE_WIRE_PIN  BIT0

class OneWire
{
public:
    OneWire();

    uint8_t reset_1w();
    uint8_t read_byte_1w();
};

#endif /* ONEWIRE_H_ */
