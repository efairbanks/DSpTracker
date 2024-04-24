#ifndef VIEWS_H
#define VIEWS_H

#include <nds.h>
#include <gl2d.h>
#include <stdio.h>

#include "graphics.h"
#include "sound.h"
#include "sequencer.h"

using namespace std;

class View {	
public:
	virtual void HandleInput(int keys, int held) = 0;
    virtual void Render() = 0;
};

class SequencerView {
    Sequence copiedSequence;
    Column copiedColumn;
    Row copiedRow;
    int cursorCol = 0;
    int cursorRow = 0;
    u8 currentSequence = 0;
public:
    virtual void HandleInput(int keys, int held) {
        touchPosition touchXY;
        touchRead(&touchXY);
        scanKeys();

        Sequence& sequence = Sequencer::getInstance()->sequences[currentSequence];
        Column& column = sequence.columns[cursorCol];
        Row& row = column.rows[cursorRow];
        int numCols = sequence.columns.size();
        int numRows = column.rows.size();

        if((held & KEY_L) && (keys & KEY_START)) {
            Sequencer::getInstance()->Reset();
        } else if(keys & KEY_START) {
            if(sequence.playing) {
                sequence.playing = false;
            } else {
                sequence.playing = true;
            }
        }

        if(held & KEY_A) {
            // modify value under cursor
            if(keys & KEY_DOWN) row.value-=16;
            if(keys & KEY_UP) row.value+=16;
            if(keys & KEY_LEFT) row.value-=1;
            if(keys & KEY_RIGHT) row.value+=1;
        } else if(held & KEY_X) {
            // modify command under cursor
            if(keys & KEY_DOWN) row.key-=16;
            if(keys & KEY_UP) row.key+=16;
            if(keys & KEY_LEFT) row.key-=1;
            if(keys & KEY_RIGHT) row.key+=1;
        } else if((held & KEY_L) && (held & KEY_R)) {
            if(keys & KEY_DOWN) column.ticksPerStep -= 16;
            if(keys & KEY_UP) column.ticksPerStep += 16;
            if(keys & KEY_LEFT) column.ticksPerStep--;
            if(keys & KEY_RIGHT) column.ticksPerStep++;
        } else if(held & KEY_R) {
            // add/subtract columns
            if((keys & KEY_LEFT) && sequence.columns.size() > 1) sequence.columns.pop_back();
            if(keys & KEY_RIGHT) sequence.columns.push_back(Column(1));
            // add/subtract rows
            if(keys & KEY_DOWN) column.rows.push_back(Row());
            if((keys & KEY_UP) && column.rows.size() > 1) column.rows.pop_back();
            if(keys & KEY_B) copiedColumn = column;
            if(keys & KEY_Y) sequence.columns[cursorCol] = copiedColumn;

        } else if(held & KEY_L) {
            // switch sequence
            if(keys & KEY_LEFT) currentSequence--;
            if(keys & KEY_RIGHT) currentSequence++;
            if(keys & KEY_DOWN) currentSequence-=16;
            if(keys & KEY_UP) currentSequence+=16;
            if(keys & KEY_B) copiedSequence = sequence;
            if(keys & KEY_Y) Sequencer::getInstance()->sequences[currentSequence] = copiedSequence;
        } else {
            // move cursor, wrapping
            if(keys & KEY_LEFT) cursorCol = wrap(cursorCol-1, numCols);
            if(keys & KEY_RIGHT) cursorCol = wrap(cursorCol+1, numCols);
            if(keys & KEY_DOWN) cursorRow = wrap(cursorRow+1, numRows);
            if(keys & KEY_UP) cursorRow = wrap(cursorRow-1, numRows);
            if(keys & KEY_B) copiedRow = row;
            if(keys & KEY_Y) column.rows[cursorRow] = copiedRow;
        }
        cursorCol = max(0, min(cursorCol, (int)(sequence.columns.size()-1)));
        cursorRow = max(0, min(cursorRow, (int)(column.rows.size()-1)));
    }
    virtual void Render() {
        static char TEXBUF[256];

        int colPadding = 26;

        glBoxFilled(
            cursorCol*colPadding+24, cursorRow*8,
            cursorCol*colPadding+49, (cursorRow+1)*8,
            RGB15(25,25,25)
        );

        glBoxFilled(0, SCREEN_HEIGHT-8, SCREEN_WIDTH, SCREEN_HEIGHT, RGB15(31,26,26));
        printf(0, SCREEN_HEIGHT-8, RGB15(0,0,0), "%02X", currentSequence);
        for(int i=0; i<Sequencer::getInstance()->sequences[currentSequence].columns.size(); i++) {
            Column& column = Sequencer::getInstance()->sequences[currentSequence].columns[i];
            printf(24+26*i, SCREEN_HEIGHT-8, RGB15(0,0,0), "%2X", column.ticksPerStep);
        }

        // render seq index line separator
        glLine(	18, 0, 17, SCREEN_HEIGHT-1-8, RGB15(31,31,31));
        // render seq
        for(int columnIndex=0; columnIndex<Sequencer::getInstance()->sequences[currentSequence].columns.size(); columnIndex++) {
            Column column = Sequencer::getInstance()->sequences[currentSequence].columns[columnIndex];
            int maxRowIndex = 0;
            for(int rowIndex=0; rowIndex<23; rowIndex++) {
                if(rowIndex<column.rows.size()) {
                    printf(0, 8*rowIndex, RGB15(31,31,31), "%2d", rowIndex);
                    u16 rowColor = Sequencer::getInstance()->sequences[currentSequence].columns[columnIndex].index == rowIndex ? RGB15(31,31,31) : RGB15(20,20,20);
                    if(rowIndex == cursorRow && columnIndex == cursorCol) rowColor = RGB15(0,0,0);
                    printf(columnIndex*colPadding+24, rowIndex*8, rowColor, "%1c%02X ", Sequencer::KeyToChar(column.rows[rowIndex].key), column.rows[rowIndex].value);
                    maxRowIndex = std::max(maxRowIndex, rowIndex);
                }
            }
        }
    }
};

