/*
Serial Terminal for the LaFortuna - Alexander Thomas (ajt1g19)
Intended for use as a matrix client with an ESP8266

*/
#include <stdlib.h>
#include "os/os.h"
#include "lcdexts.h"
#include "virtkb.h"

typedef enum {virtkeys, home} context;

context currentContext = home;
struct container* messageContainer;

char* currStr;//200 character string buffer
uint8_t currPosn = 0; //current position in string

void init(void);
int checkRotary(int state);

int checkButtons(int state);
void messageTransmit(char* message);

void main(void) {
	init();
	
	os_add_task(checkButtons, 100,1);
	sei();
	for(;;){}
	//display_string(test2);
	//display_char('A');
	//display_char_replace(' ','.',0,0);
}

void init(void) {
	os_init();

	init_lcd();

	EIMSK |= _BV(INT6); //enable tearing interrupt

	UBRR1H = (F_CPU/(9600*16L)-1) >> 8;//set baud rate to 9600
	UBRR1L = (F_CPU/(9600*16L)-1);
	UCSR1B = _BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1); //enable transmitter, reciever, interrupt on recieve complete (D.S. pp195)
	UCSR1C = _BV(UCSZ10) | _BV(UCSZ11);
	//Set data frame format: asynchronous mode,no parity, 1 stop bit, 8 bit size (Arduino IDE default - 8-N-1)
    UCSR1C=_BV(UCSZ11)|_BV(UCSZ10); 
	messageContainer = createContainer(0,0,53,30);
	currStr = calloc(200,sizeof(char));//200 character string buffer

	os_add_task(checkRotary, 16, 1);
	initKeyboard();
}

int checkRotary(int state) {
	int8_t delta = os_enc_delta();
	if(delta) {
		switch(currentContext) {
			case virtkeys:
				selectChange(delta);
				break;
			default:
				break;
		}
	}
	return state;
}

int checkButtons(int state) {
	switch(currentContext) {
		case virtkeys:
			if(get_switch_press(_BV(SWC))) {
				typeKey();
			}
			if(get_switch_press(_BV(SWN))) {
				appendChar(' ');	
			}
			if(get_switch_press(_BV(SWE))) {
				clear_screen();
				kbDisable();
				currentContext=home;
				char* stringResult = finishTyping();
				messageTransmit(stringResult);
				free(stringResult);
				containerSetEnabled(messageContainer, 1);
				containerRedraw(messageContainer);
			}
			break;
		case home:
			if(get_switch_press(_BV(SWE))) {
				drawKeyboard();
				containerSetEnabled(messageContainer,0);
				kbEnable();
				currentContext = virtkeys;
			}
			break;
	}

	return state;
}


ISR(INT6_vect) {
	doDraws();	
}


ISR(USART1_RX_vect) { //when a char has been recived from the ESP8266
	cli();
	*(currStr + currPosn) = UDR1;
	if(*(currStr + currPosn) == '\r' || *(currStr + currPosn) == '\n' || *(currStr + currPosn) == 0) {
		containerStr(messageContainer, currStr);
		for(char* currChar = currStr; *currChar; currChar++) {
			*currChar = 0;
		}
		currPosn = 0;
	} else {
		currPosn++;
		if(currPosn == 199) {
			containerStr(messageContainer, currStr);
			for(char* currChar = currStr; *currChar; currChar++) {
				*currChar = 0;
			}
			currPosn=0;
		}
	}
	sei();
}

void messageTransmit(char* str) {
	char *currentChar = str;
	while(*currentChar) {	
		while(! (UCSR1A & _BV(UDRE1))) {} //wait for empty transmit buffer
		UDR1 = *currentChar;
		currentChar++;
	}
	while(!(UCSR1A & _BV(UDRE1))) {}
	UDR1 = '\n';
}
	
