/*
 * Dht22.cpp
 *
 *  Created on: Jun 18, 2024
 *      Author: Renan Augusto Starke
 */

#include <Dht22.h>

Dht22::Dht22()
{
    // TODO Auto-generated constructor stub

}

uint8_t Dht22::dht_response() {

    uint8_t i;
    uint8_t sum = 0;

    if (OneWire::reset_1w())
        return 7;

    for(i=0; i < 4; i++) {
        dht11_data[i]  = OneWire::read_byte_1w();
        sum += dht11_data[i];
    }
    return ((sum == read_byte_1w()));
}

uint16_t Dht22::get_temp(){
    return dht11_data[2] << 8 | dht11_data[3];
}

uint16_t Dht22::get_humid(){
    return dht11_data[0] << 8 | dht11_data[1];
}

