#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <nds.h>
#include <stdio.h>
#include <vector>

#include "utils.h"
#include "synth.h"

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
            case 6:
                return 'S';
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
    int tick;
    u8 ticksPerStep = 16;
    int lastSubSequence = -1;
    Column(u32 length) {
        index = -1;
        tick = -1;
        for(int i=0; i<length; i++) rows.push_back(Row());
    }
    void Reset() {
        tick = -1;
        index = -1;
        lastSubSequence = -1;
    }
    Row& GetRow(int i=-1) {
        if(i<0) i = index;
        i = wrap(i, rows.size());
        return rows[i];
    }
    bool ProcessTick() {
        tick = wrap(tick + 1, ticksPerStep);
        if(tick == 0) {
            index = wrap(index + 1, rows.size());
            //rowsToProcess.push_back(rows[index]);
            return true;
        } else {
            return false;
        }
    }
};

class Sequence {
public:
    bool playing;
    vector<Column> columns;
    Sequence() {
        columns.push_back(Column(1));
        playing = false;
    }
    void Reset() {
        for(Column& column : columns) column.Reset();
    }
    bool ProcessTick() {
        bool columnIncremented = false;
        for(int i=0; i<columns.size(); i++) {
            columnIncremented |= columns[i].ProcessTick();
        }
        return columnIncremented;
    }
};

class Sequencer {
public:
    vector<Sequence> sequences;
    vector<Row> rows; // vector that accumulates rows that need to be processed
    int seqIndex = -1;
    static Sequencer * getInstance() {
        if(nullptr == instance) {
            instance = new Sequencer();
            for(int i=0; i<256; i++) instance->sequences.push_back(Sequence());
        }
        return instance;
    };
    ~Sequencer() = default;
    void Reset() {
        for(Sequence& sequence : sequences) {
            sequence.Reset();
        }
    }
    bool ProcessRow(Row& row, int sequenceIndex, int columnIndex, Synth& synth) {
        bool tickProcessed = false;
        if(row.key > 0) {
            tickProcessed = true;
            switch(Row::KeyToChar(row.key)) {
                case 'N':
                    synth.voices[0].PlayNote(NoteToFreq(row.value>>4, row.value & 0xF));
                    break;
                case 'E':
                    synth.voices[0].line.delta = (B32_1HZ_DELTA*8*row.value)>>4;
                    break;
                case 'F':
                    synth.voices[0].modFreqCoef = row.value;
                    break;
                case 'M':
                    synth.voices[0].modAmount = row.value;
                    break;
                case 'T':
                    synth.metro.delta = B32_1HZ_DELTA*(row.value+1);
                    break;
                case 'S':
                    if(sequences[seqIndex].columns[columnIndex].lastSubSequence >= 0) {
                        sequences[sequences[seqIndex].columns[columnIndex].lastSubSequence].playing = false;
                        sequences[sequences[seqIndex].columns[columnIndex].lastSubSequence].Reset();
                    }
                    sequences[row.value].Reset();
                    sequences[row.value].playing = true;
                    sequences[seqIndex].columns[columnIndex].lastSubSequence = row.value;
                    break;
            }
        }
        return tickProcessed;
    }
    bool ProcessTick(Synth &synth) {
        bool tickProcessed = false;
        int sequenceIndex = 0;
        for(Sequence& sequence : sequences) {
            if(sequence.playing) {
                if(sequence.ProcessTick()) {
                    int columnIndex = 0;
                    for(Column& column : sequence.columns) {
                        ProcessRow(column.GetRow(), sequenceIndex, columnIndex, synth);
                        columnIndex++;
                    }
                }
            }
            sequenceIndex++;
        }
        return tickProcessed;
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