class ScopeView {
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

class ControlsView {
public:
    virtual void HandleInput(int keys, int held) {}
    virtual void Render() {
        glBoxFilled(0, 0,
                    SCREEN_WIDTH, SCREEN_HEIGHT,
                    RGB15(0,0,0));
        printf(8,8, RGB15(31,31,31), "A+DIR:   MODIFY VAL");
        printf(8,16,RGB15(31,31,31), "X+DIR:   MODIFY CMD");
        printf(8,24,RGB15(31,31,31), "R+DIR:   ROW/COL +/-");
        printf(8,32,RGB15(31,31,31), "L+DIR:   CHANGE SEQUENCE");
        printf(8,40,RGB15(31,31,31), "SEL+DIR: CHANGE PAGE");
        printf(8,48,RGB15(31,31,31), "START:   PLAY");
    }
};

class CommandView {
public:
    virtual void HandleInput(int keys, int held) {}
    virtual void Render() {
        glBoxFilled(0, 0,
                    SCREEN_WIDTH, SCREEN_HEIGHT,
                    RGB15(0,0,0));
        printf(8,8, RGB15(31,31,31), "N: PLAY NOTE");
        printf(8,16,RGB15(31,31,31), "E: ENVELOPE SPEED");
        printf(8,24,RGB15(31,31,31), "M: PHASE MODULATION AMOUNT");
        printf(8,32,RGB15(31,31,31), "F: PHASE MODULATION FREQUENCY");
        printf(8,40,RGB15(31,31,31), "T: TEMPO");
        printf(8,48,RGB15(31,31,31), "S: PLAY SEQUENCE");
    }
};

class TextTestView {
    virtual void HandleInput(int keys, int held) {}
    virtual void Render() {
        printf(0,0, RGB15(31,31,31), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        printf(0,16, RGB15(31,31,31), "abcdefghijklmnopqrstuvwxyz");
        printf(0,32, RGB15(31,31,31), "`1234567890-=");
        printf(0,48, RGB15(31,31,31), "~!@#$%^&*()_+");
        printf(0,64, RGB15(31,31,31), "[]\\;',./{}|:\"<>?");
        printf(0,72, RGB15(31,31,31), "A B C D E F G H I J K L M N O");
        printf(0,80, RGB15(31,31,31), "P Q R S T U V W X Y Z");
    }
};

#endif