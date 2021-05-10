/*

Extensions to the lcd library - Alexander Thomas (ajt1g19)

*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdexts.h"
#include "ili934x.h"


extern char* font5x7;

struct drawTask* drawTasks = NULL;
struct drawTask* lastDrawTask = NULL; //end of queue

void addTask(struct drawTask* newTask) {
	if(!drawTasks) {
		drawTasks = newTask;
		lastDrawTask = newTask;
		lastDrawTask->next=NULL;
	} else {
		lastDrawTask->next = newTask;
		lastDrawTask = newTask;
		lastDrawTask->next=NULL;
	}
}


void doDraws() {
	while(drawTasks) {
		switch(drawTasks->op) {
			
			case fillRect:
				fill_rectangle(*(drawTasks->rectangle), *(drawTasks->col));
				free(drawTasks->rectangle);
				free(drawTasks->col);
				break;
			case fillRectIndexed:
				fill_rectangle_indexed(*(drawTasks->rectangle), drawTasks->col);
				if (drawTasks->completeFlag) {*(drawTasks->completeFlag) = 1;}
				free(drawTasks->rectangle);
				free(drawTasks->col);
				break;
			case move:
				display_move(*(drawTasks->x),*(drawTasks->y));
				free(drawTasks->x);
				free(drawTasks->y);
				break;
			case strokeRect:
				stroke_rectangle(*(drawTasks->rectangle), drawTasks->strokeWidth, *(drawTasks->col));
				free(drawTasks->rectangle);
				free(drawTasks->col);
				break;
			case dispStringConsume:
				display_string(drawTasks->str);
				free(drawTasks->str);
				break;
			case dispString:
				display_string(drawTasks->str);
				if (drawTasks->completeFlag) {*(drawTasks->completeFlag) = 1;}
				break;
			case dispStringXY: 
				display_string_xy(drawTasks->str,*(drawTasks->x),*(drawTasks->y));
				free(drawTasks->x);
				free(drawTasks->y);
				if (drawTasks->completeFlag) {*(drawTasks->completeFlag) = 1;}
				break;
			case dispChar:
				display_char(*(drawTasks->str));
				free(drawTasks->str);
				break;
			case clear:
				clear_screen();
				break;
			case setColours:
				display_color(*(drawTasks->col),*(drawTasks->bgCol));
				free(drawTasks->col);
				free(drawTasks->bgCol);
				break;
		}
		struct drawTask* currentTask = drawTasks;
		drawTasks = currentTask->next;
		free(currentTask);
	}
}

void move_synced(uint16_t x, uint16_t y) {
	struct drawTask *task = malloc(sizeof(struct drawTask));
	task->op = move;
	task->x = malloc(sizeof(uint16_t));
	task->y = malloc(sizeof(uint16_t));
	*(task->x) = x;
	*(task->y) = y;
	addTask(task);
}

void stroke_rectangle_synced(rectangle r, uint8_t strokeWidth, uint16_t col) {
	struct drawTask *task = malloc(sizeof(struct drawTask));
	task->op = strokeRect;
	task->rectangle = malloc(sizeof(rectangle));
	memcpy(task->rectangle, &r, sizeof(rectangle));
	task->strokeWidth = strokeWidth;
	task->col = malloc(sizeof(uint16_t));
	*(task->col) = col;
	addTask(task);
}

void fill_rectangle_synced(rectangle r, uint16_t col) {
	struct drawTask *task = malloc(sizeof(struct drawTask));
	task->op = fillRect;
	task->rectangle = malloc(sizeof(rectangle));
	memcpy(task->rectangle, &r, sizeof(rectangle));
	task->col = malloc(sizeof(col));
	*(task->col) = col;
	addTask(task);
}

void fill_rectangle_indexed_synced(rectangle r, uint16_t* col, char* completeFlag) {
	struct drawTask *task = malloc(sizeof(struct drawTask));
	task->op = fillRectIndexed;
	task->rectangle = malloc(sizeof(rectangle));
	memcpy(task->rectangle,&r,sizeof(rectangle));
	task->col = col;
	task->completeFlag = completeFlag;
	addTask(task);
}

void display_string_synced(char* str, char* completeFlag) {
	struct drawTask *task = malloc(sizeof(struct drawTask));
	task->op = dispString;
	task->str = str;//not sure if this is OK with object lifetimes
	task->completeFlag = completeFlag;
	addTask(task);
}

void display_string_synced_consume(char* str) {
	struct drawTask *task = malloc(sizeof(struct drawTask));
	task->op = dispStringConsume;
	task->str = str;
	addTask(task);
}

void display_char_synced(char c) {
	struct drawTask *task = malloc(sizeof(struct drawTask));
	task->op = dispChar;
	task->str = malloc(sizeof(char));
	*(task->str) = c;
	addTask(task);
}

void display_string_xy_synced(char* str, uint16_t x, uint16_t y, char* completeFlag) {
	struct drawTask *task = malloc(sizeof(struct drawTask));
	task->op = dispStringXY;
	task->str = str;
	task->x = malloc(sizeof(uint16_t));
	*(task->x) = x;
	task->y = malloc(sizeof(uint16_t));
	*(task->y) = y;
	task->completeFlag = completeFlag;
	addTask(task);
}

void clear_synced() {
	struct drawTask *task = malloc(sizeof(struct drawTask));
	task->op = clear;
	addTask(task);
}

void display_color_synced(uint16_t fg, uint16_t bg) {
	struct drawTask *task = malloc(sizeof(struct drawTask));
	task->op = setColours;
	task->col = malloc(sizeof(uint16_t));
	task->bgCol = malloc(sizeof(uint16_t));
	*(task->col) = fg;
	*(task->bgCol) = bg;
	addTask(task);
}

void stroke_rectangle(rectangle rect, uint8_t strokeWidth, uint16_t col) {
	uint16_t i;
	uint8_t j;
	write_cmd(COLUMN_ADDRESS_SET);
	write_data16(rect.left);
	write_data16(rect.right-1);
	write_cmd(PAGE_ADDRESS_SET);
	write_data16(rect.top);
	write_data16(rect.top+strokeWidth);
	write_cmd(MEMORY_WRITE);
	for (i = strokeWidth*(rect.right-rect.left); i>0; i--) {
		write_data16(col);
	}
	write_cmd(PAGE_ADDRESS_SET);
	write_data16(rect.bottom-strokeWidth);
	write_data16(rect.bottom);
	write_cmd(MEMORY_WRITE);
	for (i = strokeWidth*(rect.right-rect.left); i>0; i--) {
		write_data16(col);
	}
	write_cmd(COLUMN_ADDRESS_SET);
	write_data16(rect.left);
	write_data16(rect.left + strokeWidth -1);
	write_cmd(PAGE_ADDRESS_SET);
	write_data16(rect.top+strokeWidth);
	write_data16(rect.bottom-strokeWidth);
	write_cmd(MEMORY_WRITE);
	for(i = rect.bottom-strokeWidth; i>(rect.top+strokeWidth); i--) {
		for(j=strokeWidth; j; j--) {
			write_data16(col);
		}
	}
	write_cmd(COLUMN_ADDRESS_SET);
	write_data16(rect.right-strokeWidth);
	write_data16(rect.right-1);
	write_cmd(MEMORY_WRITE);
	for(i = rect.bottom-strokeWidth; i>(rect.top+strokeWidth); i--) {
		for(j=strokeWidth; j; j--) {
			write_data16(col);
		}
	}
}

struct container* createContainer(uint16_t x, uint16_t y, uint8_t cols, uint8_t rows) {
	uint8_t i;
	char** currentRow;
	struct container *container = malloc(sizeof(struct container));
	container->posX = 0;
	container->posY = 0;
	*(uint16_t *)&container->x=x;
	*(uint16_t *)&container->y=y;
	*(uint16_t *)&container->cols=cols;
	*(uint16_t *)&container->rows=rows;
	container->contents = malloc(rows*sizeof(char*));
	for(i = rows, currentRow=container->contents; i>0; i--, currentRow++) {
		*(currentRow) = calloc(cols+1,sizeof(char));
	}
	return container;
}

void destroyContainer(struct container* container) {
	uint8_t i;
	char** currentRow;
	for(i = container->rows, currentRow=container->contents; i>0; i--, currentRow++) {
		free(*currentRow);
	}
	free(container->contents);
	free(container);
}

void scrollContainer(struct container* container, uint8_t scrollRows) {
	uint8_t i,j;
	if(scrollRows<container->rows) {
		char** oldRows = calloc(scrollRows,sizeof(char*));
		for(i = 0; i<scrollRows; i++) { //save old rows
			*(oldRows+i) = *(container->contents + i);
			char* currentChar = *(oldRows+i);
			for(j = 0; j<container->cols; j++){
				*(currentChar+j) = ' ';
			}
		}

		for(i = 0; i+scrollRows < container->rows; i++) { //scroll rows up
			*(container->contents + i) = *(container->contents + scrollRows +  i);
			//*(container->contents+i) = "SCROLL";
		}
		for(i = 0; i<scrollRows;i++) {
			*(container->contents + container->rows -1 - i) = *(oldRows+i);
		}
		free(oldRows);
	} else {
		//populate all rows with 0s
		for(char** currentRow = container->contents; currentRow < container->contents + container->rows; currentRow++) { 
			for(char* currentChar = *currentRow; currentChar < *currentRow+container->cols; currentChar++) {
				*currentChar = ' ';
			}
		}
	}


	/*uint16_t currentY;
	for(currentRow = container->contents, currentY=container->y; currentRow < container->contents + container->rows; currentRow++, currentY+=8) {
		display_string_xy_synced(*currentRow, container-> x, currentY, NULL);
	}*/

	containerRedraw(container);

	container->posY = container->posY-scrollRows<=0 ? 0 : container->posY-scrollRows;
}

