/*
 * Battery.h
 *
 *  Created on: Jun 27, 2024
 *      Author: xtarke
 */

#ifndef BATTERY_H_
#define BATTERY_H_

#include <stdint.h>

class Battery
{
public:
    Battery();
    // virtual ~Battery();

    uint16_t get_voltage();

private:

    uint16_t voltage;

};

#endif /* BATTERY_H_ */
