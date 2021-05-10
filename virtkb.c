/*

Virtual Keyboard for the LaFortuna - Alexander Thomas (ajt1g19)

This library requires my lcd extensions (lcdexts.h/c)

*/


#include <stdlib.h>
#include "os.h"
#include "lcdexts.h"
#include "virtkb.h"

const char topRow[] = {'Q','W','E','R','T','Y','U','I','O','P'};
#define TOP_LENGTH 	10
const char midRow[] = {'A','S','D','F','G','H','J','K','L'};
#define MID_LENGTH 	9
const char botRow[] = {'Z','X','C','V','B','N','M'};
#define BOT_LENGTH 	7

#define KEY_WIDTH ((uint16_t)32) // 320px/10 keys in a row 
#define MID_OFFSET ((uint16_t)8) // 32 * 0.25 to match UK keyboard layout
#define BOT_OFFSET ((uint16_t)24) // 32 * 0.75 to match UK keyboard layout
#define SURROUND_WIDTH 1
#define SURROUND_WIDTH_SELECTED 3
#define CHAR_XPOS ((uint16_t)13) //centre character within key
#define CHAR_YPOS ((uint16_t)12)
#define KEY_SPACING 1

struct stringBuilderElement {
	char c;
	struct stringBuilderElement* next;	
};

uint16_t stringLength = 0;
struct stringBuilderElement *currentString = NULL;
struct stringBuilderElement *currentStringEnd = NULL;

void appendCharToSB(char c) {
	struct stringBuilderElement *newElem = malloc(sizeof(struct stringBuilderElement));
	newElem->c = c;
	newElem->next=NULL;
	if(!currentString) {
		currentString = newElem;
		currentStringEnd = newElem;
	} else {
		currentStringEnd->next = newElem;
		currentStringEnd = newElem;
	}
	stringLength++;
}

uint8_t selectedKey = 0; 
struct container *container;

extern lcd display;

void drawKey(char key, uint16_t topLeftX, uint16_t topLeftY);
void selectKey(KBRow row, uint8_t index);
uint16_t getOffset(KBRow row);

void initKeyboard() {
	container = createContainer(0,0,53,13);	
}

void drawKeyboard() {
	clear_synced();
	for (uint16_t i = 0; i<TOP_LENGTH; i++) {
		drawKey(topRow[i], i*KEY_WIDTH, LCDWIDTH-4*KEY_WIDTH);
	}
	for (uint16_t i = 0; i<MID_LENGTH; i++) {
		drawKey(midRow[i], i*KEY_WIDTH+MID_OFFSET, LCDWIDTH-3*KEY_WIDTH);
	}
	for (uint16_t i = 0; i<BOT_LENGTH; i++) {
		drawKey(botRow[i], i*KEY_WIDTH+BOT_OFFSET, LCDWIDTH-2*KEY_WIDTH);
	}
	selectedKey = 0;
	selectKey(top,0);
}

void drawKey(char key, uint16_t topLeftX, uint16_t topLeftY ) {
	rectangle rectangle;
	rectangle.left = topLeftX;
	rectangle.top = topLeftY;
	rectangle.right = topLeftX + KEY_WIDTH - KEY_SPACING;
	rectangle.bottom = topLeftY + KEY_WIDTH - KEY_SPACING;
	stroke_rectangle_synced(rectangle, SURROUND_WIDTH, WHITE);
	move_synced(topLeftX+CHAR_XPOS, topLeftY + CHAR_YPOS);
	display_char_synced(key);
}

void unselectKey(KBRow row, uint8_t index) {
	rectangle rectangle;
	rectangle.left = getOffset(row) + KEY_WIDTH*index + SURROUND_WIDTH;
	rectangle.right = getOffset(row) + KEY_WIDTH*(index+1) - KEY_SPACING - SURROUND_WIDTH;
	rectangle.top = LCDWIDTH - (4-row) * KEY_WIDTH + SURROUND_WIDTH;
	rectangle.bottom = LCDWIDTH - (3-row) * KEY_WIDTH - KEY_SPACING - SURROUND_WIDTH;
	stroke_rectangle_synced(rectangle, SURROUND_WIDTH_SELECTED - SURROUND_WIDTH, display.background);
}


void selectKey(KBRow row, uint8_t index) {
	rectangle rectangle;
	rectangle.left = getOffset(row) + KEY_WIDTH*index + SURROUND_WIDTH;
	rectangle.right = getOffset(row) + KEY_WIDTH*(index+1) - KEY_SPACING - SURROUND_WIDTH;
	rectangle.top = LCDWIDTH - (4-row) * KEY_WIDTH + SURROUND_WIDTH;
	rectangle.bottom = LCDWIDTH - (3-row) * KEY_WIDTH - KEY_SPACING - SURROUND_WIDTH;
	stroke_rectangle_synced(rectangle, SURROUND_WIDTH_SELECTED-SURROUND_WIDTH, WHITE);
}

uint16_t getOffset(KBRow row) {
	switch(row) {
		case top:
			return 0;
			break;
		case mid:
			return MID_OFFSET;
			break;
		case bottom:
			return BOT_OFFSET;
			break;
	}
	return 0;
}

void selectChange(int8_t delta) {
	if(selectedKey<10) {
		unselectKey(top,selectedKey);
	} else if (selectedKey <19) {
		unselectKey(mid,selectedKey-10);
	} else {
		unselectKey(bottom,selectedKey-19);
	}
	selectedKey = (selectedKey+26+delta)%26;
	if(selectedKey < 10) {
		selectKey(top,selectedKey);
	} else if (selectedKey < 19) {
		selectKey(mid, selectedKey-10);
	} else {
		selectKey(bottom, selectedKey-19);
	}
}

void typeKey() {
	char selectedKeyChar;
	if(selectedKey<10) {
		selectedKeyChar = topRow[selectedKey];
	} else if (selectedKey < 19) {
		selectedKeyChar = midRow[selectedKey-10];
	} else {
		selectedKeyChar = botRow[selectedKey-19];
	}
	if(!get_switch_state(_BV(SWW))) {
		selectedKeyChar += ('a'-'A'); //toLower
	}
	containerChar(container,(char)(selectedKeyChar));
	appendCharToSB(selectedKeyChar);
}


void appendChar(char c) {
	containerChar(container,(char)(c));
	appendCharToSB(c);
}


char* finishTyping() {
	char* completedString = malloc((stringLength+1) * sizeof(char));
	char* currentChar = completedString;
	while (currentString) {
		struct stringBuilderElement *currentElem = currentString;
		*currentChar = currentElem->c;
		currentString = currentElem->next;
		free(currentElem);
		currentChar++;
	}
	*currentChar = 0;

	currentStringEnd = NULL;
	stringLength = 0;

	containerClear(container);

	return completedString;
}

void kbEnable() {
	containerSetEnabled(container, 1);
}

void kbDisable() {
	containerSetEnabled(container,0);
}





