/*
			1 - 2023

 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"

#endif




/*										Salteo las configuraciones comunes, ya las tengo en papel.
 * Estos desarrollos se enfocan en las distintas configuraciones posibles de DMA
 *
 *
 */

uint32_t *segundo_bloque;
uint32_t *primer_bloque;

int main(void) {

	while(1){}
	return 0 ;

}



void configurarDMA(){
	/*								Con listas 				*/
	GPDMA_LLI_Type l1;
	l1.SrcAddr = (uint32_t ) &segundo_bloque;		//donde está la waveform
	l1.DstAddr = (uint32_t ) &LPC_DAC->DACR ;
	l1.NextLLI = (uint32_t)&l1;		// a esta listano hay que esperarla asi que la hago infinita
	l1.Control = 8191 | (2<<18) | (2<<21) | (1<<26);

	GPDMA_LLI_Type l2;
	l2.SrcAddr = (uint32_t ) &LPC_ADC->ADDR0;
	l2.DstAddr = (uint32_t ) &primer_bloque ;		//bloque donde se guardan los valores del adc
	l2.NextLLI = 0;		//a aesta hay que esperarla, asi que chequeo en cada fin
	l2.Control = 8191 | (2<<18) | (2<<21) | (1<<27);

	GPDMA_LLI_Type l3;
	l3.SrcAddr = (uint32_t ) &primer_bloque; //donde se guardaron los valores obtenidos del adc
	l3.DstAddr = (uint32_t ) &LPC_DAC->DACR ;		//bloque donde se guardan los valores del adc
	l3.NextLLI = (uint32_t)&l3;			//esta tampoco hay que esperarla asi q la hago infinita
	l3.Control = 8191 | (2<<18) | (2<<21) | (1<<27);

	GPDMA_Channel_CFG_Type canal;	//La de mas prioridad es la que hay que esperar, para que al activar el otro channel, se espere q termine
	canal.ChannelNum = 0;
	canal.TransferSize = 8191;
	canal.TransferWidth = 0;
	canal.SrcMemAddr = 0;
	canal.DstMemAddr = (uint32_t) &primer_bloque;
	canal.TransferType = GPDMA_TRANSFERTYPE_P2M;
	canal.SrcConn = GPDMA_CONN_ADC;
	canal.DstConn = 0;
	canal.DMALLI =(uint32_t)&l2;		//esto creo q es al pepe,
	GPDMA_Setup(&canal);


	canal.ChannelNum = 1;			//La segunda es la del segundo estado, ya que es menos que la del ADC inicial pero más que el DAC inicial
	canal.SrcMemAddr = (uint32_t ) &primer_bloque;;
	canal.DstMemAddr = 0;
	canal.TransferType = GPDMA_TRANSFERTYPE_M2P;
	canal.SrcConn = 0;
	canal.DstConn = GPDMA_CONN_DAC;
	canal.DMALLI = (uint32_t) &l3;			//este si se apunta asi mismo
	GPDMA_Setup(&canal);

	canal.ChannelNum =2;
	canal.SrcMemAddr = (uint32_t) &segundo_bloque; 		//este saca la wform
	canal.DMALLI = (uint32_t)&l1;
	GPDMA_Setup(&canal);

}

/*					Esta interrupcion tiene que variar los estados 		*/
void EINT0_IRQHandler(){
	if(state == 0){		//ADC a Mem y Waveform por DAC
		GPDMA_ChannelCmd(1,ENABLE);		//Habilito el canal, en teoria tendria mas prioridad que el 2 (el del dac) asi que no lo espera.
		//Pero si tiene menor prioridad que el del adc, a ese lo deja activo hasta que se desactive el mismo en la interrupción.
		GPDMA_ChannelCmd(2,DISABLE);
		state++;
	}else{
		state=0;	/* Vuelve al estado desde acá directamente */
		GPDMA_ChannelCmd(2,ENABLE);
		GPDMA_ChannelCmd(0,ENABLE);
		GPDMA_ChannelCmd(1,DISABLE);
	}

}


void DMA_IRQHandler(){
	/* En esta rutina lo que hago (lo puse en las hojas), es testear el canal que interrumpe
	 * como el 1 y el 3 son un bucle, nunca entrarían a interrumpir.
	 * Pero la que entra es la del ADC. Entonces ahí testeamos si "state" se puso en 1, en ese caso, desactivamos el canal.
	 *
	 *
	 * Otra opción es que modificar un contador o una variable y clavarlo en un while cuando esté el handler de eint0.
	 * Habría que preguntar un par de cosas en la consigna, pero hay varias formas de hacer.
	 *
	 *
	 * Otra opción para las listas es usar un unico canal para la waveform y el ADC.
	 * Pero en ese caso, aparte de esperar al ADC estaría esperando al DAC.
	 */
}





