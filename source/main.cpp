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
int bottomScreenView = 1;
bool flipped = 0;

void init() {
	fatInitDefault();
	sequencer = Sequencer::getInstance();
	graphicsEngine = GraphicsEngine::getInstance();
	soundEngine = SoundEngine::getInstance();
	topScreenViews.push_back((View*)(new SequencerView()));
	topScreenViews.push_back((View*)(new ScopeView));
	//bottomScreenViews.push_back((View*)(new TextTestView));
	bottomScreenViews.push_back((View*)(new SequencerView()));
	bottomScreenViews.push_back((View*)(new ScopeView));
	bottomScreenViews.push_back((View*)(new ControlsView));
	bottomScreenViews.push_back((View*)(new CommandView));
}

void handleInput() {
	scanKeys();
	int keys = keysDown();
	int held = keysHeld();

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
		if(keys & KEY_UP) topScreenView = wrap(topScreenView+1, topScreenViews.size());
		if(keys & KEY_DOWN) topScreenView = wrap(topScreenView-1, topScreenViews.size());
		if(keys & KEY_LEFT) bottomScreenView = wrap(bottomScreenView+1, bottomScreenViews.size());
		if(keys & KEY_RIGHT) bottomScreenView = wrap(bottomScreenView-1, bottomScreenViews.size());
		if(keys & KEY_R) flipped = !flipped;
	} else {
		if(flipped) {
			bottomScreenViews[bottomScreenView]->HandleInput(keys, held);
		} else {
			topScreenViews[topScreenView]->HandleInput(keys, held);
		}
	}
}

void drawTopScreen() {
	if(flipped) {
		bottomScreenViews[bottomScreenView]->Render();
	} else {
		topScreenViews[topScreenView]->Render();
	}
}

void drawBottomScreen() {
	if(!flipped) {
		bottomScreenViews[bottomScreenView]->Render();
	} else {
		topScreenViews[topScreenView]->Render();
	}
}

int main( void ) {
	init();
	while( 1 )
	{	
		// always draw first
		graphicsEngine->StartDrawing();
		if(graphicsEngine->currentScreen == GraphicsEngine::SCREEN_TOP) {
			drawTopScreen();
		} else {
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
