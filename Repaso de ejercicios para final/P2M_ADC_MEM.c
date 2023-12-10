/*
 * Ejercicio inventado . Recibir con DMA desde ADC y guardarlo en una region de memoria.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_pinsel.h"
#endif

uint32_t *region;
#define DMA_SIZE 64

void configurarPunteros();
void configurarPines();
void configurarADC();
void configurarDMA();


int main(void) {
	configurarPunteros();
	configurarPines();
	configurarADC();
	configurarDMA();
	GPDMA_ChannelCmd(0,ENABLE);
	ADC_StartCmd(LPC_ADC, ADC_START_NOW);
	while(1){}
	return 0 ;
}


void configurarPunteros(){
	region = (uint32_t *)0x2007C000;
}


void configurarPines(){
	PINSEL_CFG_Type pin;
	pin.Portnum = 0;
	pin.Pinnum = 23;
	pin.Funcnum = 1;
	pin.Pinmode = PINSEL_PINMODE_TRISTATE;
	pin.OpenDrain = 0;

	PINSEL_ConfigPin(&pin);
}

void configurarADC(){
	ADC_Init(LPC_ADC, 200000);
	ADC_ChannelCmd(LPC_ADC,0,ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0,ENABLE);

}


void configurarDMA(){
	GPDMA_LLI_Type lista;

	lista.SrcAddr = (uint32_t)&LPC_ADC->ADDR0;
	lista.DstAddr = (uint32_t)&region;
	lista.NextLLI = (uint32_t)&lista;
	lista.Control = DMA_SIZE | (2<<18) | (2<<21) | (1<<27);

	GPDMA_Init();
	GPDMA_Channel_CFG_Type dma;
	dma.ChannelNum = 0;
	dma.TransferSize = DMA_SIZE;
	dma.TransferWidth = 0;
	dma.SrcMemAddr = 0;
	dma.DstMemAddr = (uint32_t)region;
	dma.TransferType = GPDMA_TRANSFERTYPE_P2M;
	dma.SrcConn = GPDMA_CONN_ADC;
	dma.DstConn = 0;
	dma.DMALLI = (uint32_t)&lista;

	GPDMA_Setup(&dma);
}








