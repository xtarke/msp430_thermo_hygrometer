/*
 * main.c
 *
 *  Created on: Jun 06, 2024
 *      Author: Renan Augusto Starke
 *      Instituto Federal de Santa Catarina
 *
 *      - OLED thermo hygrometer using MSP430, DTH22 and SSD1306 OLED display
 *
 *           OLED SSD1306              MSP430F247
 *             +-------+           +-------------------+
 *             |    SDA|<  -|---+->|P3.1/UCB0SDA       |        
 *             |       |    |      |                   |         DHT22
 *             |       |    |      |                   |       +-------+
 *             |       |    |      |                   |       |       | 
 *             |    SCL|<----+-----|P3.2/UCB0SCL   P2.0| <---> |DQ     |
 *              -------            |                   |       +-------+
 *
 */

/* System includes */
#include <string.h>
#include <msp430.h>
#include <stdint.h>
#include "intrinsics.h"

/* Project includes */
#include "i2c_master_f247.h"
#include "bits.h"
#include "gpio.h"
#include "one_wire.h"
#include "dht22.h"
#include "ssd1366.h"

#define LED BIT4
#define LED_PORT P3


/**
  * @brief  Configura sistema de clock para usar o Digitally Controlled Oscillator (DCO).
  *         Utililiza-se as calibrações internas gravadas na flash.
  *         Exemplo baseado na documentação da Texas: msp430g2xxx3_dco_calib.c  *
  * @param  none
  *
  * @retval none
  */
void init_clock_system(){

#ifdef CLOCK_1MHz
    /* Se calibração foi apagada, para aplicação */
    if (CALBC1_1MHZ==0xFF)
        while(1);
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
#endif

#ifdef CLOCK_8MHz

    /* Se calibração foi apagada, para aplicação */
    if (CALBC1_8MHZ==0xFF)
        while(1);

    DCOCTL = 0;
    BCSCTL1 = CALBC1_8MHZ;
    DCOCTL = CALDCO_8MHZ;

    /* Outras fonte de clock devem ser configuradas também *
     * de acordo com a aplicação  */
#endif

#ifdef CLOCK_12MHz
    /* Se calibração foi apagada, para aplicação */
    if (CALBC1_12MHZ==0xFF)
        while(1);
    DCOCTL = 0;
    BCSCTL1 = CALBC1_12MHZ;
    DCOCTL = CALDCO_12MHZ;
#endif

#ifdef CLOCK_16MHz
    /* Se calibração foi apagada, para aplicação */
    if (CALBC1_16MHZ==0xFF)
        while(1);
    DCOCTL = 0;
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;
#endif
}

/**
  * @brief  Configura temporizador watchdog.
  *
  * @param  none
  *
  * @retval none
  */
void config_wd_as_timer(){
    /* Configura Watch dog como temporizador:
     *
     * WDT_ADLY_250 <= (WDTPW+WDTTMSEL+WDTCNTCL+WDTSSEL+WDTIS0)
     *
     * WDTPW -> "Senha" para alterar confgiuração.
     * WDTTMSEL -> Temporizador ao invés de reset.
     * WDTSSEL -> Fonte de clock de ACLK
     * WDTIS1+WDTIS0 -> Clock / 8192
     *
     */
    WDTCTL = WDT_ADLY_1000;
    /* Ativa IRQ do Watchdog */
    IE1 |= WDTIE;
}


uint16_t temp = 0;
uint16_t humi = 0;

int main(void) {
    int i;
    /* Desliga Watchdog */
    WDTCTL = WDTPW | WDTHOLD;

    init_clock_system();
    init_i2c_master_mode();
    config_wd_as_timer();

    /* Debug LED */
    P3DIR |= BIT4;
    SET_BIT(P3OUT,BIT4);
 
    ssd1306_init();

    ssd1306_draw_pixel(16,16,WHITE_PIXEL);
    ssd1306_write_scaled_char(0,0, 'T',2);
    ssd1306_write_scaled_char(16,0, ':',2);
    ssd1306_write_scaled_char(0,24, 'h',2);
    ssd1306_write_scaled_char(16,24, ':',2);

    ssd1306_display_data();

    uint8_t digits[3];
    uint8_t checksum_valid;
    while (1){
        checksum_valid = dht_response();

        if (checksum_valid){
            temp =  get_temp();
            humi = get_humid();
        }
        else {
            temp = 0;
            humi = 0;
        }

        for (i=2; i >= 0; i--){
            digits[i] = temp % 10;
            temp = temp / 10;
        }

        ssd1306_write_scaled_char(32,0, ' ', 2);
        ssd1306_write_scaled_char(48,0, ' ', 2);
        ssd1306_write_scaled_char(64,0, ' ', 2);
        ssd1306_write_scaled_char(32,0, '0' + digits[0] ,2);
        ssd1306_write_scaled_char(48,0, '0' + digits[1] ,2);
        ssd1306_write_scaled_char(64,0, '.' ,2);
        ssd1306_write_scaled_char(80,0, '0' + digits[2] ,2);
        ssd1306_write_scaled_char(96,0, 'o',1);
        ssd1306_write_scaled_char(104,0, 'C',2);

         for (i=2; i >= 0; i--){
            digits[i] = humi % 10;
            humi = humi / 10;
        }

        ssd1306_write_scaled_char(32,24, ' ', 2);
        ssd1306_write_scaled_char(48,24, ' ', 2);
        ssd1306_write_scaled_char(64,24, ' ', 2);
        ssd1306_write_scaled_char(32,24, '0' + digits[0] ,2);
        ssd1306_write_scaled_char(48,24, '0' + digits[1] ,2);
        ssd1306_write_scaled_char(64,24, '.', 2);
        ssd1306_write_scaled_char(80,24, '0' + digits[2] ,2);
        ssd1306_write_scaled_char(96,24, '%', 2);

        ssd1306_display_data();

        __bis_SR_register(LPM0_bits + GIE);        
    }
    
}


/* ISR do watchdog: executado toda a vez que o temporizador estoura */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(WDT_VECTOR))) watchdog_timer (void)
#else
#error Compiler not supported!
#endif
{
    static uint16_t x = 0;

    if (x >= 10) {
        PORT_OUT(LED_PORT) ^= LED;
        x = 0;
        __bic_SR_register_on_exit(CPUOFF);
    }
    x++;
}
