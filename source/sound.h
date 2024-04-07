#ifndef SOUND_ENGINE_H
#define SOUND_ENGING_H

#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>

#include "sequencer.h"

#define SAMPLE_RATE 25000
#define B32_1HZ_DELTA ((0xFFFFFFFF)/SAMPLE_RATE)

class SawOsc {
    u32 phase;
    u32 delta;
public:
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
    u32 phase;
    u32 delta;
public:
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
    s16 Process() {
        phase += delta;
        s16 out = sinLerp(((s16)(phase>>17)));
        return out;
    }
};

class Line {
    u32 phase;
    u32 delta;
    u32 lastPhase;
    bool firing;
public:
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
    u32 phase;
    u32 delta;
    u32 lastPhase;
public:
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
Line line;
SinOsc osc;
public:
    Voice() {
        line.Init();
        osc.Init();
    }
    void PlayNote(int freq) {
        line.Reset();
        osc.Reset();
        osc.SetFreq(freq);
    }
    s16 Process() {
        return (osc.Process() * line.Process())>>12;
    }
};

class SoundEngine {
public:
    static SoundEngine * getInstance() {
        if(nullptr == instance) {
            instance = new SoundEngine();
        }
        return instance;
    };
    Voice voice;
    Metro metro;
    void Init() {
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
    s16 Process() {
        if(metro.Process()) {
            Sequencer::getInstance()->seqIndex = (Sequencer::getInstance()->seqIndex + 1) % Sequencer::getInstance()->seq.size();
            int index = Sequencer::getInstance()->seqIndex;
            if(index >= 0) {
                int freq = Sequencer::getInstance()->seq[index];
                if(freq > 0) voice.PlayNote(freq*4);
            }
        }
        return voice.Process();
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