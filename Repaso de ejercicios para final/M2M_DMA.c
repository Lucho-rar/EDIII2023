#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_gpdma.h"
#endif




uint32_t *region_inicial;
uint32_t *region_final;
uint8_t aux = 0;

#define DMA_SIZE 64

void configurarPunteros();
void configurarDMA();


int main(void) {
	configurarPunteros();
	configurarDMA();
	GPDMA_ChannelCmd(0,ENABLE);
	NVIC_EnableIRQ(DMA_IRQn);

	while(aux==0){

	}
	return 0;
}


void configurarPunteros(){
	region_inicial = (uint32_t *) 0x2007C000;
	region_final = (uint32_t *)0x2007E000;
	return;
}


void configurarDMA(){
	GPDMA_Channel_CFG_Type dma;
	dma.ChannelNum = 0;
	dma.TransferSize = DMA_SIZE;
	dma.TransferWidth = 0;
	dma.SrcMemAddr = (uint32_t)&region_inicial;
	dma.DstMemAddr = (uint32_t)&region_final;
	dma.TransferType = GPDMA_TRANSFERTYPE_M2M;
	dma.SrcConn = 0;
	dma.DstConn = 0;
	dma.DMALLI = 0;
	GPDMA_Init();
	GPDMA_Setup(&dma);
}


void DMA_IRQHandler(){
	if(GPDMA_IntGetStatus(GPDMA_STAT_INT, 0)){		//Funcion para detectar que channel interrumpe
		if(GPDMA_IntGetStatus(GPDMA_STATCLR_INTTC, 0)){		//Interrumpe por finalizacion de transferencia
			aux++;
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);
			return;
		}
		if(GPDMA_IntGetStatus(GPDMA_STATCLR_INTERR, 0)){
			aux++;
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 0);
			return;
		}

	}
}










