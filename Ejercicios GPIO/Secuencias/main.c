/*
 *Una famosa empresa de calzados a incorporado a sus zapatillas 10 luces leds comandadas
 *por un microcontrolador LPC1769 y ha pedido a su grupo de ingenieros que diseñen
 *2 secuencias de luces que cada cierto tiempo se vayan intercalando (secuencia A - secuencia B- secuencia A- ... ).
 *Como todavía no se ha definido la frecuencia a la cual va a funcionar el CPU del microcontrolador,
 *las funciones de retardos que se incorporen deben tener como parametros de entrada variables que permitan
 *modificar el tiempo de retardo que se vaya a utilizar finalmente. Se pide escribir el código que resuelva
 *este pedido, considerando que los leds se encuentran conectados en los puertos P0,0 al P0.9.
 */


#include "LPC17xx.h"
void confGpio(void);
void delay (uint32_t);




int main(void) {
    int secuenciaA[2]={0x1F,0x3E0};
	int secuenciaB[5]={0x3,0xF,0x3F,0xFF,0x3FF};

	SystemInit();
    confGpio();
    while(1) {
    	for(int i=0;i<2;i++){
    		LPC_GPIO0->FIOPIN=(secuenciaA[i]);
    		delay(7000000);
    	}
    	for(int j=0;j<5;j++){
    		LPC_GPIO0->FIOPIN=(secuenciaB[j]);
    		delay(4000000);
    	}

    }
    return 0 ;
}

void confGpio(void){
	LPC_PINCON->PINSEL0&=~(0xFFFFF);//Configura los pines del P0.0 al P0.9 como GPIO
	LPC_GPIO0->FIODIR|=(0x3FF);//Configura los pines del P0.0 al P0.9 como salida
	return;
}
void delay (uint32_t tiempo){
	for (int cont=0;cont<tiempo;cont++){}
}
