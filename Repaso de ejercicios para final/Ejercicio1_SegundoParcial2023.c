/*
 * Programar un codigo usando un timer y un pin de capture para demodular una PWM que ingresa.
 * Calcular su DutyCicle y el Periodo. Sacar una tension  proporcional al duty cicle a traves del DAC
 * con un rango dinamico 0-2V y un rango de actualización 0.5s del promedio de los ultimos diez valores obtenidos de la captura.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"
#endif

void configurarPines();
void configurarTimer();
void guardarDuty(uint32_t);
uint32_t calcularPromedio();
void rate(uint32_t delay);

uint32_t flanco_subida_inicial = 0; //detector subida inicial
uint32_t flanco_bajada = 0;			//detector bajada
uint32_t flanco_subida_final = 0;		// detector subida final
uint8_t flanco = 0 ; 		//auxiliar para detectar cual es el flanco
uint32_t duty = 0;
uint32_t periodo = 0;
uint32_t valores_duty[10] = {0,0,0,0,0,0,0,0,0,0};

int main(void) {
	configurarPines();
	configurarTimer();
	DAC_Init(LPC_DAC);
	NVIC_EnableIRQ(TIMER1_IRQn);

	while(1){
		DAC_UpdateValue(LPC_DAC,duty);
		rate(calcularPromedio()*5);

	}
    return 0 ;
}

uint32_t calcularPromedio(){
	uint32_t promedio = 0;
	for (uint16_t i = 0 ; i < 10 ; i++){
		promedio += valores_duty[i];
	}
	promedio= promedio/10;
	return promedio;
}

void rate(uint32_t delay){
	/*			Como no uso DMA tengo que setear un delay de 0.5 por el promedio. Para eso uso systick.
	 * 		Lo seteo default en 0.1 segundos, y se repite n veces, con N= Promedio de valores x 5 (0.5s).
	 */
	SysTick->LOAD = 9999999;
	SysTick->VAL = 0;
	SysTick->CTRL |= (1<<0) | (1<<2);

	for (int i =0 ; i < delay ; i ++){
		while(!(SysTick->CTRL & (1<<16)));
	}

	SysTick->CTRL = 0;
}

void configurarPines(){
	/*		CAP1.0 		*/
	PINSEL_CFG_Type pin;
	pin.Portnum = 1;
	pin.Pinnum = 18 ;
	pin.Funcnum = 3 ;
	pin.Pinmode = PINSEL_PINMODE_TRISTATE;
	pin.OpenDrain = 0;
	PINSEL_ConfigPin(&pin);
}


/*						 Configuracion del timer					*/


void configurarTimer(){
	TIM_TIMERCFG_Type tim;
	tim.PrescaleOption = TIM_PRESCALE_TICKVAL;
	tim.PrescaleValue = 0;

	TIM_CAPTURECFG_Type cap;
	cap.CaptureChannel = 0;		//1.0
	cap.RisingEdge = ENABLE;
	cap.FallingEdge = ENABLE;
	cap.IntOnCaption = DISABLE;

	TIM_Init(LPC_TIM1, 0, &tim);		//este init configura el cclk en /4. Si trabajamos a 100Mhz -> 25MHZ trabaja el timer.
	TIM_ConfigCapture(LPC_TIM1, &cap);
	TIM_Cmd(LPC_TIM1,ENABLE);
}



/*					Lo que hago  es detectar los flancos para hacer los calculos de valor captado.
 * Recordar que el timer esta con el prescaler en 0 y el timer está a cclk/4 por defecto de los drivers.
 * Entonces:
 * 					Tres = PR + 1 / 25Mhz      -> 0.04 uS.
 * 					Es decir, cada ese tiempo se incrementará el TC.
 *
 * Supongo que la señal arranca en reposo  ___________ -> ___|--|___
 */
void TIMER1_IRQHandler(){
	if(flanco == 0) {//Primera vez que interrumpe
		flanco_subida_inicial = LPC_TIM1->CR0; 		//valor de tc.
	}else if(flanco==1){
		flanco_bajada = LPC_TIM1->CR0;
		duty = (flanco_bajada - flanco_subida_inicial) * 0.00000004;
		guardarDuty(duty);
	}else{
		flanco_subida_final = LPC_TIM1->CR0;
		periodo = (flanco_subida_final - flanco_subida_inicial) * 0.00000004;
	}
	flanco++;
	if (flanco>2){
		flanco=0;
	}

	TIM_ClearIntCapturePending(LPC_TIM1, TIM_CR0_INT);		//clear flags

}


void guardarDuty(uint32_t value){
	for (uint16_t i = 0 ; i < 9 ; i ++){				//Actualizo valores
		valores_duty[i] = valores_duty[i+1];
	}
	valores_duty[9] = value;
	return;
}








