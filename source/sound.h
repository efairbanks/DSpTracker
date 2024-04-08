#ifndef SOUND_ENGINE_H
#define SOUND_ENGING_H

#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>
#include <vector>

#include "sequencer.h"

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
    s16 Process(s16 phaseMod=0) {
        phase += delta;
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

class Metro {
public:
    u32 phase;
    u32 delta;
    u32 lastPhase;
    void Init() {
        phase = 0xFFFFFFFF;
        delta = B32_1HZ_DELTA * 4;
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
    s16 modAmount;
    s16 modFreqCoef;
    Voice() {
        line.Init();
        carrier.Init();
        modAmount = 0x10;
        modFreqCoef = 0x08;
    }
    void PlayNote(int freq) {
        line.Reset();
        modulator.Reset();
        modulator.SetFreq((freq*modFreqCoef)>>3);
        carrier.Reset();
        carrier.SetFreq(freq);
    }
    s16 Process() {
        return (carrier.Process((modulator.Process()*modAmount)>>3) * line.Process())>>12;
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

class SoundEngine {
public:
    static SoundEngine * getInstance() {
        if(nullptr == instance) {
            instance = new SoundEngine();
            // initialize maxmod without any soundbank (unusual setup)
            mm_ds_system sys;
            sys.mod_count 			= 0;
            sys.samp_count			= 0;
            sys.mem_bank			= 0;
            sys.fifo_channel		= FIFO_MAXMOD;
            mmInit( &sys );

            // open stream
            mm_stream mystream;
            mystream.sampling_rate	= SAMPLE_RATE;			    // sampling rate = 25khz
            mystream.buffer_length	= 1200;						// buffer length = 1200 samples
            mystream.callback		= on_stream_request;		// set callback function
            mystream.format			= MM_STREAM_16BIT_STEREO;	// format = stereo 16-bit
            mystream.timer			= MM_TIMER0;				// use hardware timer 0
            mystream.manual			= false;					// don't use manual filling
            mmStreamOpen( &mystream );
        }
        return instance;
    };
    Voice voices[3];
    Metro metro;
    Scope scope;
    s16 Process() {
        if(metro.Process()) {
            Sequencer* sequencer = Sequencer::getInstance();
            for(int i=0; i<sequencer->sequence.columns.size(); i++) {
                sequencer->sequence.columns[i].Increment();
                Row row = sequencer->sequence.columns[i].GetRow();
                if(row.key > 0) {
                    switch(row.key+64) {
                        case 'A':
                            voices[0].PlayNote(row.value*8);
                            break;
                        case 'B':
                            voices[0].line.delta = (B32_1HZ_DELTA*8*row.value)>>4;
                            break;
                        case 'C':
                            voices[0].modFreqCoef = row.value;
                            break;
                        case 'D':
                            voices[0].modAmount = row.value;
                            break;
                    }
                } 
            }
        }
        s16 out = 0;
        for(int i=0; i<3; i++) out += voices[i].Process();
        scope.Process(out);
        return out;
    }
    ~SoundEngine() = default;
private:
    static SoundEngine * instance;
    SoundEngine() = default;
    SoundEngine(const SoundEngine&)= delete;
    SoundEngine& operator=(const SoundEngine&)= delete;
    static mm_word on_stream_request( mm_word length, mm_addr dest, mm_stream_formats format ) {
	s16 *target = (s16*)dest;
	
	int len = length;
	for( ; len; len-- )
	{
		int sample = instance->Process();
		
		*target++ = sample;
		*target++ = sample;
	}
	
	return length;
}
};
SoundEngine * SoundEngine::instance = nullptr;

#endif