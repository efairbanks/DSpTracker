#include <nds.h>
#include <gl2d.h>
#include <stdio.h>
#include <maxmod9.h>

#include "graphics.h"
#include "sound.h"
#include "sequencer.h"
#include "input.h"

touchPosition touchXY;
int cursorX = 0;
int cursorY = 0;

void init() {
	GraphicsEngine::getInstance()->Init();
	SoundEngine::getInstance()->Init();
	Sequencer::getInstance()->Init();
}

void handleInput() {
	scanKeys();
	int keys = keysDown();
	int held = keysHeld();
	touchRead(&touchXY);

	if(held & KEY_A) {
		switch(keys) {
			case KEY_DOWN:
				Sequencer::getInstance()->seq[cursorY]--;
				break;
			case KEY_UP:
				Sequencer::getInstance()->seq[cursorY]++;
				break;
			case KEY_LEFT:
				Sequencer::getInstance()->seq[cursorY]-=16;
				break;
			case KEY_RIGHT:
				Sequencer::getInstance()->seq[cursorY]+=16;
				break;
		}
	} else {
		switch(keys) {
			case KEY_DOWN:
				cursorY++;
				break;
			case KEY_UP:
				cursorY--;
				break;
			case KEY_LEFT:
				cursorX--;
				break;
			case KEY_RIGHT:
				cursorX++;
				break;
			case KEY_B:
				SoundEngine::getInstance()->voice.PlayNote(220+rand()%200);
		}
	}
}

void drawScreen() {
	iprintf("\x1b[0;0H");
	for(int i=0; i<Sequencer::getInstance()->seq.size(); i++) {
		iprintf("%2d %02X\n", i, Sequencer::getInstance()->seq[i]);
	}

	glBegin2D();

	glLine(	17, 0,
			16, SCREEN_HEIGHT-1-8,
			RGB15(31,31,31));

	glBoxFilled(19, Sequencer::getInstance()->seqIndex*8,
				21, (Sequencer::getInstance()->seqIndex+1)*8-2,
				RGB15(31,31,31));

	glLine(	18 + cursorX*24+5, cursorY*8,
			17 + cursorX*24+5, (cursorY+1)*8-2,
			RGB15(31,31,31));

	glLine(	17 + (cursorX+1)*24-1, cursorY*8,
			16 + (cursorX+1)*24-1, (cursorY+1)*8-2,
			RGB15(31,31,31));



	glEnd2D();
	glFlush(0);

	swiWaitForVBlank();
}

int main( void ) {

	init();

	while( 1 )
	{		
		handleInput();
		drawScreen();				
	}
	
	return 0;
}
