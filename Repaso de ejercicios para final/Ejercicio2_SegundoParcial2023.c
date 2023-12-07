
/*	Mediante el ADC se debe digitalizar una señal de 16KHz con una amplitud pico a pico de 3.3V.
 *  los datos se deben guardar usando DMA en la primer parte del bank0 de AHB SRAM (todos los valores posibles). El ADC sigue funcionando es un buffer circular
 *
 *
 *  También se tiene una forma de onda como la figura. La señal se debe generar y reproducirse por el DAC desde la segunda mitad
 *  del bank0 de AHB SRAM usando DMA con un periodo de la señal de 6140uS con máxima resolución.
 *
 *  Durante la operación normal se genera la forma de onda por el DAC y el ADC también está funcionando.
 *  Cuando interrumpe una EXTINT, se debe esperar la ultima conversión del ADCm, frenarlo y se comienzan a sacar las muestras desde la memoria
 *  por el DAC tambien usando DMA.
 *  Cuando vuelve a interrumpir, vuelve a sacar la wform por el DAC y el ADC a convertir, y así sucesivamente.
 *
 *  CCLK 80MHz
 *
 *
 *
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#endif


uint32_t *primer_bloque;		//En este bloque se guardan los valores del ADC
uint32_t *segundo_bloque;		//En este bloque se genera la waveform de la  figura.
uint32_t numero_muestras_waveform;
uint8_t estado=0;

void posicionarPunteros();
void generarWaveform();
void configurarPines();
void configurarADCyDAC();
void configurarDMA();
void sincronizarArranque();

int main(void) {
	posicionarPunteros();			//direcciones
	configurarPines();			//pines
	generarWaveform();			//forma de onda
	configurarADCyDAC();			//perifericos
	configurarDMA();			//dma
	sincronizarArranque();			//
	while(1){}
    return 0 ;
}

/*	El AHB SRAM BANK 0 tiene 16Kb. Tenemos que dividirlo en 2 (es decir 8KHz) para establecer la primera y la segunda parte.
 * 	Entonces la primera parte:
 * 	0x2007C000 - 0x2007DFFF
 * 	y la segunda:
 * 	0x2007E000 - 0x2007FFFF
 */
void posicionarPunteros(){
	primer_bloque = (uint32_t *)0x2007C000;
	segundo_bloque = (uint32_t *)0x2007E000;

}


/*	Como no se aceptan los valores negativos, se posiciona inicialmente a 512 y el valor minimo será 0.
 *  En la primera parte sube bit a bit de 512 a 1023. De 1023 pasa a 0, y asciende hasta 512.
 *  Es decir, son 1024 muestras en total. (1023 incluyendo la posicion inicial).
 *
 */
void generarWaveform(){
	numero_muestras_waveform = 1023;
	for (uint32_t i = 0 ; i <= numero_muestras_waveform ; i++){
		if (i < 512){		//Acordarse que lo almacenamos en el segundo bloque de memoria.
			segundo_bloque[i] = 512+ i;
		}else if(i==512){
			segundo_bloque[i] = 0;
		}else{	//>512
			segundo_bloque[i] = i - 512;
		}

		//	Desplazo el valor por el registro del DAC -> VALUE 15:6
		segundo_bloque[i] = (segundo_bloque[i]<<6);
	}
}

/*		Necesito configurar:
 * 				-	El aout del dac.
 * 				-   Pin de entrada del ADC.
 * 				-   Botones para interrupcion externa.
 */
void configurarPines(){
		PINSEL_CFG_Type pin;
		pin.Portnum = 0; 		// AD0.0
		pin.Pinnum = 23;
		pin.Funcnum = 1;
		pin.Pinmode = PINSEL_PINMODE_TRISTATE;
		pin.OpenDrain = 0;
		PINSEL_ConfigPin(&pin);

		pin.Portnum = 0; 		// AOUT
		pin.Pinnum = 26;
		pin.Funcnum = 2;
		pin.Pinmode = PINSEL_PINMODE_TRISTATE;
		pin.OpenDrain = 0;
		PINSEL_ConfigPin(&pin);


		pin.Portnum = 2; 		// EINT0
		pin.Pinnum = 10;
		pin.Funcnum = 1;
		pin.Pinmode = PINSEL_PINMODE_PULLUP;
		pin.OpenDrain = 0;
		PINSEL_ConfigPin(&pin);

		LPC_SC->EXTINT |= (1<<0);		//registros de interrupcion, el enable lo hago en otra funcion .
		LPC_SC->EXTMODE |= (1<<0);
		LPC_SC->EXTPOLAR &= ~(1<<0);

}

void configurarADCyDAC(){
	/*	 En teoria a frecuencias mas bajas habría que configurarlo con un timer al ADC para que no genere errores
	 *   A fin de cumplir la consigna lo pongo directo
	 */
	uint32_t timeout;
	ADC_Init(LPC_ADC,32000); // 16K x 2 por T. de muestreo
	ADC_ChannelCmd(LPC_ADC, 0, ENABLE);

	DAC_CONVERTER_CFG_Type dacCfg;
	dacCfg.CNT_ENA = SET;		//Habilito el contador para usar el timeout
	dacCfg.DMA_ENA = SET;			// Habilito el acceso de DMa

	DAC_Init(LPC_DAC);



	/*	Calculo del timeout: es el tau para que el DMA actualice el valor a sacar por el dac.
	 *  En nuestro caso el periodo de la señal debe ser 6140uS. Es decir la frecuencia tiende a ser 163Hz.
	 *
	 *  Los calculos son : 1024 muestras -> 6140 uS
	 *  				   1 muestra -> 6uS (Este es el tiempo entre cada muestra)
	 *
	 *  Ahora el timer incluido en el DAC funciona a 1/cclk (no usamos divisor asi que son 80MHz). Por cada valor de este
	 *  se aumenta el "TC" del timer del dac. Entonces:
	 *  				   1/80MHz ~ 0.0125uS -> 1
	 *  				   6 uS   -> x = ~ 480. Este es el valor del timeout.
	 *
	 *  Tambien se puede calcular con CCLK/(frecuencia de la señal * numero de muestras).
	 *
	 *
	 *
	 */
	timeout = 480;
	DAC_SetDMATimeOut(LPC_DAC, timeout);
	DAC_ConfigDAConverterControl(LPC_DAC, &dacCfg);



}

