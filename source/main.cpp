#include <nds.h>
#include <gl2d.h>
#include <stdio.h>

#include "views.h"

GraphicsEngine* graphicsEngine;
SoundEngine* soundEngine;
Sequencer* sequencer;

std::vector<View*> views;
int topScreenView = 0;
int bottomScreenView = 1;

void init() {
	sequencer = Sequencer::getInstance();
	graphicsEngine = GraphicsEngine::getInstance();
	soundEngine = SoundEngine::getInstance();
	views.push_back((View*)(new SequencerView()));
	views.push_back((View*)(new ControlsView));
	views.push_back((View*)(new CommandView));
	views.push_back((View*)(new ScopeView));
}

void handleInput() {
	scanKeys();
	int keys = keysDown();
	int held = keysHeld();

	if(held & KEY_SELECT) {
		if(keys & KEY_UP) topScreenView = wrap(topScreenView+1, views.size());
		if(keys & KEY_DOWN) topScreenView = wrap(topScreenView-1, views.size());
		if(keys & KEY_LEFT) bottomScreenView = wrap(bottomScreenView+1, views.size());
		if(keys & KEY_RIGHT) bottomScreenView = wrap(bottomScreenView-1, views.size());
	} else {
		views[topScreenView]->HandleInput(keys, held);
	}
}

void drawTopScreen() {
	views[topScreenView]->Render();
}

void drawBottomScreen() {
	views[bottomScreenView]->Render();
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
