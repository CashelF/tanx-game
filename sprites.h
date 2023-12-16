// sprites.h
// Runs on TM4C123 or LM4F120
// Prototypes for basic functions for sprites
// tan(x)
// Cashel Fitzgerald
// 5-1-2022
#ifndef __SPRITES_h__
#define __SPRITES_h__


#include <stdint.h>
#include "Images.h"
#include "../inc/tm4c123gh6pm.h"
#include "SlidePot.h"

class sprite{
public:
	uint32_t x;      // x coordinate
  uint32_t y;      // y coordinate
  const unsigned short *image; // ptr->image
  int health;            // hit pts
	uint32_t w;
	uint32_t h;
	int moved;
	int shot;
	int hit;
	int playerShot;

	sprite(uint32_t x, uint32_t y, const unsigned short *image, int health, uint32_t w, uint32_t h){
		this->x = x;
		this->y =y;
		this->image = image;
		this->health = health;
		this->w = w;
		this->h = h;
	}
	
	//constructor for bullet
	sprite(uint32_t x, uint32_t y, const unsigned short *image, int health, uint32_t w, uint32_t h, int shot, int hit){
		this->x = x;
		this->y =y;
		this->image = image;
		this->health = health;
		this->w = w;
		this->h = h;
		this->shot= shot;
		this->hit = hit;
	}
	
};


void shoot(void);

void moveButton(uint32_t playerFlag);

void moveSlide(uint32_t playerFlag);
void move(void);

void portBInit(void);
	

#endif
