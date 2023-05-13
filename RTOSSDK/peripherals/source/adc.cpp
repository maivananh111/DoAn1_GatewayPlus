/*
 * adc.cpp
 *
 *  Created on: Jun 30, 2022
 *      Author: A315-56
 */

#include "peripheral_config.h"
#if ENABLE_ADC

#include "periph/adc.h"
#include "periph/gpio.h"
#include "periph/rcc.h"

#include "math.h"


adc::adc(ADC_TypeDef *adc){
	_adc = adc;
}


stm_ret_t adc::init(adc_config_t *conf){
	stm_ret_t ret;

	_conf = conf;
	uint32_t tmp_ADC_SMPR1 = 0;
	uint32_t tmp_ADC_SMPR2 = 0;

	/* ADC CLOCK ENABLE */
	if(_adc == ADC1) RCC -> APB2ENR |= RCC_APB2ENR_ADC1EN;
#if defined(ADC2)
	if(_adc == ADC2) RCC -> APB2ENR |= RCC_APB2ENR_ADC2EN;
#endif /* defined(ADC2) */
#if defined(ADC3)
	if(_adc == ADC3) RCC -> APB2ENR |= RCC_APB2ENR_ADC3EN;
#endif /* defined(ADC3) */

	/* GPIO SETUP AT ANALOG MODE */
	for(uint8_t i=0; i<_conf -> num_channel; i++) {
		if(_conf -> pin_list[i] <= 15) gpio_set_mode(_conf -> port_list[i], _conf -> pin_list[i], GPIO_ANALOG);
	}

	/* ADC PRESCALER DIVISION */
#if defined(STM32F1)
	__IO uint32_t adc_pres = ceil((float)rcc_get_bus_frequency(APB2)/14000000.0);
	if(adc_pres > 8U) adc_pres = 8U;
	if(adc_pres == 0U) adc_pres = 2U;
	if(adc_pres % 2 != 0U && adc_pres != 0U) adc_pres += 1;
	adc_pres = (adc_pres >> 1U) - 1;
	RCC -> CFGR |= (adc_pres << RCC_CFGR_ADCPRE_Pos);
#elif defined(STM32F4)
	ADC123_COMMON -> CCR &=~ ADC_CCR_ADCPRE;
	ADC123_COMMON -> CCR |= _conf -> prescaler;
#endif

	/* ADC CONFIGURATION */
	_adc -> CR1 &=~ ADC_CR1_SCAN;
	if(sizeof(_conf -> pin_list) > 1)
		_adc -> CR1 |= ADC_CR1_SCAN;   // ADC SCAN MODE.

	_adc -> CR1 &=~ ADC_CR1_RES;
	_adc -> CR1 |= _conf -> resolution; // ADC RESOLUTION.

	_adc -> CR2 &=~ ADC_CR2_ALIGN; // ADC DATA ALIGN RIGHT.
	if(_conf -> dataalign == ADC_DATAALIGN_LEFT)
		_adc -> CR2 |= ADC_CR2_ALIGN;

	_adc -> CR2 &=~ ADC_CR2_EXTSEL; // ADC SELECT SOFTWARE START.
	_adc -> CR2 &=~ ADC_CR2_EXTEN;  // ADC DISABLE EXTERNAL TRIGER START.

	_adc -> CR2 &=~ ADC_CR2_CONT;
	_adc -> CR2 |= _conf -> continuos; // ADC CONTINUOUS MODE ENABLE.
	_adc -> CR1 &=~ ADC_CR1_DISCEN;    // ADC DISCONTINUOUS MODE DISABLE.

	for(uint8_t i=0; i<_conf -> num_channel; i++){
		if(_conf -> rank_list[i] <= 9) tmp_ADC_SMPR2 |= (_conf -> sampletime_list[i] << (3*_conf -> rank_list[i]));      // SMPR1 REGISTER
		else 				           tmp_ADC_SMPR1 |= (_conf -> sampletime_list[i] << (3*(_conf -> rank_list[i] - 10))); // SMPR1 REGISTER
	}
	_adc -> SMPR1 = tmp_ADC_SMPR1; // ADC CHANNEL 10-17 SAMPLING TIME
	_adc -> SMPR2 = tmp_ADC_SMPR2; // ADC CHANNEL 0 - 9 SAMPLING TIME

	uint8_t num_reg = ceil((float)_conf -> num_channel/6.0);
	uint32_t ADC_SQR1 = 0, ADC_SQR2 = 0, ADC_SQR3 = 0;
	if(num_reg == 0) num_reg++;
	if(num_reg == 1 && _conf -> num_channel <= 6){
		for(uint8_t i=0; i<_conf -> num_channel; i++)  ADC_SQR3 |= (_conf -> rank_list[i] << i*5);
	}
	else if(num_reg == 2 && _conf -> num_channel <=12){
		for(uint8_t i=0; i<6; i++)                          ADC_SQR3 |= (_conf -> rank_list[i] << i*5); //0 -> 5
		for(uint8_t i=6; i<_conf -> num_channel; i++)  ADC_SQR2 |= (_conf -> rank_list[i] << (i-6)*5); // 6 -> numcvt
	}
	else{
		for(uint8_t i=0; i<6; i++)                          ADC_SQR3 |= (_conf -> rank_list[i] << i*5); //0 -> 5
		for(uint8_t i=6; i<12; i++)                         ADC_SQR2 |= (_conf -> rank_list[i] << (i-6)*5); // 6 -> 11
		for(uint8_t i=12; i<_conf -> num_channel; i++) ADC_SQR1 |= (_conf -> rank_list[i] << (i-12)*5); // 12 -> numcvt
	}
	_adc -> SQR1 = ADC_SQR1; // ADC SEQUENCE NUMBER.
	_adc -> SQR2 = ADC_SQR2; // ADC SEQUENCE NUMBER.
	_adc -> SQR3 = ADC_SQR3; // ADC SEQUENCE NUMBER.

	_adc -> SQR1 |= ((_conf -> num_channel - (uint8_t)1) << ADC_SQR1_L_Pos); // ADC NUMBER OF CONVERSION SEQUENCE

//	if(_conf -> adc_temp_vref == true) _adc -> CR2 |= ADC_CR2_TSVREFE;

	_adc -> CR2 |= ADC_CR2_ADON; // TURN ON ADC AND TO START CONVERSION
}
/*
void ADC_Start(ADC_TypeDef *ADC){
	ADC -> CR2 |= ADC_CR2_ADON; // ENABLE ADC
	ADC -> SR = 0; // CLEAR ADC STATUS REGISTER
	ADC -> CR2 |= ADC_CR2_EXTTRIG; // ENABLE ADC START BY EXTERNAL TRIGER
	ADC -> CR2 |= ADC_CR2_SWSTART; // START ADC
}

void ADC_Stop(ADC_TypeDef *ADC){
	ADC -> CR2 &=~ ADC_CR2_ADON; // DISABLE ADC
	ADC -> CR2 &=~ ADC_CR2_EXTTRIG; // DISABLE ADC START BY EXTERNAL TRIGER
	ADC -> CR2 &=~ ADC_CR2_SWSTART; // STOP ADC
}

uint16_t ADC_Read(ADC_TypeDef *ADC){
	return ADC -> DR;
}

Result_t ADC_Start_DMA(ADC_TypeDef *ADC, DMA dma, uint16_t *adc_data, uint16_t num_channel){
	Result_t res = {
		.Status = OKE,
		.CodeLine = 0,
	};
	ADC -> CR2 |= ADC_CR2_DMA; // ENABLE ADC DMA
	res = dma.Start((uint32_t)&ADC -> DR, (uint32_t)adc_data,  num_channel); // SETUP DMA
	if(res.Status != OKE){
		res.CodeLine = __LINE__;
		return res;
	}
	ADC -> SR = 0; // CLEAR ADC STATUS REGISTER
	ADC -> CR2 |= ADC_CR2_EXTTRIG; // ENABLE ADC START BY EXTERNAL TRIGER
	ADC -> CR2 |= ADC_CR2_SWSTART; // START ADC

	return res;
}

Result_t ADC_Stop_DMA(ADC_TypeDef *ADC, DMA dma){
	Result_t res = {
		.Status = OKE,
		.CodeLine = 0,
	};
	ADC -> CR2 &=~ ADC_CR2_DMA; // DISABLE ADC DMA
	res = dma.Stop(); // STOP DMA
	if(res.Status != OKE){
		res.CodeLine = __LINE__;
		return res;
	}
	ADC -> CR2 &=~ ADC_CR2_EXTTRIG;
	ADC -> CR2 &=~ ADC_CR2_SWSTART; // STOP ADC

	return res;
}

float Internal_TemperatureSensor(ADC_TypeDef *ADC, uint16_t ITempSS_Data){
	return (float)((1.43 - ((3.3/4096.0)*(float)ITempSS_Data))/0.0043) + 25.0;
//	return 357.558 - 0.187364 * ITempSS_Data;
}
*/
#endif /* ENABLE_ADC */

