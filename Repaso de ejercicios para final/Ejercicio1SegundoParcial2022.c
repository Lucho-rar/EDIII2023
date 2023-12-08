/* Programar el microcontrolador LPC1769 para que mediante su ADC digitalice
 *  dos señales analógicas cuyos anchos de bandas son de 10 Khz cada una.
 *  Los canales utilizados deben ser el 2 y el 4 y los datos deben ser guardados
 *	en dos regiones de memorias distintas que permitan contar con los últimos 20 datos
 *  de cada canal. Suponer una frecuencia de core cclk de 100 Mhz.
 *   El código debe estar debidamente comentado.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#endif

/*									Encabezado					*/
uint32_t channel2_region[20];
uint32_t channel4_region[20];

void inicializarRegiones();
void configurarPines();
void configurarADC();
void configurarTimer();
void cargarDato(uint32_t,uint8_t);
/*									Main						*/
int main(void) {
	inicializarRegiones();
	configurarPines();
	configurarTimer();
	configurarADC();

	while(1){}

    return 0 ;
}


/*							Desarrollo de funciones				*/

void inicializarRegiones(){
	for (uint8_t i=0 ; i<20 ; i++){
		channel2_region[i] = 0;
		channel4_region[i] = 0;
	}
}

void configurarPines(){
	/*				AD0.2			*/

	PINSEL_CFG_Type pin;
	pin.Portnum = 0;
	pin.Pinnum = 25;
	pin.Funcnum = 1;
	pin.Pinmode = PINSEL_PINMODE_TRISTATE;
	pin.OpenDrain = 0;

	PINSEL_ConfigPin(&pin);

	/*				AD0.4			*/

	pin.Portnum = 1;
	pin.Pinnum = 30;
	pin.Funcnum = 3;

	PINSEL_ConfigPin(&pin);

}


/*	Se reciben entonces 2 señales de 10KHz. La frecuencia de muestreo se divide en los canales.
 *  Si ambas señales son de 10KHz, debemos muestrear cada una al doble, de la máxima. Es decir, la frecuencia de muestreo
 *  debe ser 40KHz para muestrear cada señal a 20KHz (fmuestreo/channels).
 *	Al bajar la frecuencia tanto, lo ideal es que que el ADC muestree a 200 KHz y habilitar la conversion con un timmer.
 *
 *	A 200KHZ se tienen 5uS. A 40 Khz es 25uS. Es decir, podemos configurar el timmer a 20uS + 5uS(del adc) para obtener los 25uS.
 */

void configurarADC(){
	ADC_Init(LPC_ADC, 200000);
	ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
	ADC_BurstCmd(LPC_ADC, DISABLE);
	ADC_ChannelCmd(LPC_ADC, 2, ENABLE);
	ADC_ChannelCmd(LPC_ADC, 4, ENABLE);
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_RISING);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, ENABLE);

	NVIC_EnableIRQ(ADC_IRQn);
}

/*	El timmer debe interrumpir entonces cada 20uS. Recordar que la libreria lo incializa con cclk/4.
 * 	Tres = (PR +1) / CCLK -> Si pr= 0  -> 0.04uS.
 * 	Match = 20 uS -> (20uS / 0.04uS) - 1= 499;
 *
 *
 *
 *
 */
void configurarTimer(){
	TIM_TIMERCFG_Type tim;
	tim.PrescaleOption = TIM_PRESCALE_TICKVAL;
	tim.PrescaleValue = 0;

	TIM_MATCHCFG_Type match;
	match.MatchChannel = 1 ;
	match.IntOnMatch = DISABLE;
	match.StopOnMatch = DISABLE;
	match.ResetOnMatch = ENABLE;
	match.ExtMatchOutputType = 0;
	match.MatchValue = 499;

	TIM_Init(LPC_TIM0, 0, &tim);
	TIM_ConfigMatch(LPC_TIM0, &match);
	TIM_Cmd(LPC_TIM0,ENABLE);

}


void ADC_IRQHandler(){


	if (ADC_ChannelGetStatus(LPC_ADC, 2, ADC_DATA_DONE)){		//Si es el channel 2
		cargarDato(ADC_ChannelGetData(LPC_ADC,2),2);
	}else{
		cargarDato(ADC_ChannelGetData(LPC_ADC,4),4);
	}

}

void cargarDato(uint32_t valor, uint8_t canal){
	if(canal==2){
		for (uint8_t i = 0 ; i < 19 ; i ++){
			channel2_region[i] =channel2_region[i+1];
		}
		channel2_region[19] = valor;
	}else{
		for (uint8_t i = 0 ; i < 19 ; i++){
			channel4_region[i] = channel4_region[i+1];
		}
		channel4_region[19] = valor;
	}

	return;
}














