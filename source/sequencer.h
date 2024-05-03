#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <nds.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>

#include "utils.h"
#include "synth.h"

using namespace std;

u32 NOTE_FREQ_TABLE[12] = {4186, 4435, 4698, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902};

class Row {
public:
    u8 key;
    u8 value;
    void serialize(ofstream& stream) {
        stream.write((char*)(&key), sizeof(key));
        stream.write((char*)(&value), sizeof(value));
    }
    void deserialize(ifstream& stream) {
        stream.read((char*)(&key), sizeof(key));
        stream.read((char*)(&value), sizeof(value));
    }
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
    void serialize(ofstream& stream) {
        stream.write((char*)(&index), sizeof(index));
        stream.write((char*)(&tick), sizeof(tick));
        stream.write((char*)(&ticksPerStep), sizeof(ticksPerStep));
        stream.write((char*)(&lastSubSequence), sizeof(lastSubSequence));
        int numRows = rows.size();
        stream.write((char*)(&numRows), sizeof(numRows));
        for(int i=0; i<numRows; i++) {
            rows[i].serialize(stream);
        }
    }
    void deserialize(ifstream& stream) {
        stream.read((char*)(&index), sizeof(index));
        stream.read((char*)(&tick), sizeof(tick));
        stream.read((char*)(&ticksPerStep), sizeof(ticksPerStep));
        stream.read((char*)(&lastSubSequence), sizeof(lastSubSequence));
        int numRows = rows.size();
        stream.read((char*)(&numRows), sizeof(numRows));
        rows.clear();
        for(int i=0; i<numRows; i++) {
            rows.push_back(Row());
            rows[i].deserialize(stream);
        }
    }
    Column(u32 length=1) {
        index = -1;
        tick = -1;
        for(int i=0; i<length; i++) rows.push_back(Row());
    }
    void Reset(int indexOffset=-1) {
        tick = -1;
        index = wrap(indexOffset, rows.size());
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
            return true;
        } else {
            return false;
        }
    }
};

class Sequence {
public:
    bool playing;
    int voice;
    int subSequenceOffset = 0;
    vector<Column> columns;
    void serialize(ofstream& stream) {
        stream.write((char*)(&playing), sizeof(playing));
        stream.write((char*)(&voice), sizeof(voice));
        int numCols = columns.size();
        stream.write((char*)(&numCols), sizeof(numCols));
        for(int i=0; i<numCols; i++) {
            columns[i].serialize(stream);
        }
    }
    void deserialize(ifstream& stream) {
        stream.read((char*)(&playing), sizeof(playing));
        stream.read((char*)(&voice), sizeof(voice));
        int numCols = columns.size();
        stream.read((char*)(&numCols), sizeof(numCols));
        columns.clear();
        for(int i=0; i<numCols; i++) {
            columns.push_back(Column());
            columns[i].deserialize(stream);
        }
    }
    Sequence() {
        columns.push_back(Column(1));
        playing = false;
        voice = 0;
    }
    void Reset(int indexOffset=-1) {
        for(int i=0; i<columns.size(); i++) columns[i].Reset(indexOffset);
    }
};

class InstrumentTable {
public:
    vector<u8> values {0x40,0xc0,0xc0,0,0,0,0,0,0,0,0,0,0,0};
    void serialize(ofstream& stream) {
        int numVals = values.size();
        stream.write((char*)(&numVals), sizeof(numVals));
        for(int i=0; i<numVals; i++) {
            u8 value = values[i];
            stream.write((char*)(&value), sizeof(value));
        }
    }
    void deserialize(ifstream& stream) {
        int numVals = values.size();
        stream.read((char*)(&numVals), sizeof(numVals));
        values.clear();
        for(int i=0; i<numVals; i++) {
            u8 value;
            stream.read((char*)(&value), sizeof(value));
            values.push_back(value);
        }
    }
    InstrumentTable() {}
    ~InstrumentTable() {}
    u8 GetValue(u8 key) {
        if(key>=0 && key<values.size()) {
            return values[key];
        } else {
            return 0;
        }
    }
};

