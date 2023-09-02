/*					Ejercicio Toggle
 *
 *				P0.0 salida - muestra la secuencia 1010100
 *				P1.0 entrada - cambia la secuencia a 0101101
 *
 *
 */


#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <stdio.h>
#include <stdbool.h>

void configGPIO(void);
void mostrarSecuenciaA();
void mostrarSecuenciaB();
bool testEntrada(void);
void delay(uint32_t);

int main(void) {

	configGPIO();
    while(1) {
        if(testEntrada() ==true){
        	mostrarSecuenciaA();
        }else{
        	mostrarSecuenciaB();
        }
    }
    return 0 ;
}

void mostrarSecuenciaA(){

	LPC_GPIO0->FIOSET |= 1;
	delay(2);
	LPC_GPIO0->FIOCLR |= 1;
	delay(2);
	LPC_GPIO0->FIOSET |= 1;
	delay(2);
	LPC_GPIO0->FIOSET |= 1;
	delay(2);
	LPC_GPIO0->FIOSET |= 1;
	delay(2);
	LPC_GPIO0->FIOCLR |= 1;
	delay(2);
	LPC_GPIO0->FIOCLR |= 1;
	delay(2);
}

void mostrarSecuenciaB(){
	LPC_GPIO0->FIOCLR |= 1;
	delay(2);
	LPC_GPIO0->FIOSET |= 1;
	delay(2);
	LPC_GPIO0->FIOCLR |= 1;
	delay(2);
	LPC_GPIO0->FIOSET |= 1;
	delay(2);
	LPC_GPIO0->FIOSET |= 1;
	delay(2);
	LPC_GPIO0->FIOCLR |= 1;
	delay(2);
	LPC_GPIO0->FIOSET |= 1;
	delay(2);

}

void configGPIO(){
	/* PINSEL */
	LPC_PINCON->PINSEL0 &= ~(0x3<<0);			//P0.0 GPIO
	LPC_PINCON->PINSEL2 &= ~(0x3<<0);			//P1.0 GPIO

	/* FIODIR */
	LPC_GPIO0->FIODIR |= (1<<0);		// P0.0 salida
	LPC_GPIO2->FIODIR |= (0<<0);		// P1.0 entrada

}

bool testEntrada(){
	if ((LPC_GPIO2->FIOPIN & 1) ==1 ){
		return true;			//Hay un 1 en la entrada
	}else{
		return false;			//No hay un 1 en la entrada
	}
}

void delay(uint32_t times) {
	for(uint32_t i=0; i<times; i++)
		for(uint32_t j=0; j<times; j++);
}

