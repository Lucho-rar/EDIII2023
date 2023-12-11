
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#
#endif


void generarWaveform();
void configurarDAC();
void configurarDMA();

uint32_t wf[2046];

int main(void) {
	PINSEL_CFG_Type pin;
	pin.Portnum = 0;
	pin.Pinnum = 26;
	pin.Funcnum = 2;
	pin.Pinmode = PINSEL_PINMODE_TRISTATE;
	pin.OpenDrain = 0 ;
	PINSEL_ConfigPin(&pin);
	generarWaveform();
	configurarDAC();
	configurarDMA();
	GPDMA_ChannelCmd(0,ENABLE);
	while(1){}

    return 0 ;
}



/*			Suponiendo la máxima resolución y refresco . Debe funcionar a 1uS entre muestra y aumentar
 * 			bit a bit (para una triangular).
 */
void generarWaveform(){
	for(uint16_t i = 0 ;  i < 1023*2; i++){
		if(i<=1023){
			wf[i] = i;
		}else{
			wf[i] = (1023*2) - i;
		}

		wf[i] = wf[i] <<6 ;
	}

	return;
}


void configurarDAC(){
	DAC_CONVERTER_CFG_Type dac;
	dac.CNT_ENA = SET;
	dac.DMA_ENA = SET;
	DAC_ConfigDAConverterControl(LPC_DAC, &dac); //Está comentado el divisor de la lib para el cclk.
	DAC_Init(LPC_DAC);

	/*											Calculos suponiendo 25MHZ de cclk
	 * Si tenemos que incrementamos bit a bit y 2046 muestras, entre muestra y muestra como máximo podemos poner 1 uS para respetar el BIAS
	 * 						2046		-----		Tseñal
	 * 						1			----- 		1uS
	 * 						Tseñal -> 2046uS
	 * Luego, debemos compararlo al clock del dac, es el incremento del contador del dac
	 * 						1/fDAC		-----		1
	 * 						1uS			-----		Valor TimeOut -> 25
	 *
	 *
	 *
	 */
	DAC_SetDMATimeOut(LPC_DAC,25);
}

void configurarDMA(){
	GPDMA_LLI_Type lista;
	lista.SrcAddr = (uint32_t)&(wf);
	lista.DstAddr = (uint32_t)LPC_DAC->DACR;
	lista.NextLLI = (uint32_t)&lista;
	lista.Control = 2046 | (2<<18) | (2<<21) | (1<<26);

	GPDMA_Channel_CFG_Type canal;
	canal.ChannelNum = 0;
	canal.TransferSize = 2046;
	canal.TransferWidth = 0;
	canal.SrcMemAddr = (uint32_t)&wf;
	canal.DstMemAddr = 0;
	canal.TransferType = GPDMA_TRANSFERTYPE_M2P;
	canal.SrcConn = 0;
	canal.DstConn = GPDMA_CONN_DAC;
	canal.DMALLI = (uint32_t)&lista;

	GPDMA_Setup(&canal);

}









