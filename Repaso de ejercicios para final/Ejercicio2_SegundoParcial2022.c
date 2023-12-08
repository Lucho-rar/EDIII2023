/*
 * Utilizando el timer0, un dac, interrupciones y el driver del LPC1769 ,
 * escribir un código que permita generar una señal triangular periódica simétrica,
 *  que tenga el mínimo periodo posible, la máxima excursión de voltaje pico a pico
 *  posible y el mínimo incremento de señal posible por el dac. Suponer una frecuencia
 *   de core cclk de 100 Mhz. El código debe estar debidamente comentado.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"
#endif

uint32_t waveform[2046];
uint32_t indice = 0;
void generar_waveform();
void configurarTimer();
int main(void) {
	generar_waveform();
	DAC_Init(LPC_DAC);
	configurarTimer();



	while(1){

	}

    return 0 ;
}


/*	Los requerimientos para la señal es el minimo incremento y voltaje maximo pico a pico (3.3).
 *  Entonces debemos ir incrementado bit a bit y con sacarlo con el minimo timeout. Para esto
 *  la señal triangular es una rampa de 0 a 1023 y otra rampa (descendente) de 1023 a 0.
 *  Es decir, se necesitan 2x1023 muestras. (1024x2 nada mas que se incluye el 0).
 *
 */
void generar_waveform(){
	for(uint16_t i = 0 ; i <= 2046 ; i++){
		if(i<1023){
			waveform[i] = i;
		}else if(i==1023){
			waveform[i] = 1023;
		}else{
			waveform[i] = 2046 - i;
		}
		waveform[i] = waveform[i]<<6;
	}
}

/* El timmer a usar para actualizarlo tambien lo usamos al minimo del cclk que tenemos. Recordar que la libreria
 * divide el periferico con cclk/4 por defecto. Si el prescaler es 0
 * TRes = PR + 1 / (100M/4) = 0.04 uS. Pero, en teoria el máximo rate de 1 MHz por lo tanto el delay no debe ser menor a 1us.
 * Entonces el valor de match se debe dar a 1uS es decir -> 1uS / 0.04uS - 1= 24;
 *
 *
 */
void configurarTimer(){
	TIM_TIMERCFG_Type tim;
	tim.PrescaleOption = TIM_PRESCALE_TICKVAL;
	tim.PrescaleValue = 0;

	TIM_MATCHCFG_Type match;
	match.MatchChannel = 0;
	match.IntOnMatch = ENABLE;
	match.StopOnMatch = DISABLE;
	match.ResetOnMatch = ENABLE;
	match.ExtMatchOutputType = 0;
	match.MatchValue = 24;

	TIM_Init(LPC_TIM0, 0, &tim);
	TIM_ConfigMatch(LPC_TIM0,&match);
	TIM_Cmd(LPC_TIM0,ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
}


/* Por cada interrupcion actualizo elvalor del dac*/
void TIMER0_IRQHandler(){
	DAC_UpdateValue(LPC_DAC,waveform[indice]);
	indice++;
	if(indice>2046) indice =0;
	TIM_ClearIntPending(LPC_TIM0, 0);
}



