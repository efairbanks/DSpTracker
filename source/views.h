#ifndef VIEWS_H
#define VIEWS_H

#include <nds.h>
#include <gl2d.h>
#include <stdio.h>

#include "graphics.h"
#include "sound.h"
#include "sequencer.h"

class View {	
public:
	virtual void HandleInput(int keys, int held) = 0;
    virtual void Render() = 0;
};

class SequencerView {
    int cursorCol = 0;
    int cursorRow = 0;
public:
    virtual void HandleInput(int keys, int held) {
        touchPosition touchXY;
        touchRead(&touchXY);
        scanKeys();

        if(held & KEY_A) {
            switch(keys) {
                case KEY_DOWN:
                    Sequencer::getInstance()->sequence.columns[cursorCol].rows[cursorRow].value-=16;
                    break;
                case KEY_UP:
                    Sequencer::getInstance()->sequence.columns[cursorCol].rows[cursorRow].value+=16;
                    break;
                case KEY_LEFT:
                    Sequencer::getInstance()->sequence.columns[cursorCol].rows[cursorRow].value-=1;
                    break;
                case KEY_RIGHT:
                    Sequencer::getInstance()->sequence.columns[cursorCol].rows[cursorRow].value+=1;
                    break;
            }
        } else if(held & KEY_X) {
            switch(keys) {
                case KEY_DOWN:
                    Sequencer::getInstance()->sequence.columns[cursorCol].rows[cursorRow].key-=16;
                    break;
                case KEY_UP:
                    Sequencer::getInstance()->sequence.columns[cursorCol].rows[cursorRow].key+=16;
                    break;
                case KEY_LEFT:
                    Sequencer::getInstance()->sequence.columns[cursorCol].rows[cursorRow].key-=1;
                    break;
                case KEY_RIGHT:
                    Sequencer::getInstance()->sequence.columns[cursorCol].rows[cursorRow].key+=1;
                    break;
            }
        } else if(held & KEY_R) {
            switch(keys) {
                case KEY_DOWN:
                    Sequencer::getInstance()->sequence.columns[cursorCol].rows.push_back(Row());
                    break;
                case KEY_UP:
                    Sequencer::getInstance()->sequence.columns[cursorCol].rows.pop_back();
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
            if(cursorCol<0) cursorCol+=Sequencer::getInstance()->sequence.columns.size();
            if(cursorCol>=Sequencer::getInstance()->sequence.columns.size()) cursorCol-=Sequencer::getInstance()->sequence.columns.size();
            if(cursorRow<0) cursorRow+=Sequencer::getInstance()->sequence.columns[cursorCol].rows.size();
            if(cursorRow>=Sequencer::getInstance()->sequence.columns[cursorCol].rows.size()) cursorRow-=Sequencer::getInstance()->sequence.columns[cursorCol].rows.size();
        }
    }
    virtual void Render() {
        static char TEXBUF[256];
        for(int columnIndex=0; columnIndex<Sequencer::getInstance()->sequence.columns.size(); columnIndex++) {
            Column column = Sequencer::getInstance()->sequence.columns[columnIndex];
            int maxRowIndex = 0;
            for(int rowIndex=0; rowIndex<23; rowIndex++) {
                if(rowIndex<column.rows.size()) {
                    printf(0, 8*rowIndex, "%2d", rowIndex);
                    printf((columnIndex*4+3)*8, rowIndex*8, "%1c%02X ", Row::KeyToChar(column.rows[rowIndex].key), column.rows[rowIndex].value);
                    maxRowIndex = std::max(maxRowIndex, rowIndex);
                }
            }
        }

        glLine(	18, 0,
                17, SCREEN_HEIGHT-1-8,
                RGB15(31,31,31));

        glLine(	18 + cursorCol*32+5, cursorRow*8+1,
                17 + cursorCol*32+5, (cursorRow+1)*8-1,
                RGB15(31,31,31));

        glLine(	21 + (cursorCol+1)*32-1, cursorRow*8+1,
                20 + (cursorCol+1)*32-1, (cursorRow+1)*8-1,
                RGB15(31,31,31));
    }
};

class ScopeView {
    int cursorCol = 0;
    int cursorRow = 0;
public:
    virtual void HandleInput(int keys, int held) {}
    virtual void Render() {
        glBoxFilled(0, 0,
                    SCREEN_WIDTH, SCREEN_HEIGHT,
                    RGB15(0,0,0));

        GraphicsEngine::getInstance()->DrawScope(SoundEngine::getInstance()->scope.buffer, SoundEngine::getInstance()->scope.length, RGB15(20,20,31));
        if(SoundEngine::getInstance()->scope.IsReady()) SoundEngine::getInstance()->scope.Reset();
    }
};

#endif