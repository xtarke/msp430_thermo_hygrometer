


#include <msp430.h>
#include "one_wire.h"
#include "bits.h"
#include "gpio.h"
#include "intrinsics.h"


/**
 * @brief  Read logic level of one wire pin
 * @param  Nenhum
 *
 * @retval Nenhum.
 */
static inline uint8_t test_dq(){
   return TST_BIT(PORT_IN(ONE_WIRE_PORT), ONE_WIRE_PIN);
}

/**
 * @brief  Change one wire pin to output mode
 * @param  Nenhum
 *
 * @retval Nenhum.
 */
static inline void dq_output(){
   SET_BIT(PORT_DIR(ONE_WIRE_PORT), ONE_WIRE_PIN);
}

/**
 * @brief  Change one wire pin to input mode
 * @param  Nenhum
 *
 * @retval Nenhum.
 */
static inline void dq_input(){
   /* Set as input */
   CLR_BIT(PORT_DIR(ONE_WIRE_PORT), ONE_WIRE_PIN);
   /* Enable pull/down resistors */
   // SET_BIT(PORT_REN(ONE_WIRE_PORT), ONE_WIRE_PIN);
   /* Enable pull up */
   // SET_BIT(PORT_OUT(ONE_WIRE_PORT), ONE_WIRE_PIN);
}

/**
 * @brief  Force dq pin to 0.
 * @param  Nenhum
 *
 * @retval Nenhum.
 */
static inline void clear_dq() {
    CLR_BIT(PORT_OUT(ONE_WIRE_PORT),ONE_WIRE_PIN);
}

/**
 * @brief  Force dq pin to 1.
 * @param  Nenhum
 *
 * @retval Nenhum.
 */
static inline void set_dq() {
    SET_BIT(PORT_OUT(ONE_WIRE_PORT),ONE_WIRE_PIN);
}

uint8_t reset_1w()
{
	dq_output();
	clear_dq();
	/* 500us : 16Mhz -> 8000 cycles*/
    __delay_cycles(17000); 

    dq_input();
	/* 40us : 16Mhz -> 640 cycles*/
    __delay_cycles(400);

	if (test_dq())
	 	return 1;

	__delay_cycles(1280);
    
    if (!test_dq())
	 	return 2;
	
    __delay_cycles(1280);

	return 0;
}
/**
 * @brief  Read one wire byte.
 * @param  Nenhum
 *
 * @retval Nenhum.
 */
uint8_t read_byte_1w()
{
	uint8_t i, dado = 0;

	for (i=0; i < 8; i++) {

		while (!test_dq());
	    __delay_cycles(480);
        //_delay_us(30);

		if (test_dq())
			SET_BIT(dado, (1 << (7-i)));

		while (test_dq());
	}

	return (dado);
}
