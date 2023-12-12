/*
			Programar el microcontrolador LPC1769 para que mediante su ADC digitalice
			dos señales analógicas cuyos anchos de bandas son de 10 Khz cada una. Los canales
			utilizados deben ser el 2 y el 4 y los datos deben ser guardados en dos regiones de
			 memorias distintas que permitan contar con los últimos 20 datos de cada canal. Suponer
			  una frecuencia de core cclk de 100 Mhz. El código debe estar debidamente comentado.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"

#endif

void configuracionTimerConD();
void configuracionTimerSinD();
void configuracionADCConD();
void configuracionADCSinD();
void cargarDatos(uint16_t , uint8_t);
void pines();



uint32_t datos_ch2[20];
uint32_t datos_ch4[20];

int main(void) {
	pines();
	configuracionTimerConD();
	configuracionADCConD();
	NVIC_EnableIRQ(ADC_IRQn);
	while(1){}
	return 0 ;

}

void pines(){
	PINSEL_CFG_Type pin;
	pin.Portnum = 0;			//ad2
	pin.Pinnum = 25;
	pin.OpenDrain= 0;
	pin.Pinmode = PINSEL_PINMODE_TRISTATE;
	pin.Funcnum = 1;
	PINSEL_ConfigPin(&pin);
	//ad4
	pin.Portnum = 1;
	pin.Funcnum = 3;
	pin.Pinnum = 30;
	PINSEL_ConfigPin(&pin);
	//mat0.1
	pin.Pinnum = 28;
	pin.Pinmode = PINSEL_PINMODE_PULLDOWN;
}

/* Cada señal tiene 10 Khz, por lo que se muestrea a 40KHz. El adc a 200K funciona a 5uS, y a 40Khz necesitamos 25 uS, por lo que el
 * timer funciona a 20 uS . (+5uS del ADC obtenemos los 25uS de muestreo).
 * Pero el start se hace con el MAT a un cierto edge, entonces tenemos que configurar que el pin toggleé a la mitad, es decir 10uS.
 * Supongo un PR a 199. Tres = 200/100Mhz = 2uS.
 * Match = 10uS / 2uS  -1 = 4
 *
 * El enunciado pide a 100Mhz de cclk, habria que modificar la lib para que no use el divisor.
 *
 */
void configuracionTimerConD(){
	TIM_TIMERCFG_Type timer;
	timer.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timer.PrescaleValue = 199;

	TIM_MATCHCFG_Type match;
	match.MatchChannel = 1;
	match.IntOnMatch = DISABLE;
	match.StopOnMatch = DISABLE;
	match.ResetOnMatch = ENABLE;
	match.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	match.MatchValue = 4;

	TIM_Init(LPC_TIM0,0, &timer);
	TIM_ConfigMatch(LPC_TIM0,&match);
	TIM_Cmd(LPC_TIM0,ENABLE);

}

void configuracionTimerSinD(){
	LPC_SC->PCONP |= (1<<1);
	LPC_SC->PCLKSEL0 |= (1<<2);
	LPC_TIM0->PR = 199;
	LPC_TIM0->MR1 = 4;
	LPC_TIM0->IR |= (1<<0);
	LPC_TIM0->EMR |= (0x3<<6);
	LPC_TIM0->TCR |= 3;
	LPC_TIM0->TCR &= ~2;

}
void configuracionADCConD(){
	ADC_Init(LPC_ADC,200000);
	ADC_BurstCmd(LPC_ADC,DISABLE);
	ADC_StartCmd(LPC_ADC,ADC_START_ON_MAT01);
	ADC_EdgeStartConfig(LPC_ADC,0); //RISING;
	ADC_IntConfig(LPC_ADC,2,ENABLE);
	ADC_IntConfig(LPC_ADC,4,ENABLE);

	return;
}

void configuracionADCSinD(){
	LPC_ADC->ADCR |= (1<<2) | (1<<4); //CHANEL 2 Y 4
	LPC_ADC->ADCR &= ~(0xff<<8);	//NO CLKDIV
	LPC_ADC->ADCR &= ~(1<<16); 	//NO BURST
	LPC_ADC->ADCR |= (4<<24); //Start con mat01
	LPC_ADC->ADCR &= ~(1<<27); //RISING;
	LPC_ADC->ADINTEN |= (1<<2) | (1<<4);

	return;

}


void ADC_IRQHandler(){

	if(ADC_ChannelGetStatus(LPC_ADC,2,ADC_DATA_DONE)){		//LPC_ADC->ADDR2 & (1<<31) "DONE" del canal 2. Si no es, es el del canal 4-.
		cargarDatos(ADC_ChannelGetData(LPC_ADC,2),2);		//ADDR2 >>4 Y CASTEADO EN 12BITS
		LPC_ADC->ADGDR &= LPC_ADC->ADGDR;
	}else{
		cargarDatos(ADC_ChannelGetData(LPC_ADC,4),4);
		LPC_ADC->ADGDR &= LPC_ADC->ADGDR;
	}
	return;

}

void cargarDatos(uint16_t valor, uint8_t canal){
	if(canal ==2){
		for(uint8_t i = 0 ; i < 19 ; i ++){
			datos_ch2[i] = datos_ch2[i+1];
		}
		datos_ch2[19] = valor;
	}else{
		for(uint8_t i = 0 ; i < 19 ; i ++){
			datos_ch4[i] = datos_ch4[i+1];
		}
		datos_ch4[19] = valor;
	}
	return;
}




