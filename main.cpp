// main.cpp
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10 in C++

// Last Modified: 1/17/2020 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2017

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2017

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "SlidePot.h"
#include "Images.h"
#include "UART.h"
#include "Timer0.h"
#include "Timer1.h"
#include "sprites.h"
#include "Sound.h"
#include "Timer2.h"
#include <vector>
using namespace std;

SlidePot p1Slide(1500,0);
SlidePot p2Slide(1500,0);

#define XMAX 128
#define YMAX 160
#define nmlMAX 50
#define nmlMIN 110

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);


int languageFlag;

sprite player1(0,28,tank1,100, 17, 29);
sprite player2(112,160,tank2,100, 17, 29);



sprite player1shot((player1.x/2) - 3, player1.y - player1.h, bullet, 100, 4, 7, 0, 0);
sprite player2shot((player2.x/2) + 3, player2.y + player2.h, bullet, 100, 4, 7, 0, 0);

sprite tree1(0, 0, tree, 0, 20, 20);
sprite tree2(0, 0, tree, 0, 20, 20);
sprite tree3(0, 0, tree, 0, 20, 20);
sprite tree4(0, 0, tree, 0, 20, 20);
sprite trees[4] = {tree1, tree2, tree3, tree4};

sprite p1Health(0, 60, P1Health100, 0, 33, 10);
sprite p2Health(90, 110, P2Health100, 0, 33, 10);



extern int FsoundShoot;
extern int FsoundExp;

int p1shootCounter = 50;
int p2shootCounter = 50;

void portBInit(void){
	volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x02;
	__asm("nop");
	__asm("nop");
  GPIO_PORTB_DEN_R |= 0x0FF;
	GPIO_PORTB_DIR_R &= 0x00;
}


void shootChk(void){
  if(((GPIO_PORTB_DATA_R & 0x040) >> 6  == 1) && player1.shot == 0){
		player1.shot = 1;
		player1shot.x = player1.x + player1.w/2 - 2;
		player1shot.y = player1.y + player1shot.h - 4;
		p1shootCounter = 2000;
		FsoundShoot = 1;
	}
	if(((GPIO_PORTB_DATA_R & 0x080) >> 7 == 1) && player2.shot == 0){
		player2.shot = 1;
		player2shot.x = player2.x + player2.w/2 - 2;
		player2shot.y = player2.y - player2.h + 4;
		p2shootCounter = 2000;
		FsoundShoot = 1;
	}
}


//PB7:P2 shot; PB6: P1 shot; PB5: P2 slidepot; PB4: P1 slidepot; PB3: P2 up; PB2: P2 down; PB1: P1 up; PB0: P1 down
void move(void){ //for up and down only
  if((GPIO_PORTB_DATA_R & 0x03) != 0) player1.moved = 1;
	if((GPIO_PORTB_DATA_R & 0x0C) != 0) player2.moved = 1;
	p1Slide.Save(ADC0_In());
	p2Slide.Save(ADC1_In());
	shootChk();
	
	
	moveSlide(1);
	moveSlide(2);
}

void moveButton(uint32_t playerFlag){
	if(playerFlag == 1){
		if(((GPIO_PORTB_DATA_R & 0x02) >> 1 == 1) && player1.y >= player1.h) player1.y -= 2; //higher the added number, higher the velocity
		if(((GPIO_PORTB_DATA_R & 0x01) == 1) && player1.y <= nmlMAX) player1.y += 2;
	}
	
	else{
		if(((GPIO_PORTB_DATA_R & 0x08) >> 3 == 1) && (player2.y >= nmlMIN + player2.h)) player2.y -= 2;
		if(((GPIO_PORTB_DATA_R & 0x04) >> 2 == 1) && (player2.y <= 159)) player2.y +=2;
	}
}

void moveSlide(uint32_t playerFlag) {
	if(player1.x == 0xFFFFFFFF) player1.x = 0;
	if(player2.x == 0xFFFFFFFF) player2.x = 0;
	
	if(playerFlag == 1){
		if((p1Slide.Distance()< 30) && (player1.x > 0) && (player1.x < XMAX)){
			player1.moved = 1;
			player1.x -= 2;
		}
		else if((30 <= p1Slide.Distance()) && (p1Slide.Distance()< 60) && (player1.x > 0) && (player1.x < XMAX)){
			player1.moved = 1;
			player1.x -= 1;
		}

		
		else if((120 <= p1Slide.Distance()) && (p1Slide.Distance() < 150) && (player1.x < XMAX - player1.w)){
			player1.moved = 1;
			player1.x += 1;
		}
		else if((150 <= p1Slide.Distance()) && (player1.x < XMAX - player1.w)){
			player1.moved = 1;
			player1.x += 2;
		}
	}
	
	if(playerFlag == 2){
		if((p2Slide.Distance() < 30) && (player2.x > 0) && (player2.x < XMAX)){
			player2.moved = 1;
			player2.x -= 2;
		}
		else if((30 <= p2Slide.Distance()) && (p2Slide.Distance() < 60) && (player2.x > 0) && (player2.x < XMAX)){
			player2.moved = 1;
			player2.x -= 1;
		}

		
		else if((120 <= p2Slide.Distance()) && (p2Slide.Distance() < 150) && (player2.x < XMAX - player2.w)){
			player2.moved = 1;
			player2.x += 1;
		}
		else if((150 <= p2Slide.Distance()) && (player2.x < XMAX - player2.w)){
			player2.moved = 1;
			player2.x += 2;
		}
	}
}


