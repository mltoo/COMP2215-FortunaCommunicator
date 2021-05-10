/*

Virtual Keyboard for the LaFortuna - Alexander Thomas (ajt1g19)
Recommended Configuration (This must be hooked up manually, this code doesnt have its own ISRs):
	- Rotary Encoder: Call selectChange() with delta
	- Centre Button: Call typeKey() on press
	- Up Arrow: Call appendChar(' ') on press
	- Right Arrow: Call finishTyping to get the final string, then call kbDisable

The left arrow acts as a shift modifier (this is handled already using FortunaOS functions)

This library requires my lcd extensions (lcdexts.h/c)

*/
#include <stdio.h>

extern const char topRow[];
extern const char midRow[];
extern const char botRow[];

extern const uint8_t keyWidth;
extern const uint8_t midOffset;
extern const uint8_t botOffset;

typedef enum {top=0, mid=1, bottom=2} KBRow;

extern struct container *container;

void initKeyboard();

void drawKeyboard();

void selectChange(int8_t delta);

void typeKey();

void appendChar(char c);

char* finishTyping(); //after calling, string must be freed. This becomes the reposibility of the caller.

void kbEnable();

void kbDisable();
