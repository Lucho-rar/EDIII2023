/*
			1 - 2023

 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_dac.h"

#endif


void configurarTimer();
uint32_t obtenerPromedio();
void cargarDuty(uint32_t);
void pines();


uint8_t state=0,reposo=0;
uint32_t periodo, tc_inicial = 0, tc_duty =0, tc_final = 0, duty = 0;
uint32_t dutys[10] = {0,0,0,0,0,0,0,0,0,0};
TIM_CAPTURECFG_Type cap;
TIM_TIMERCFG_Type timer;
TIM_MATCHCFG_Type match;


uint32_t promedio = 0;
int main(void) {
	pines();
	configurarTimer();
	DAC_Init(LPC_DAC);

	while(1){}
	return 0 ;

}


void configurarTimer(){

	timer.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timer.PrescaleValue = 49999999;		//Configuro el timer en 0.5 segundos para que en ese tiempo usar el match también.


	match.MatchChannel = 0;
	match.IntOnMatch = ENABLE;
	match.StopOnMatch = DISABLE;
	match.ResetOnMatch = DISABLE;
	match.ExtMatchOutputType = 0;
	match.MatchValue = promedio;


	cap.CaptureChannel = 0 ;
	cap.RisingEdge = ENABLE;				//Solo habilito en rising primero para empezar a contar desde "reposo"
	cap.FallingEdge = DISABLE;
	cap.IntOnCaption =ENABLE;

	TIM_Init(LPC_TIM0, 0, &timer);
	TIM_ConfigMatch(LPC_TIM0,&match);
	TIM_ConfigCapture(LPC_TIM0,&cap);
	TIM_Cmd(LPC_TIM0, ENABLE);

}

void TIMER0_IRQHandler(){
	if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)){			//Si interrumpe el match
		DAC_UpdateValue(LPC_DAC,duty);		//AOUT = 2 x Duty / 1024
		TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT);
		return;
	}else if(TIM_GetIntCaptureStatus(LPC_TIM0, TIM_CR0_INT)  && state == 0){		//Si interrumpe el capture y state es 0, significa que es el inicio del capture(rising)
		tc_inicial = TIM_GetCaptureValue(LPC_TIM0, TIM_COUNTER_INCAP0); 	//Tira el CR0
		if(reposo==0){
			cap.FallingEdge = ENABLE; 		//En caso de ser el primero habilito el falling, con eso me aseguro de sincronizar las capturas
			TIM_ConfigCapture(LPC_TIM0,&cap);
			reposo++;
		}
		state++;
		TIM_ClearIntCapturePending(LPC_TIM0, TIM_CR0_INT);
		return;
	}else if(TIM_GetIntCaptureStatus(LPC_TIM0,TIM_CR0_INT) && state == 1){		//Significa que ya entro el primer flanco, y ahora entró el flanco de bajada
		tc_duty = TIM_GetCaptureValue(LPC_TIM0,TIM_COUNTER_INCAP0);
		if(tc_duty<tc_inicial){
			duty = (0xFFFFFFFF - tc_inicial + tc_duty) *0.5;	//chequeo si se pasó por las dudas
		}else{
			duty = (tc_duty - tc_inicial) * 0.5 ;  // Diferencia x Tres del timer
		}

		cargarDuty(duty);
		TIM_UpdateMatchValue(LPC_TIM0, 0,obtenerPromedio());	//No se entiende cual es el valor de promedio, yo lo hice con el promedio de los ultimos 10 duty;

		state++;
		TIM_ClearIntCapturePending(LPC_TIM0, TIM_CR0_INT);

	}else{		//CR0 y state > 1
		tc_final = TIM_GetCaptureValue(LPC_TIM0,TIM_COUNTER_INCAP0);
		if(tc_final<tc_inicial){
			periodo= (0xFFFFFFFF - tc_inicial + tc_final ) *0.5;
		}else{
			periodo = (tc_final - tc_inicial) *0.5 ; //Diferencia x Tres
		}

		state=0;
		TIM_ClearIntCapturePending(LPC_TIM0,TIM_CR0_INT);
		return;
	}
}


void cargarDuty(uint32_t d){ 		//Actualiza los valores
	for (uint8_t i = 0 ; i < 9 ; i++){
		dutys[i] = dutys[i+1];
	}
	dutys[9] = d;
	return;
}


uint32_t obtenerPromedio(){
	uint32_t promedio  = 0;
	for(uint8_t i = 0 ; i <10 ; i++){
		promedio+=dutys[i];
	}
	return promedio/10;
}

void pines(){
	/*Paso la configuracion de pines porque las hice 549 mil veces xd */
	//Pero sería, AOUT y C0.0.
}
















