#include <nds.h>
#include <fat.h>
#include <gl2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <fstream>

#include "views.h"

GraphicsEngine* graphicsEngine;
SoundEngine* soundEngine;
Sequencer* sequencer;

std::vector<View*> topScreenViews;
std::vector<View*> bottomScreenViews;
int topScreenView = 0;
int bottomScreenView = 0;
bool activeScreen = GraphicsEngine::SCREEN_TOP;

typedef enum {
	DIRPAD_L,
	DIRPAD_R,
	DIRPAD_U,
	DIRPAD_D,
	DIRPAD_LAST
} DirPadDir;
u32 dirPadHeld[DIRPAD_LAST];
u32 dirPadBitFlag[] = {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN};


void init() {
	fatInitDefault();
	sequencer = Sequencer::getInstance();
	graphicsEngine = GraphicsEngine::getInstance();
	soundEngine = SoundEngine::getInstance();
	topScreenViews.push_back((View*)(new SequencerView()));
	topScreenViews.push_back((View*)(new ScopeView));
	//bottomScreenViews.push_back((View*)(new TextTestView));
	bottomScreenViews.push_back((View*)(new TableView));
	bottomScreenViews.push_back((View*)(new ScopeView));
	bottomScreenViews.push_back((View*)(new SequencerView()));
	bottomScreenViews.push_back((View*)(new ControlsView));
	bottomScreenViews.push_back((View*)(new CommandView));
}

void handleInput() {
	scanKeys();
	u32 keys = keysDown();
	u32 held = keysHeld();
	u32 released = keysUp();

	for(int i=0; i<DIRPAD_LAST; i++) {
		if(held & dirPadBitFlag[i]) dirPadHeld[i]++;
		if(released & dirPadBitFlag[i]) dirPadHeld[i] = 0;
		if(dirPadHeld[i] > 15 && ((dirPadHeld[i]&0x3)==0)) keys = keys | dirPadBitFlag[i];
	}

	if((held & KEY_L) && (held & KEY_R)) {
		if(keys & KEY_START) {
			ofstream ostream;
			ostream.open("trackersave.bin", ofstream::binary);
			ostream.clear();
			ostream.seekp(0);
			sequencer->serialize(ostream);
			ostream.close();
		}
		if(keys & KEY_SELECT) {
			ifstream istream;
			istream.open("trackersave.bin", ifstream::binary);
			istream.clear();
			istream.seekg(0);
			sequencer->deserialize(istream);
			istream.close();
		}
	}

	if(held & KEY_SELECT) {
		if(keys & KEY_UP) activeScreen = !activeScreen;
		if(keys & KEY_DOWN) activeScreen = !activeScreen;
		if(keys & KEY_LEFT) {
			if(activeScreen == GraphicsEngine::SCREEN_BOTTOM) {
				bottomScreenView = wrap(bottomScreenView-1, bottomScreenViews.size());
			} else {
				topScreenView = wrap(topScreenView-1, topScreenViews.size());
			}
		}
		if(keys & KEY_RIGHT) {
			if(activeScreen == GraphicsEngine::SCREEN_BOTTOM) {
				bottomScreenView = wrap(bottomScreenView+1, bottomScreenViews.size());
			} else {
				topScreenView = wrap(topScreenView+1, topScreenViews.size());
			}
		}
	} else {
		if(activeScreen == GraphicsEngine::SCREEN_BOTTOM) {
			bottomScreenViews[bottomScreenView]->HandleInput(keys, held);
		} else {
			topScreenViews[topScreenView]->HandleInput(keys, held);
		}
		bottomScreenViews[bottomScreenView]->HandleTouchInput(keys, held);
	}
}

void drawTopScreen() {
	topScreenViews[topScreenView]->Render();
}

void drawBottomScreen() {
	bottomScreenViews[bottomScreenView]->Render();
}

int main( void ) {
	init();
	while(1)
	{	
		// always draw first
		graphicsEngine->StartDrawing();
		if(graphicsEngine->currentScreen == GraphicsEngine::SCREEN_TOP) {
			if(activeScreen == GraphicsEngine::SCREEN_TOP) {
				for(int i=1; i<8; i++) {
					glBoxFilled(0+i*16, 0+i*12, SCREEN_WIDTH-i*16, SCREEN_HEIGHT-i*12, (i&0x1) == 0 ? RGB15(0,0,0) : RGB15(2,2,2));
				}
			}
			drawTopScreen();
		} else {
			if(activeScreen == GraphicsEngine::SCREEN_BOTTOM) {
				for(int i=1; i<8; i++) {
					glBoxFilled(0+i*16, 0+i*12, SCREEN_WIDTH-i*16, SCREEN_HEIGHT-i*12, (i&0x1) == 0 ? RGB15(0,0,0) : RGB15(2,2,2));
				}
			}
			drawBottomScreen();
		}
		graphicsEngine->StopDrawing();

		// render audio and handle input
		soundEngine->RenderAudio();
		handleInput();

		// always wait for next vblank last
		swiWaitForVBlank();
	}
	
	return 0;
}
