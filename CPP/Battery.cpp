/*
 * Battery.cpp
 *
 *  Created on: Jun 27, 2024
 *      Author: Renan Augusto Starke
 */

#include <msp430.h>
#include <Battery.h>

static volatile uint16_t adc_val;

Battery::Battery()
{
    /* ADC10CTL0:
     * ADC10SHT_2:  16 x ADC10CLKs
     * ADC10ON   :  ADC10 On/Enable
     * ADC10IE   :  IRQ Enable
     */
    ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE;

    /* Input A0 */
    ADC10CTL1 = INCH_1;

    /*  P1.0 ADC option select */
    ADC10AE0 |= BIT1;
}

uint16_t Battery::get_voltage(){
    /* Início da conversão: trigger por software */
    ADC10CTL0 |= ENC + ADC10SC;

    /* Desliga CPU até ADC terminar
     * Nesse exemplo o LCD é muito mais lento */
    __bis_SR_register(CPUOFF + GIE);


    voltage = (ADC10MEM * 33) >> 10;


    return voltage;
}

//Battery::~Battery()
//{
//
//}

/* ISR do ADC10. Executada quando a conversão terminar */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC10_VECTOR))) ADC10_ISR (void)
#else
#error Compiler not supported!
#endif
{
    adc_val = ADC10MEM;
    __bic_SR_register_on_exit(CPUOFF);
}