uint8_t test = 0;

void containerChar(struct container *container, char c) {
	if(c != '\n') {
		if(container->enabled) {
			move_synced(container->x+(6*container->posX), container->y+(8*container->posY));
			display_char_synced(c);
		}
		*(*(container->contents + container->posY) + container->posX) = c;
		container -> posX++;
		if(container->posX >= container->cols) {
			container->posX = 0;
			container->posY++;
		}
	} else {
		container->posX=0;
		container->posY++;
	}
	if(container->posY >= container->rows) {
		scrollContainer(container,1);
	}
}

void containerStr(struct container *container, char *str) {
	char* currentChar = str;
	while(*currentChar) {
		containerChar(container,*currentChar);
		currentChar++;
	}
}

void containerMove(struct container *container, uint8_t x, uint8_t y) {
	container->posX = x;
	container->posY = y;
}

void containerClear(struct container *container) {
	uint8_t i,j;
	rectangle rect;
	rect.left = container->x;
	rect.right = container->x + 6*container->cols;
	rect.top = container->y;
	rect.bottom = container->y + 8*container->rows;
	fill_rectangle_synced(rect, display.background);
	for(i=0;i<container->rows;i++) {
		char* currentRow = *(container->contents+i);
		for(j=0;j<container->cols;j++) {
			*(currentRow+j) = ' ';
		}
	}
	container -> posX = 0;
	container -> posY = 0;
}

void containerSetEnabled(struct container *container, uint8_t enabled) {
	container->enabled = enabled;
}

void containerRedraw(struct container *container) {
	if(container->enabled) {
		uint16_t currentY;
		char** currentRow;
		for(currentRow = container->contents, currentY=container->y; currentRow < container->contents + container->rows; currentRow++, currentY+=8) {
			display_string_xy_synced(*currentRow, container-> x, currentY, NULL);
		}
	}
}






