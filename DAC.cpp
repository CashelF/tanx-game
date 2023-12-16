// put implementations for functions, explain how it works
// put your names here, date
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#include "DAC.h"

void DAC_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x10;
	__asm("nop");
	__asm("nop");
  GPIO_PORTE_DEN_R |= 0x3F;
	GPIO_PORTE_DIR_R |= 0x3F;
	GPIO_PORTE_DR8R_R |= 0x3F;
}

void DAC_Out(uint32_t data){
	GPIO_PORTE_DATA_R = data;
}
