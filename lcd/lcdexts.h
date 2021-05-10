/*
Extensions to the lcd library - Alexander Thomas (ajt1g19)

This library provides:
- Delayed draw calls
- Text containers
- Outline rectangles

*/

#pragma once
#ifndef OS_H 
#include "lcd.h"
#endif


enum opType {fillRect, fillRectIndexed, move, strokeRect, dispString, dispStringConsume, dispChar, dispStringXY, clear, setColours};

struct drawTask {
	enum opType op;
	rectangle *rectangle;
	uint8_t strokeWidth;
	uint16_t *col;
	uint16_t *x;
	uint16_t *y;
	char *str;
	char *completeFlag;
	uint16_t *bgCol;
	struct drawTask *next; //next task in queue
};

struct container {
	const uint16_t x;
	const uint16_t y;
	const uint8_t cols;
	const uint8_t rows;
	uint8_t posX;
	uint8_t posY;
	char** contents;
	uint8_t enabled;
};


void stroke_rectangle(rectangle r, uint8_t strokeWidth, uint16_t col);



void doDraws(); // call when ready to do draw operations
void move_synced(uint16_t x, uint16_t y);
void stroke_rectangle_synced(rectangle r, uint8_t strokeWidth, uint16_t col);
void fill_rectangle_synced(rectangle rect, uint16_t col);
void fill_rectangle_indexed_synced(rectangle rect, uint16_t* col, char* completeFlag); //completeFlag set true when writing to colour array is OK again
void display_char_synced(char c);
void display_string_synced(char* str, char* completeFlag);
void display_string_synced_consume(char* str);//takes responsibility for freeing string after draw completes
void display_string_xy_synced(char* str, uint16_t x, uint16_t y, char* completeFlag);
void clear_synced();
void display_color_synced(uint16_t fg, uint16_t bg);


void display_string_contained(rectangle container, char* str, int xOffset, int yOffset);
struct container* createContainer(uint16_t x, uint16_t y, uint8_t cols, uint8_t rows);
void scrollContainer(struct container*, uint8_t scrollRows);
void containerMove(struct container *container, uint8_t x, uint8_t y);
void containerChar(struct container *container, char c);
void containerStr(struct container *container, char* str);
void containerClear(struct container *container);
void containerSetEnabled(struct container* container, uint8_t enabled);
void containerRedraw(struct container* container);