class Sequencer {
public:
    bool playing = false;
    vector<Sequence> sequences;
    vector<Row> rows; // vector that accumulates rows that need to be processed
    vector<InstrumentTable> tables;
    void serialize(ofstream& stream) {
        stream.write((char*)(&playing), sizeof(playing));
        int numSeqs = sequences.size();
        stream.write((char*)(&numSeqs), sizeof(numSeqs));
        for(int i=0; i<numSeqs; i++) {
            sequences[i].serialize(stream);
        }
        int numTables = tables.size();
        stream.write((char*)(&numTables), sizeof(numTables));
        for(int i=0; i<numTables; i++) {
            tables[i].serialize(stream);
        }
    }
    void deserialize(ifstream& stream) {
        stream.read((char*)(&playing), sizeof(playing));
        int numSeqs = sequences.size();
        stream.read((char*)(&numSeqs), sizeof(numSeqs));
        sequences.clear();
        for(int i=0; i<numSeqs; i++) {
            sequences.push_back(Sequence());
            sequences[i].deserialize(stream);
        }
        int numTables = tables.size();
        stream.read((char*)(&numTables), sizeof(numTables));
        tables.clear();
        for(int i=0; i<numTables; i++) {
            tables.push_back(InstrumentTable());
            tables[i].deserialize(stream);
        }
    }
    static Sequencer * getInstance() {
        if(nullptr == instance) {
            instance = new Sequencer();
            for(int i=0; i<256; i++) {
                instance->sequences.push_back(Sequence());
                instance->tables.push_back(InstrumentTable());
            }
        }
        return instance;
    };
    ~Sequencer() = default;
    void Reset() {
        for(int i=0; i<sequences.size(); i++) {
            sequences[i].Reset();
        }
    }
    char KeyToChar(int key) {
        key = wrap(key, commandChars.size()+1);
        if(key == 0) {
            return '-';
        } else {
            return commandChars[key-1];
        }
    }
    vector<char> commandChars {'N','A','E','e','M','m','F','f','B','b','p','c','R','V','J','S','s','T','I','i'};
    bool ProcessRow(Row& row, int sequenceIndex, int columnIndex, Synth& synth, bool triggerNote=true) {
        bool tickProcessed = false;
        if(row.key > 0) {
            Sequence& sequence = sequences[sequenceIndex];
            Column& column = sequence.columns[columnIndex];
            s32 deltaCoef = row.value;
            Voice& voice = synth.voices[sequence.voice];
            switch(KeyToChar(row.key)) {
                case 'I':
                case 'i':
                    for(int tableIndex=tables[row.value].values.size()-1; tableIndex>=0; tableIndex--) {
                        Row dummyRow;
                        dummyRow.key = tableIndex+1;
                        dummyRow.value = tables[row.value].values[tableIndex];
                        ProcessRow(dummyRow, sequenceIndex, columnIndex, synth, KeyToChar(row.key) == 'I');
                    }
                    break;
                case 'N':
                    if(triggerNote) {
                        voice.PlayNote(NoteToFreq(row.value>>4, row.value & 0xF));
                    } else {
                        voice.SetNote(NoteToFreq(row.value>>4, row.value & 0xF));
                    }
                    break;
                case 'A':
                    voice.amp = row.value;
                    break;
                case 'E':
                    deltaCoef = 0xFF-deltaCoef;
                    deltaCoef *= deltaCoef;
                    voice.line.fallingDelta = (B32_1HZ_DELTA>>8) * deltaCoef;
                    break;
                case 'e':
                    deltaCoef = 0xFF-deltaCoef;
                    deltaCoef *= deltaCoef;
                    voice.line.risingDelta = (B32_1HZ_DELTA>>8) * deltaCoef;
                    break;
                case 'M':
                    voice.modCoef = row.value;
                    break;
                case 'm':
                    voice.modEnvCoef = (s8)row.value;
                    break;
                case 'F':
                    voice.modFreqCoef = row.value;
                    break;
                case 'f':
                    voice.modFreqEnvCoef = (s8)row.value;
                    break;
                case 'B':
                    voice.feedbackCoef = row.value;
                    break;
                case 'b':
                    voice.feedbackEnvCoef = (s8)row.value;
                    break;
                case 'p':
                    voice.carFreqEnvCoef = (u8)row.value;
                    break;
                case 'c':
                    voice.ampCurve = (row.value & 0xF0)>>4;
                    voice.modCurve = (row.value & 0x0F);
                    break;
                case 'R':
                    voice.verbAmp = (u8)row.value;
                    break;
                case 'V':
                    sequence.voice = wrap(row.value, synth.voices.size());
                    break;
                case 'J':
                    column.index = row.value;
                    if(KeyToChar(column.rows[column.index].key) != 'J')
                    ProcessRow(
                        column.rows[column.index],
                        sequenceIndex,
                        columnIndex,
                        synth
                    );
                    break;
                case 'S':
                    if(column.lastSubSequence >= 0) {
                        sequences[column.lastSubSequence].playing = false;
                        sequences[column.lastSubSequence].Reset();
                    }
                    sequences[row.value].Reset(sequence.subSequenceOffset-1);
                    sequences[row.value].playing = true;
                    column.lastSubSequence = row.value;
                    break;
                case 's':
                    sequence.subSequenceOffset = row.value;
                    break;
                case 'T':
                    if(row.value > 0) {
                        synth.metro.delta = METRO_1BPM_DELTA*row.value;
                    }
                    break;
                default:
                    break;
            }
            tickProcessed = true;
        }
        return tickProcessed;
    }
    bool ProcessTick(Synth &synth) {
        bool tickProcessed = false;
        int sequenceIndex = 0;
        for(int sequenceIndex=0; sequenceIndex<sequences.size(); sequenceIndex++) {
            Sequence& sequence = sequences[sequenceIndex];
            if(sequence.playing) {
                for(int columnIndex=0; columnIndex<sequence.columns.size(); columnIndex++) {
                    Column& column = sequence.columns[columnIndex];
                    if(column.ticksPerStep > 0) {
                        if(column.ProcessTick()) {
                            ProcessRow(column.GetRow(), sequenceIndex, columnIndex, synth);
                        }
                    } else {
                        for(int rowIndex=0; rowIndex<column.rows.size(); rowIndex++) {
                            ProcessRow(column.rows[rowIndex], sequenceIndex, columnIndex, synth);
                        }
                    }
                }
            }
        }
        return tickProcessed;
    }
    int NoteToFreq(u8 octave, u8 note) {
        int oct = octave-8;
        while(note>11) { note-=12; oct++; }
        if(oct >= 0) {
            return NOTE_FREQ_TABLE[wrap(note, 12)]<<abs(oct);
        } else {
            return NOTE_FREQ_TABLE[wrap(note, 12)]>>abs(oct);
        }
    }
private:
    static Sequencer * instance;
    Sequencer() = default;
    Sequencer(const Sequencer&)= delete;
    Sequencer& operator=(const Sequencer&)= delete;
};
Sequencer * Sequencer::instance = nullptr;

#endif