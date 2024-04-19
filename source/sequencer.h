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
                return 'm';
                break;
            case 5:
                return 'F';
                break;
            case 6:
                return 'f';
                break;
            case 7:
                return 'B';
                break;
            case 8:
                return 'b';
                break;
            case 9:
                return 'p';
                break;
            case 10:
                return 'c';
                break;
            case 11:
                return 'V';
                break;
            case 12:
                return 'S';
                break;
            case 13:
                return 'T';
                break;
            default:
                return '?';
                break;
        }
    }
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
    void Reset() {
        for(int i=0; i<columns.size(); i++) columns[i].Reset();
    }
};

class Sequencer {
public:
    bool playing = false;
    vector<Sequence> sequences;
    vector<Row> rows; // vector that accumulates rows that need to be processed
    void serialize(ofstream& stream) {
        int numSeqs = sequences.size();
        stream.write((char*)(&numSeqs), sizeof(numSeqs));
        for(int i=0; i<numSeqs; i++) {
            sequences[i].serialize(stream);
        }
    }
    void deserialize(ifstream& stream) {
        int numSeqs = sequences.size();
        stream.read((char*)(&numSeqs), sizeof(numSeqs));
        sequences.clear();
        for(int i=0; i<numSeqs; i++) {
            sequences.push_back(Sequence());
            sequences[i].deserialize(stream);
        }
    }
    static Sequencer * getInstance() {
        if(nullptr == instance) {
            instance = new Sequencer();
            for(int i=0; i<256; i++) instance->sequences.push_back(Sequence());
        }
        return instance;
    };
    ~Sequencer() = default;
    void Reset() {
        for(int i=0; i<sequences.size(); i++) {
            sequences[i].Reset();
        }
    }
    bool ProcessRow(Row& row, int sequenceIndex, int columnIndex, Synth& synth) {
        bool tickProcessed = false;
        if(row.key > 0) {
            tickProcessed = true;
            int voice = sequences[sequenceIndex].voice;
            switch(Row::KeyToChar(row.key)) {
                case 'N':
                    synth.voices[voice].PlayNote(NoteToFreq(row.value>>4, row.value & 0xF));
                    break;
                case 'E':
                    synth.voices[voice].line.delta = (B32_1HZ_DELTA*8*row.value)>>4;
                    break;
                case 'F':
                    synth.voices[voice].modFreqCoef = row.value;
                    break;
                case 'f':
                    synth.voices[voice].modFreqEnvCoef = (s8)row.value;
                    break;
                case 'M':
                    synth.voices[voice].modCoef = row.value;
                    break;
                case 'm':
                    synth.voices[voice].modEnvCoef = (s8)row.value;
                    break;
                case 'p':
                    synth.voices[voice].carFreqEnvCoef = (u8)row.value;
                    break;
                case 'T':
                    if(row.value > 0) {
                        synth.metro.delta = METRO_1BPM_DELTA*row.value;
                    }
                    break;
                case 'S':
                    if(sequences[sequenceIndex].columns[columnIndex].lastSubSequence >= 0) {
                        sequences[sequences[sequenceIndex].columns[columnIndex].lastSubSequence].playing = false;
                        sequences[sequences[sequenceIndex].columns[columnIndex].lastSubSequence].Reset();
                    }
                    sequences[row.value].Reset();
                    sequences[row.value].playing = true;
                    sequences[sequenceIndex].columns[columnIndex].lastSubSequence = row.value;
                    break;
                case 'V':
                    sequences[sequenceIndex].voice = wrap(row.value, synth.voices.size());
                    break;
                case 'B':
                    synth.voices[voice].feedbackCoef = row.value;
                    break;
                case 'b':
                    synth.voices[voice].feedbackEnvCoef = (s8)row.value;
                    break;
                case 'c':
                    synth.voices[voice].ampCurve = (row.value & 0xF0)>>4;
                    synth.voices[voice].modCurve = (row.value & 0x0F);
                    break;
                default:
                    break;
            }
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
                    if(column.ProcessTick()) {
                        ProcessRow(column.GetRow(), sequenceIndex, columnIndex, synth);
                    }
                }
            }
        }
        return tickProcessed;
    }
    int NoteToFreq(u8 octave, u8 note) {
        while(note>11) { note-=12; octave++; }
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