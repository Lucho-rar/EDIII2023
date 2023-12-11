
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"

#endif

uint32_t buffer_origen[10] = {231,43,456,234,23,1,9,23,123,23};
uint32_t buffer_destino[10];

void configurarTimer();
void configurarDMA();

int main(void) {
	configurarTimer();
	configurarDMA();
	GPDMA_ChannelCmd(0,ENABLE);
	while(1){}

    return 0 ;
}


void configurarTimer(){
	TIM_TIMERCFG_Type timer;
	timer.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timer.PrescaleValue = 499;

	TIM_MATCHCFG_Type match;
	match.MatchChannel = 0;
	match.IntOnMatch = DISABLE;
	match.StopOnMatch = ENABLE;
	match.ResetOnMatch = DISABLE;
	match.ExtMatchOutputType = 0;
	match.MatchValue = 25;

	TIM_Init(LPC_TIM0, 0,&timer);
	TIM_ConfigMatch(LPC_TIM0, &match);
	TIM_Cmd(LPC_TIM0, ENABLE);
}


void configurarDMA(){
	GPDMA_Channel_CFG_Type canal;
	canal.ChannelNum = 0 ;
	canal.TransferSize = 10;
	canal.TransferWidth = 0;
	canal.SrcMemAddr = (uint32_t)&buffer_origen;
	canal.DstMemAddr = (uint32_t)&buffer_destino;
	canal.TransferType = GPDMA_TRANSFERTYPE_P2M;
	canal.SrcConn = GPDMA_CONN_MAT0_0;
	canal.DstConn = 0;
	canal.DMALLI = 0 ;

	GPDMA_Init();
	GPDMA_Setup(&canal);
}




