#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <nds.h>
#include <gl2d.h>
#include <stdio.h>
#include "font.h"

class GraphicsEngine {
public:
  static GraphicsEngine * getInstance() {
    if(nullptr == instance) {
        instance = new GraphicsEngine();
        glScreen2D();

        videoSetMode( MODE_5_3D );
        videoSetModeSub(MODE_5_2D);
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
    return instance;
  };
  void DrawScope(s16* buffer, int length, u16 color) {
    for(int i=0; i<length - 1; i++) {
        s16 sample = buffer[i];
        s16 nextSample = buffer[i+1];
        glLine(
            i*(SCREEN_WIDTH/length), (SCREEN_HEIGHT>>1) + ((sample*SCREEN_HEIGHT)>>13),
            (i+1)*(SCREEN_WIDTH/length), (SCREEN_HEIGHT>>1) + ((nextSample*SCREEN_HEIGHT)>>13),
            color
        );
    }
  }
  ~GraphicsEngine() = default;
private:
  static GraphicsEngine * instance;
  GraphicsEngine() = default;
  GraphicsEngine(const GraphicsEngine&)= delete;
  GraphicsEngine& operator=(const GraphicsEngine&)= delete;
};
GraphicsEngine * GraphicsEngine::instance = nullptr;

#endif