int shotCollide(int pBullet, sprite s){
	if(pBullet == 2) return (((player2shot.y - player2shot.h + 1) <= (s.y - 2)) && ((player2shot.y - player2shot.h + 1) >= s.y - s.h + 2) && 
		((((player2shot.x + player2shot.w - 1) >= (s.x + 2)) && ((player2shot.x + player2shot.w - 1) <= (s.x + s.w - 2))) || (((player2shot.x + 1) <= (s.x + s.w - 2)) && ((player2shot.x + 1) >= (s.x + 2)))));
	else return (((player1shot.y + player1shot.h - 1) >= (s.y - s.h + 2)) && ((player1shot.y + player1shot.h - 1) <= s.y - 2) && 
		((((player1shot.x + player1shot.w - 1) >= (s.x + 2)) && ((player1shot.x + player1shot.w - 1) <= (s.x + s.w - 2))) || (((player1shot.x + 1) <= (s.x + s.w - 2)) && ((player1shot.x + 1) >= (s.x + 2)))));
}


void run(){	
  while(player1.health > 0 && player2.health > 0){
		if(Random32() % 1000000 == 5){
			int r = Random()%4;
			int i = r;
			do{
				if(trees[i].health <= 0){
					trees[i].health = 40;
					if(i == 0) trees[i].x = Random() % 12;
					if(i == 1) trees[i].x = Random() % 12 + 32;
					if(i == 2) trees[i].x = Random() % 12 + 64;
					if(i == 3) trees[i].x = Random() % 12 + 96;
					trees[i].y = Random() % 40 + 50;
					if(Random() % 2 == 0){
						ST7735_DrawBitmap(trees[i].x, trees[i].y, tree, trees[i].w, trees[i].h);
						ST7735_DrawBitmap(p1Health.x, p1Health.y, p1Health.image, p1Health.w, p1Health.h); // player1 health
						ST7735_DrawBitmap(p2Health.x, p2Health.y, p2Health.image, p2Health.w, p2Health.h); // player2 health
					}
					else{
						ST7735_DrawBitmap(trees[i].x, trees[i].y, tree180, trees[i].w, trees[i].h);
						ST7735_DrawBitmap(p1Health.x, p1Health.y, p1Health.image, p1Health.w, p1Health.h); // player1 health
						ST7735_DrawBitmap(p2Health.x, p2Health.y, p2Health.image, p2Health.w, p2Health.h); // player2 health
					}
					break;
				}
				i++;
				i%=4;
			}while(i != r);
		}
		
		
    if(player1.moved){
			player1.moved = 0;
			moveButton(1);
			ST7735_DrawBitmap(player1.x, player1.y, player1.image, player1.w, player1.h);
		}
		if(player2.moved){
			player2.moved = 0;
			moveButton(2);
			ST7735_DrawBitmap(player2.x, player2.y, player2.image, player2.w, player2.h);
		}
		
		if(player1.shot){
			ST7735_DrawBitmap(player1shot.x, player1shot.y, player1shot.image, player1shot.w, player1shot.h);
			if(shotCollide(1, player2)){
				ST7735_DrawBitmap(player1shot.x, player1shot.y, bulletGone, player1shot.w, player1shot.h);
				FsoundExp = 1;
				player1.shot = 0;
				player2.health -= 20;
				if(player2.health == 80) p2Health.image = P2Health80;
				if(player2.health == 60) p2Health.image = P2Health60;
				if(player2.health == 40) p2Health.image = P2Health40;
				if(player2.health == 20) p2Health.image = P2Health20;
				ST7735_DrawBitmap(p2Health.x, p2Health.y, p2Health.image, p2Health.w, p2Health.h);
				ST7735_DrawBitmap(p1Health.x, p1Health.y, p1Health.image, p1Health.w, p1Health.h);
			}
			
			
			for(int i = 0; i < 4; i++){
				if(shotCollide(1, trees[i]) && (trees[i].health > 0)){
					ST7735_DrawBitmap(player1shot.x, player1shot.y, bulletGone, player1shot.w, player1shot.h);
					FsoundExp = 1;
					player1.shot = 0;
					trees[i].health -= 20;
					if(trees[i].health <= 0) ST7735_DrawBitmap(trees[i].x, trees[i].y, treeGone, trees[i].w, trees[i].h);
					ST7735_DrawBitmap(p1Health.x, p1Health.y, p1Health.image, p1Health.w, p1Health.h); // player1 health
					ST7735_DrawBitmap(p2Health.x, p2Health.y, p2Health.image, p2Health.w, p2Health.h); // player2 health
					break;
				}
			}
			
			
			if(player1shot.y - 1 >= 160){
				ST7735_DrawBitmap(player1shot.x, player1shot.y, bulletGone, player1shot.w, player1shot.h);
				ST7735_DrawBitmap(p1Health.x, p1Health.y, p1Health.image, p1Health.w, p1Health.h);
				ST7735_DrawBitmap(p2Health.x, p2Health.y, p2Health.image, p2Health.w, p2Health.h);
				player1.shot = 0;
			}
			p1shootCounter--;
			if(p1shootCounter == 0) {
				p1shootCounter = 50;
				player1shot.y++;
			}
		}
		if(player2.shot) {
			ST7735_DrawBitmap(player2shot.x, player2shot.y, player2shot.image, player2shot.w, player2shot.h);
			if(shotCollide(2, player1)){
				ST7735_DrawBitmap(player2shot.x, player2shot.y, bulletGone, player2shot.w, player2shot.h);
				FsoundExp = 1;
				player2.shot = 0;
				player1.health -= 20;
				if(player1.health == 80) p1Health.image = P1Health80;
				if(player1.health == 60) p1Health.image = P1Health60;
				if(player1.health == 40) p1Health.image = P1Health40;
				if(player1.health == 20) p1Health.image = P1Health20;
				ST7735_DrawBitmap(p1Health.x, p1Health.y, p1Health.image, p1Health.w, p1Health.h);
				ST7735_DrawBitmap(p2Health.x, p2Health.y, p2Health.image, p2Health.w, p2Health.h);

			}
			
			
			
			for(int i = 0; i < 4; i++){
				if(shotCollide(2, trees[i]) && (trees[i].health > 0)){
					ST7735_DrawBitmap(player2shot.x, player2shot.y, bulletGone, player2shot.w, player2shot.h);
					FsoundExp = 1;
					player2.shot = 0;
					trees[i].health -= 20;
					if(trees[i].health <= 0) ST7735_DrawBitmap(trees[i].x, trees[i].y, treeGone, trees[i].w, trees[i].h);
					ST7735_DrawBitmap(p1Health.x, p1Health.y, p1Health.image, p1Health.w, p1Health.h); // player1 health
					ST7735_DrawBitmap(p2Health.x, p2Health.y, p2Health.image, p2Health.w, p2Health.h); // player2 health
					break;
				}
			}
			
			
			if(player2shot.y - player2shot.h + 1 <= 0){
				ST7735_DrawBitmap(player2shot.x, player2shot.y, bulletGone, player2shot.w, player2shot.h);
				ST7735_DrawBitmap(p1Health.x, p1Health.y, p1Health.image, p1Health.w, p1Health.h);
				ST7735_DrawBitmap(p2Health.x, p2Health.y, p2Health.image, p2Health.w, p2Health.h);
				player2.shot = 0;
			}
			p2shootCounter--;
			if(p2shootCounter == 0) {
				p2shootCounter = 50;
				player2shot.y--;
			}
		}
  }
}

