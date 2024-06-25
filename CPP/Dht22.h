/*
 * Dht22.h
 *
 *  Created on: Jun 18, 2024
 *      Author: xtarke
 */

#ifndef DHT22_H_
#define DHT22_H_

#include <stdint.h>

#include "OneWire.h"

class Dht22: public OneWire
{
public:
    Dht22();

    uint8_t dht_response();
    uint16_t get_temp();
    uint16_t get_humid();

private:
    uint8_t dht11_data[4];

};

#endif /* DHT22_H_ */
