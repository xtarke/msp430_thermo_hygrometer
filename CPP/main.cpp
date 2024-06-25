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
#include <lib/i2c_master_f247_g2xxx.h>
#include <string.h>
#include <msp430.h>
#include <stdint.h>

/* Project low level includes */
#include "./lib/bits.h"
#include "./lib/gpio.h"

/* Project classes includes */
#include "SSD1306.h"
#include "Dht22.h"

#define CLOCK_16MHz
#define OLED_I2C_ADDRESS   0x3C

#define LED_PIN BIT0
#define LED_PORT P1

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

    /* Configure ACLK as VLO: ~12KHz
     * LFXT1 = VLO */
    BCSCTL3 |= LFXT1S_2;


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

/* OLED SSD1306 class instance: allocate RAM in bss section */
SSD1306 my_oled(OLED_I2C_ADDRESS);
Dht22 my_temp_sensor;

int main(void)
{
    /* Desliga Watchdog */
    WDTCTL = WDTPW | WDTHOLD;

    /* Debug LED */
    SET_BIT(PORT_DIR(LED_PORT), LED_PIN);
    SET_BIT(PORT_OUT(LED_PORT), LED_PIN);
    /* Low level system initialization */
    init_clock_system();
    init_i2c_master_mode();
    config_wd_as_timer();

    /* Init OLED display AFTER i2c initializaion  */
    my_oled.Init();

    my_oled.Refresh(SSD1306::LINE_1);
    my_oled.Refresh(SSD1306::LINE_2);
    my_oled.Refresh(SSD1306::LINE_3);
    my_oled.Refresh(SSD1306::LINE_4);

    uint16_t temp = 0;
    uint16_t humi = 0;
    uint8_t checksum_valid;
    uint8_t digits[3];

    while (1){
        checksum_valid = my_temp_sensor.dht_response();

        if (checksum_valid){
            temp =  my_temp_sensor.get_temp();
            humi = my_temp_sensor.get_humid();
        }
        else {
            temp = 0;
            humi = 0;
        }

        for (int i=2; i >= 0; i--){
            digits[i] = temp % 10;
            temp = temp / 10;
        }
        my_oled.ClearFrameBuffer();
        my_oled.WriteScaledChar(0, 0, 'T', 2);
        my_oled.WriteScaledChar(16,0, ':',2);
        my_oled.WriteScaledChar(32,0, ' ', 2);
        my_oled.WriteScaledChar(48,0, ' ', 2);
        my_oled.WriteScaledChar(64,0, ' ', 2);
        my_oled.WriteScaledChar(32,0, '0' + digits[0] ,2);
        my_oled.WriteScaledChar(48,0, '0' + digits[1] ,2);
        my_oled.WriteScaledChar(64,0, '.' ,2);
        my_oled.WriteScaledChar(80,0, '0' + digits[2] ,2);
        my_oled.WriteScaledChar(96,0, 'o',1);
        my_oled.WriteScaledChar(104,0, 'C',2);
        my_oled.Refresh(SSD1306::LINE_1);

        for (int i=2; i >= 0; i--){
            digits[i] = humi % 10;
            humi = humi / 10;
        }

        my_oled.ClearFrameBuffer();
        my_oled.WriteScaledChar(0, 0, 'h',2);
        my_oled.WriteScaledChar(16,0, ':',2);
        my_oled.WriteScaledChar(32,0, ' ', 2);
        my_oled.WriteScaledChar(48,0, ' ', 2);
        my_oled.WriteScaledChar(64,0, ' ', 2);
        my_oled.WriteScaledChar(32,0, '0' + digits[0] ,2);
        my_oled.WriteScaledChar(48,0, '0' + digits[1] ,2);
        my_oled.WriteScaledChar(64,0, '.', 2);
        my_oled.WriteScaledChar(80,0, '0' + digits[2] ,2);
        my_oled.WriteScaledChar(96,0, '%', 2);

        my_oled.Refresh(SSD1306::LINE_3);

        __bis_SR_register(LPM0_bits + GIE);
    }

    return 0;
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
        CPL_BIT(PORT_OUT(LED_PORT),LED_PIN);
        x = 0;
        __bic_SR_register_on_exit(CPUOFF);
    }

    x++;
}