int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  //TExaS_Init();
  Random_Init(5);
	ST7735_InitR(INITR_REDTAB);
	portBInit();
	ADC_Init();
	Sound_Init(); //sets sound on Timer0
	Timer1_Init(&move,1600000); // 50 Hz
	ST7735_InvertDisplay(1);
	EnableInterrupts();

	while(1){
		if((p1Slide.Distance()< 90)) {
			languageFlag = 1;
			ST7735_DrawBitmap(0, 160, titlescreen1, 128, 160); // english select screen
		}else{
		languageFlag = 2;
		ST7735_DrawBitmap(0, 160, titlescreen2, 128, 160); // spanish select screen
	}
	if((GPIO_PORTB_DATA_R & 0x40) >> 6 == 1) break;
}
	DisableInterrupts();
	
		
  //Timer2_Init(&Sound_Run,791568254); // every 9 secs
	ST7735_FillScreen(0x0000);
	//ST7735_DrawBitmap(0, 160, ForestBackground, 128, 160);
	
  ST7735_DrawBitmap(player1.x, player1.y, player1.image, player1.w, player1.h); // player1 tank bottom left
	ST7735_DrawBitmap(player2.x, player2.y, player2.image, player2.w, player2.h); // player2 tank top right
	ST7735_DrawBitmap(p1Health.x, p1Health.y, P1Health100, p1Health.w, p1Health.h); // player1 health
	ST7735_DrawBitmap(p2Health.x, p2Health.y, P2Health100, p2Health.w, p2Health.h); // player2 health
	EnableInterrupts();

	run();

	if(languageFlag == 1){
		ST7735_FillScreen(0x0000);
		ST7735_DrawBitmap(20, 95, GameOverEnglish, 80, 57);
		if(player1.health > 0) ST7735_DrawBitmap(25, 121, P1winEnglish, 73, 14); // english p1 wins
		else ST7735_DrawBitmap(25, 121, P2winEnglish, 75, 14); // english select screen
	}else{
		ST7735_FillScreen(0x0000);
		ST7735_DrawBitmap(20, 100, GameOverSpanish, 80, 53);
		if(player1.health > 0) ST7735_DrawBitmap(29, 114, P1winSpanish, 63, 12); // english p1 wins
		else ST7735_DrawBitmap(29, 114, P2winSpanish, 66, 13); // english select screen
	}

}




