#ifndef SYNTH_H
#define SYNTH_H

#include <nds.h>
#include <stdio.h>
#include <vector>

using namespace std;

#define SAMPLE_RATE 25000
#define B32_1HZ_DELTA ((0xFFFFFFFF)/SAMPLE_RATE)

class SawOsc {
public:
    u32 phase;
    u32 delta;
    void Init() {
        phase = 0;
        delta = B32_1HZ_DELTA * 110;
    }
    SawOsc() {
        Init();
    }
    void Reset() { phase = 0; }
    void SetFreq(u32 freq) {
        delta = B32_1HZ_DELTA * freq;
    }
    s16 Process() {
        phase += delta;
        s16 out = ((s16)(phase>>17)) - (1<<14);
        return out>>2;
    }
};

class SinOsc {
public:
    u32 phase;
    u32 delta;
    void Init() {
        phase = 0;
        delta = B32_1HZ_DELTA * 220;
    }
    SinOsc() {
        Init();
    }
    void Reset() { phase = 0; }
    void SetFreq(u32 freq) {
        delta = B32_1HZ_DELTA * freq;
    }
    s16 Process(s16 phaseMod=0, s16 deltaMod=0) {
        phase += delta + deltaMod;
        s16 out = sinLerp(((s16)(phase>>17)) + phaseMod);
        return out;
    }
};

class Line {
public:
    u32 phase;
    u32 delta;
    u32 lastPhase;
    bool firing;
    void Init() {
        phase = 0xFFFFFFFF;
        delta = B32_1HZ_DELTA * 2;
        lastPhase = 0xFFFFFFFF;
        firing = false;
    }
    Line() {
        Init();
    }
    void Reset() {
        phase = 0xFFFFFFFF;
        lastPhase = 0xFFFFFFFF;
        firing = true;
    }
    s16 Process() {
        s16 out = 0;
        if(phase > lastPhase) firing = false;
        if(firing) {
            out = phase>>(32-12);
            lastPhase = phase;
            phase -= delta;
        }
        return (out*out)>>12;
    }
};

#define METRO_1BPM_DELTA ((B32_1HZ_DELTA*64)/60)
class Metro {
public:
    u32 phase;
    u32 delta;
    u32 lastPhase;
    void Init() {
        phase = 0xFFFFFFFF;
        delta = METRO_1BPM_DELTA * 120;
        lastPhase = 0;
    }
    Metro() {
        Init();
    }
    void Reset() {
        phase = 0xFFFFFFFF;
        lastPhase = 0;
    }
    s16 Process() {
        s16 out = 0;
        if(phase > lastPhase) out = 1<<12;
        lastPhase = phase;
        phase -= delta;
        return out;
    }
};

class Voice {
public:
    Line line;
    SinOsc modulator;
    SinOsc carrier;
    s16 modCoef;
    s16 modFreqCoef;
    s16 feedbackCoef;
    s16 modEnvCoef;
    s16 modFreqEnvCoef;
    s16 feedbackEnvCoef;
    s16 lastVal;
    Voice() {
        line.Init();
        carrier.Init();
        modCoef = 0x10;
        modFreqCoef = 0x08;
        feedbackCoef = 0x00;
        modEnvCoef = 0x00;
        modFreqEnvCoef = 0x00;
        feedbackEnvCoef = 0x00;
        lastVal = 0;
    }
    void PlayNote(int freq) {
        line.Reset();
        modulator.Reset();
        modulator.SetFreq((freq*modFreqCoef)>>3);
        carrier.Reset();
        carrier.SetFreq(freq);
    }
    s16 Process() {
        int out = 0;
        if(line.firing) {
            s16 m = modulator.Process(((lastVal*feedbackCoef)>>4) + ((lastVal*feedbackEnvCoef)>>4));
            s16 env = line.Process();
            out = carrier.Process(((m*modCoef)>>4) + ((((env*modEnvCoef)>>4)*m)>>12), (env*modFreqCoef)>>4);
            lastVal = out;
            out = (out * env)>>12;
        } else {
            lastVal = 0;
        }
        return out;
    }
};

class Scope {
    u16 index;
public:
    s16* buffer;
    s16* backBuffer;
    u16 length;
    u16 superSample;
    u16 superSampleIndex;
    Scope() {
        index = 0;
        length = 128;
        superSample = 9;
        superSampleIndex = 0;
        buffer = (s16*)malloc(sizeof(u16) * length);
        backBuffer = (s16*)malloc(sizeof(u16) * length);
        for(int i=0; i<length; i++) {
            buffer[i] = 0;
            backBuffer[i] = 0;
        }
    }
    ~Scope() { free(buffer); }
    bool IsReady() { return index == length; }
    void Reset() { index = 0; }
    void Process(s16 in) {
        if(index == 0) {
            s16* temp = backBuffer;
            backBuffer = buffer;
            buffer = temp;
        }
        if(index < length) {
            if(superSampleIndex == 0) {
                backBuffer[index] = in;
            } else {
                backBuffer[index] = (in>>1) + (backBuffer[index]>>1);
            }
            if(superSampleIndex == 0) index = index + 1;
            superSampleIndex = (superSampleIndex+1) % superSample;
        }
    }
};

class Comb {
public:
    s16* buffer;
    s16 coef;
    int length;
    int index;
    Comb(int len=10, s16 c=-4000) {
        length = len;
        buffer = (s16*)malloc(sizeof(s16)*length);
        coef = c;
        index = 0;
    }
    ~Comb() { free(buffer); }
    s16 Process(s16 in, s16 phase=0) {
        int i = wrap(index+1, length);
        s16 out = in + ((buffer[i]*coef)>>12);
        buffer[wrap(i+phase, length)] = out;
        index=wrap(index+1, length);
        return out;
    }
};

class Allpass {
public:
    Comb* a;
    Comb* b;
    Allpass(int len=10, s16 c=-4000) {
        a = new Comb(len, c);
        b = new Comb(len, -c);
    }
    //~Allpass() { destroy a; destroy b; }
    s16 Process(s16 in, s16 phase=0) {
        return a->Process(b->Process(in, phase), phase);
    }
};

class BrownNoise {
    s32 phase;
    s16 delta;
public:
    BrownNoise(s32 d=11) {
        phase = 0;
        delta = d;
    }
    s16 Process() {
        phase += rand()&0x1 ? delta : -delta;
        if(phase >= 0x7FFF) phase -= delta;
        if(phase <= -0x7FFF) phase += delta;
        return phase;
    }
};

class Lowpass {
public:
    s32 last;
    Lowpass() {}
    s16 Process(s32 in, s16 coef=16) {
        in = in<<8;
        last = ((in*coef)>>8) + (last*((1<<8)-coef)>>8);
        return last>>8;
    }
};

class Synth {
public:
    vector<Voice> voices;
    Metro metro;
    Synth() {
        for(int i=0; i<8; i++) voices.push_back(Voice());
    }
    bool GetTick() {
        return metro.Process();
    }
    void PlayNote(int freq, int voice=0) {
        voices[voice].PlayNote(freq);
    }
    s16 Process() {
        s16 out = 0;
        for(int i=0; i<voices.size(); i++) out += voices[i].Process();
        return out;
    }
};

#endif