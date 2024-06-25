#include "dht22.h"
#include "one_wire.h"

uint8_t dht11_data[4];

uint8_t dht_response() {

	uint8_t i;
	uint8_t sum = 0;

	if (reset_1w())
		return 7;

	for(i=0; i < 4; i++) {
		dht11_data[i]  = read_byte_1w();
		sum += dht11_data[i];
	}
	return ((sum == read_byte_1w()));
}

uint16_t get_temp(){
    return dht11_data[2] << 8 | dht11_data[3];
}

uint16_t get_humid(){
    return dht11_data[0] << 8 | dht11_data[1];
}
