#include <nds.h>
#include <gl2d.h>
#include <stdio.h>
#include "font.h"

class GraphicsEngine {
public:
  static GraphicsEngine * getInstance() {
    if(nullptr == instance) {
      instance = new GraphicsEngine();
    }
    return instance;
  };
  void Init() {
	glScreen2D();

    videoSetMode( MODE_5_3D );
	vramSetBankC(VRAM_C_MAIN_BG_0x06000000);
	PrintConsole *console = consoleInit(0,1, BgType_Text4bpp, BgSize_T_256x256, 31,0, true, true);
	bgSetPriority(0, 1);

	ConsoleFont font;
	font.gfx = (u16*)fontTiles;
	font.pal = (u16*)fontPal;
	font.numChars = 95;
	font.numColors =  fontPalLen / 2;
	font.bpp = 4;
	font.asciiOffset = 32;
	font.convertSingleColor = false;
	consoleSetFont(console, &font);
  }
  ~GraphicsEngine() = default;
private:
  static GraphicsEngine * instance;
  GraphicsEngine() = default;
  GraphicsEngine(const GraphicsEngine&)= delete;
  GraphicsEngine& operator=(const GraphicsEngine&)= delete;
};
GraphicsEngine * GraphicsEngine::instance = nullptr;