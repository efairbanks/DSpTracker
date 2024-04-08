#include <nds.h>
#include <gl2d.h>
#include <stdio.h>
#include <maxmod9.h>

#include "graphics.h"
#include "sound.h"
#include "sequencer.h"
#include "input.h"

touchPosition touchXY;
int cursorCol = 0;
int cursorRow = 0;

GraphicsEngine* graphicsEngine;
SoundEngine* soundEngine;
Sequencer* sequencer;

void init() {
	sequencer = Sequencer::getInstance();
	graphicsEngine = GraphicsEngine::getInstance();
	soundEngine = SoundEngine::getInstance();
}

void handleInput() {
	scanKeys();
	int keys = keysDown();
	int held = keysHeld();
	touchRead(&touchXY);

	if(held & KEY_A) {
		switch(keys) {
			case KEY_DOWN:
				sequencer->sequence.columns[cursorCol].rows[cursorRow].value-=16;
				break;
			case KEY_UP:
				sequencer->sequence.columns[cursorCol].rows[cursorRow].value+=16;
				break;
			case KEY_LEFT:
				sequencer->sequence.columns[cursorCol].rows[cursorRow].value-=1;
				break;
			case KEY_RIGHT:
				sequencer->sequence.columns[cursorCol].rows[cursorRow].value+=1;
				break;
		}
	} else if(held & KEY_X) {
		switch(keys) {
			case KEY_DOWN:
				sequencer->sequence.columns[cursorCol].rows[cursorRow].key-=16;
				break;
			case KEY_UP:
				sequencer->sequence.columns[cursorCol].rows[cursorRow].key+=16;
				break;
			case KEY_LEFT:
				sequencer->sequence.columns[cursorCol].rows[cursorRow].key-=1;
				break;
			case KEY_RIGHT:
				sequencer->sequence.columns[cursorCol].rows[cursorRow].key+=1;
				break;
		}
	} else if(held & KEY_R) {
		switch(keys) {
			case KEY_DOWN:
				sequencer->sequence.columns[cursorCol].rows.push_back(Row());
				break;
			case KEY_UP:
				sequencer->sequence.columns[cursorCol].rows.pop_back();
				break;
			case KEY_LEFT:
				break;
			case KEY_RIGHT:
				break;
		}
	} else {
		switch(keys) {
			case KEY_DOWN:
				cursorRow++;
				break;
			case KEY_UP:
				cursorRow--;
				break;
			case KEY_LEFT:
				cursorCol--;
				break;
			case KEY_RIGHT:
				cursorCol++;
				break;
		}
		if(cursorCol<0) cursorCol+=sequencer->sequence.columns.size();
		if(cursorCol>=sequencer->sequence.columns.size()) cursorCol-=sequencer->sequence.columns.size();
		if(cursorRow<0) cursorRow+=sequencer->sequence.columns[cursorCol].rows.size();
		if(cursorRow>=sequencer->sequence.columns[cursorCol].rows.size()) cursorRow-=sequencer->sequence.columns[cursorCol].rows.size();
	}
}

char keyToChar(int key) {
	switch(key) {
		case 0:
			return '-';
			break;
		default:
			return key+64;
			break;
	}
}

void drawScreen() {
	for(int columnIndex=0; columnIndex<sequencer->sequence.columns.size(); columnIndex++) {
		Column column = sequencer->sequence.columns[columnIndex];
		iprintf("\x1b[%d;%dH", 0, 2);
		int maxRowIndex = 0;
		for(int rowIndex=0; rowIndex<23; rowIndex++) {
			if(rowIndex<column.rows.size()) {
				iprintf("\x1b[%d;%dH%2d", rowIndex, 0, rowIndex);
				iprintf("\x1b[%d;%dH%1c%02X \n", rowIndex, columnIndex*4+3, keyToChar(column.rows[rowIndex].key), column.rows[rowIndex].value);
				maxRowIndex = std::max(maxRowIndex, rowIndex);
			} else {
				iprintf("\x1b[%d;%dH    \n", rowIndex, columnIndex*4+3);
			}
		}
	}

	glBegin2D();

	for(int i=0; i<soundEngine->scope.length - 1; i++) {
		s16 sample = soundEngine->scope.buffer[i];
		s16 nextSample = soundEngine->scope.buffer[i+1];
		glLine(
			i*(SCREEN_WIDTH/soundEngine->scope.length), (SCREEN_HEIGHT>>1) + ((sample*SCREEN_HEIGHT)>>13),
			(i+1)*(SCREEN_WIDTH/soundEngine->scope.length), (SCREEN_HEIGHT>>1) + ((nextSample*SCREEN_HEIGHT)>>13),
			RGB15(20,20,31)
		);
	}
	if(soundEngine->scope.IsReady()) soundEngine->scope.Reset();

	glLine(	17, 0,
			16, SCREEN_HEIGHT-1-8,
			RGB15(31,31,31));

	glBoxFilled(19, Sequencer::getInstance()->seqIndex*8,
				21, (Sequencer::getInstance()->seqIndex+1)*8-2,
				RGB15(31,31,31));

	glLine(	16 + cursorCol*32+5, cursorRow*8,
			15 + cursorCol*32+5, (cursorRow+1)*8-2,
			RGB15(31,31,31));

	glLine(	19 + (cursorCol+1)*32-1, cursorRow*8,
			18 + (cursorCol+1)*32-1, (cursorRow+1)*8-2,
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
