// SlidePot.cpp
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "SlidePot.h"
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
	uint32_t delay;
  GPIO_PORTB_AFSEL_R |= 0x30;     // 3) enable alternate fun
  GPIO_PORTB_DEN_R &= ~0x30;      // 4) disable digital I/O
  GPIO_PORTB_AMSEL_R |= 0x30;     // 5) enable analog fun
  SYSCTL_RCGCADC_R |= 0x03;       // 6) activate ADC0 
  delay = SYSCTL_RCGCADC_R;       // extra time to stabilize
  delay = SYSCTL_RCGCADC_R;       // extra time to stabilize
  delay = SYSCTL_RCGCADC_R;       // extra time to stabilize
  delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	
	
  ADC0_PC_R &= ~0xF;  
  ADC0_PC_R |= 0x01;               // 7) configure for 125K 
  //ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+5;  // 11) Ain9 (PE4)
	ADC0_SSPRI_R = 0x0123;
	ADC0_ACTSS_R &= ~0x0008;
	ADC0_EMUX_R &= ~0xF000;
	ADC0_SSMUX3_R &= ~0x000F;
	ADC0_SSMUX3_R += 10;
  ADC0_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
  ADC0_IM_R &= ~0x0008;           // 13) disable SS3 interrupts
  ADC0_ACTSS_R |= 0x0008;         // 14) enable sample sequencer 3
	ADC0_SAC_R = 10;
	
	ADC1_PC_R &= ~0xF;  
  ADC1_PC_R |= 0x01;               // 7) configure for 125K 
  //ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+5;  // 11) Ain9 (PE4)
	ADC1_SSPRI_R = 0x0123;
	ADC1_ACTSS_R &= ~0x0008;
	ADC1_EMUX_R &= ~0xF000;
	ADC1_SSMUX3_R &= ~0x000F;
	ADC1_SSMUX3_R += 11;
  ADC1_SSCTL3_R = 0x0006;     
  ADC1_IM_R &= ~0x0008;   
  ADC1_ACTSS_R |= 0x0008; 
	ADC1_SAC_R = 11;
}

//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC0_In(void){  
	uint32_t data;
  ADC0_PSSI_R = 0x0008;            
  while((ADC0_RIS_R&0x08)==0){};   
  data = ADC0_SSFIFO3_R&0xFFF; 
  ADC0_ISC_R = 0x0008; 
  return data;
}

uint32_t ADC1_In(void){  
	uint32_t data;
  ADC1_PSSI_R = 0x0008;            
  while((ADC1_RIS_R&0x08)==0){};   
  data = ADC1_SSFIFO3_R&0xFFF; 
  ADC1_ISC_R = 0x0008; 
  return data;
}

// constructor, invoked on creation of class
// m and b are linear calibration coeffients 
SlidePot::SlidePot(uint32_t m, uint32_t b){
	this->slope = m;
	this->offset = b;
}

void SlidePot::Save(uint32_t n){
	this->data = n;
	this->distance = Convert(n);
	this->flag = 1;
}
uint32_t SlidePot::Convert(uint32_t n){
  return (177*n)/4096+11;
}

void SlidePot::Sync(void){
	while(this->flag == 0){}
	this->flag = 0;
}

uint32_t SlidePot::ADCsample(void){ // return ADC sample value (0 to 4095)
  return this->data;
}

uint32_t SlidePot::Distance(void){  // return distance value (0 to 2000), 0.001cm
  return this->distance;
}


