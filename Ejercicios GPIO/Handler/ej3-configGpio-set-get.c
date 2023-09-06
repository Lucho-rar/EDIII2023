/*
 * Copyright 2022 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */


#include "LPC17xx.h"

void configGpio(int,int,int,int);
void getGpio(int);
void delay(uint32_t);

int main(void) {

   SystemInit();
   configGpio(12,0,22,1); //P0.22 digital output
    while(1) {
    	     LPC_GPIO0->FIOCLR|=(1<<22);//apago el led
    	     delay(6000000);
    	     LPC_GPIO0->FIOSET|=(1<<22);//enciendo el led
    	     delay(6000000);
    	     getGpio(0);
    }
    return 0 ;
}

void configGpio(int bit ,int port,int pin,int iO){
	if(port==0){
		if (pin>=0 && pin<=15){
			LPC_PINCON->PINSEL0&=(0b11<<bit);
		}
		else {
			LPC_PINCON->PINSEL1&=(0b11<<bit);
		}
	}
	if(port==1){
		if (pin>=0 && pin<=15){
				LPC_PINCON->PINSEL2&=(0b11<<bit);
		}
		else {
				LPC_PINCON->PINSEL3&=(0b11<<bit);
		}
	}
	if(port==2){
		if (pin>=0 && pin<=15){
			LPC_PINCON->PINSEL4&=(0b11<<bit);
		}
	}
	if(port==3){
		LPC_PINCON->PINSEL7&=(0b11<<bit);
	}
	if(port==4){
		LPC_PINCON->PINSEL9&=(0b11<<bit);
	}

	if(iO){//Output 1
		switch (port){
		case 0:
			   LPC_GPIO0->FIODIR|=(1<<pin);
			   break;
		case 1:
			   LPC_GPIO1->FIODIR|=(1<<pin);
			   break;
		case 2:
			   LPC_GPIO2->FIODIR|=(1<<pin);
			   break;
		case 3:
			   LPC_GPIO3->FIODIR|=(1<<pin);
			  break;
		case 4:
			  LPC_GPIO4->FIODIR|=(1<<pin);
			  break;
		}
	}else{//Input 0
		switch (port){
				case 0:
					   LPC_GPIO0->FIODIR&=~(1<<pin);
					   break;
				case 1:
					   LPC_GPIO1->FIODIR&=~(1<<pin);
					   break;
				case 2:
					   LPC_GPIO2->FIODIR&=~(1<<pin);
					   break;
				case 3:
					   LPC_GPIO3->FIODIR&=~(1<<pin);
					  break;
				case 4:
					  LPC_GPIO4->FIODIR&=~(1<<pin);
					  break;
				}
	}
}
void getGpio(int port){
	int lectura;
	switch (port){
					case 0:
						   lectura=LPC_GPIO0->FIOPIN;
						   break;
					case 1:
						 lectura=LPC_GPIO1->FIOPIN;
						   break;
					case 2:
						 lectura=LPC_GPIO2->FIOPIN;
						   break;
					case 3:
						 lectura=LPC_GPIO3->FIOPIN;
						  break;
					case 4:
						 lectura= LPC_GPIO4->FIOPIN;
						  break;
					}
}
void delay(uint32_t tiempo){
	for (int cont=0;cont<tiempo;cont++){}
}
