#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <nds.h>
#include <stdio.h>
#include <vector>

#include "utils.h"

using namespace std;

u32 NOTE_FREQ_TABLE[12] = {4186, 4435, 4698, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902};

class Row {
public:
    static char KeyToChar(int key) {
        switch(key) {
            case 0:
                return '-';
                break;
            case 1:
                return 'N';
                break;
            case 2:
                return 'E';
                break;
            case 3:
                return 'M';
                break;
            case 4:
                return 'F';
                break;
            case 5:
                return 'T';
                break;
            default:
                return '?';
                break;
        }
    }
    u8 key;
    u8 value;
    Row() {
        key = 0;
        value = 0;
    }
};

class Column {
public:
    vector<Row> rows;
    int index;
    Column(u32 length) {
        index = -1;
        for(int i=0; i<length; i++) rows.push_back(Row());
    }
    Row GetRow(int i=-1) {
        if(i<0) i = index;
        while(i < 0) i += rows.size();
        i %= rows.size();
        return rows[i];
    }
    void Increment() {
        index = (index + 1) % rows.size();
    };
};

class Sequence {
public:
    vector<Column> columns;
    Sequence() {
        columns.push_back(Column(1));
        columns.push_back(Column(1));
        columns.push_back(Column(1));
    }
};

class Sequencer {
public:
    Sequence sequence;
    int seqIndex = -1;
    static Sequencer * getInstance() {
        if(nullptr == instance) {
            instance = new Sequencer();
        }
        return instance;
    };
    ~Sequencer() = default;
    void ProcessRow() {
        sequence.columns[0].Increment();
        Row row = sequence.columns[0].GetRow();
    }
    int NoteToFreq(u8 octave, u8 note) {
        while(note>11) { note--; octave++; }
        return NOTE_FREQ_TABLE[wrap(note, 12)]>>(8-octave);
    }
private:
    static Sequencer * instance;
    Sequencer() = default;
    Sequencer(const Sequencer&)= delete;
    Sequencer& operator=(const Sequencer&)= delete;
};
Sequencer * Sequencer::instance = nullptr;

#endif