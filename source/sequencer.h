#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <nds.h>
#include <stdio.h>
#include <vector>

using namespace std;

class Sequencer {
public:
    vector<char> seq;
    int seqIndex = -1;
    static Sequencer * getInstance() {
        if(nullptr == instance) {
        instance = new Sequencer();
        }
        return instance;
    };
    void Init() {
        for(int i=0; i<8; i++) seq.push_back(0);
    }
    ~Sequencer() = default;
private:
    static Sequencer * instance;
    Sequencer() = default;
    Sequencer(const Sequencer&)= delete;
    Sequencer& operator=(const Sequencer&)= delete;
};
Sequencer * Sequencer::instance = nullptr;

#endif