/*	Para DMA uso 3 canales:
 * 							- 1. ADC a Mem
 * 							- 2. Waveform a DAC.
 * 							- 3. La region de memoria del adc a DAC.
 * 							Se podría unir 1 y 3 y cambiar la configuracion para usar 2 canales nomas.
 *
 *
 *
 */

void configurarDMA(){
	GPDMA_LLI_Type adc_mem, wf_dac, mem_dac;

	/*				LISTA DE ADC A MEMORIA		*/
	adc_mem.SrcAddr = (uint32_t)&(LPC_ADC->ADDR0);
	adc_mem.DstAddr = (uint32_t)&primer_bloque;
	adc_mem.NextLLI = (uint32_t)&adc_mem; 	//buffer circular, la lista se apunta a si misma

	/* El size son la cantidad de posiciones que entran en el bloque, es decir 8K -> 0 - 8191 	*/
	adc_mem.Control = 8191
					| (2<<18) //ancho de 32 de fuente
					| (2<<21)	// 32 de llegada
					| (1<<27); // destino incremental.

	/*			   Channel para esta lista		*/
	GPDMA_Init();
	GPDMA_Channel_CFG_Type cfg_dma;
	cfg_dma.ChannelNum = 0;
	cfg_dma.TransferSize = 8191;
	cfg_dma.TransferWidth = 0; //unused
	cfg_dma.SrcMemAddr = 0;
	cfg_dma.DstMemAddr = (uint32_t)&primer_bloque;
	cfg_dma.TransferType = GPDMA_TRANSFERTYPE_P2M;
	cfg_dma.SrcConn = GPDMA_CONN_ADC;
	cfg_dma.DstConn = 0;
	cfg_dma.DMALLI = (uint32_t)&adc_mem;		//Acordarse que si encadenamos mas listas hay que pasarle la segunda no la primera.

	GPDMA_Setup(&cfg_dma);

	/*				LISTA DE WAVEFORM A DAC  */

	wf_dac.SrcAddr = (uint32_t)&segundo_bloque;
	wf_dac.DstAddr = (uint32_t)&LPC_DAC->DACR;
	wf_dac.NextLLI = (uint32_t) &wf_dac;
	//El tamaño es el mismo que el anterior porque es el segundo bloque
	wf_dac.Control = 8191
				   | (2<<18)
				   | (2<<21)
				   | (1<<26); //Fuente incremental
	cfg_dma.ChannelNum = 1; //canal 1.
	cfg_dma.SrcMemAddr = (uint32_t)&segundo_bloque;
	cfg_dma.DstMemAddr = 0;
	cfg_dma.TransferType = GPDMA_TRANSFERTYPE_M2P;
	cfg_dma.SrcConn = 0;
	cfg_dma.DstConn = GPDMA_CONN_DAC;
	cfg_dma.DMALLI = (uint32_t)&wf_dac;

	GPDMA_Setup(&cfg_dma);

	/*			LISTA DE MEM A DAC 		*/
	mem_dac = wf_dac;
	mem_dac.SrcAddr = (uint32_t)&primer_bloque;
	mem_dac.NextLLI = (uint32_t) &mem_dac;

	/*			Channel		*/
	cfg_dma.ChannelNum = 2;
	cfg_dma.SrcMemAddr =(uint32_t) &primer_bloque;
	cfg_dma.DMALLI = (uint32_t)&mem_dac;
	GPDMA_Setup(&cfg_dma);

}

void sincronizarArranque(){
	NVIC_EnableIRQ(EINT0_IRQn);
	GPDMA_ChannelCmd(0,ENABLE);
	GPDMA_ChannelCmd(1,ENABLE);
	GODMA_ChannelCmd(2,DISABLE);
	NVIC_EnableIRQ(DMA_IRQn);				//Duda para el profe. Como se startea el ADC con DMA.
}

void DMA_IRQHandler(){

	/*				La interrupcion externa modifica una variable de estado. Pero como debo esperar a la ultima conversion del adc
	 *  			antes de cambiar de "estado", lo activo y desactivo desde esta interrupcion.
	 */
	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 0) && estado==1){		//Si terminó el ADC y se cambió elestado hay que pasar al otro
			GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 0);
			GPDMA_ChannelCmd(0,DISABLE);
			GPDMA_ChannelCmd(1,DISABLE);
			GPDMA_ChannelCmd(2,ENABLE);
	}else if(GPDMA_IntGetStatus(GPDMA_STAT_INT, 0) && estado!=1){		//Si no simplemente limpio las flags.
		GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 0);
	}else if(GPDMA_IntGetStatus(GPDMA_STAT_INT, 1)){
		GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 1);
	}else{
		GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 2);
	}
}



void EINT0_IRQHandler(){
	estado++;
	if(estado>1){
		estado ==0;			//si es la segunda vez, vuelvo al estado inicial
		GPDMA_ChannelCmd(0,ENABLE);
		GPDMA_ChannelCmd(1,ENABLE);
		GODMA_ChannelCmd(2,DISABLE);
	}

	LPC_SC->EXTINT |= (1<<0);

}






