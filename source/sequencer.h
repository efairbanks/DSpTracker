#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <nds.h>
#include <stdio.h>
#include <vector>

using namespace std;

/*
Sequencer
    Track[]
        Pattern[]
            Seq[]
                Row[]
    Table[]
    Instrument[]
    Config
*/

typedef enum {
    COLUMN_NOTE,
    COLUMN_EFFECT
} ColumnType;

class Row {
public:
    u8 key;
    u8 value;
    Row() {
        key = 0;
        value = 0;
    }
};

class Column {
public:
    ColumnType type;
    vector<Row> rows;
    int index;
    Column(ColumnType colType, u32 length) {
        type = colType;
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
        columns.push_back(Column(COLUMN_NOTE, 16));
        columns.push_back(Column(COLUMN_EFFECT, 16));
        columns.push_back(Column(COLUMN_EFFECT, 16));
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
private:
    static Sequencer * instance;
    Sequencer() = default;
    Sequencer(const Sequencer&)= delete;
    Sequencer& operator=(const Sequencer&)= delete;
};
Sequencer * Sequencer::instance = nullptr;

#endif