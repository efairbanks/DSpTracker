#ifndef SOUND_ENGINE_H
#define SOUND_ENGING_H

#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>
#include <vector>

#include "sequencer.h"

class SoundEngine {
public:
    static SoundEngine * getInstance() {
        if(nullptr == instance) {
            instance = new SoundEngine();
            instance->allpass = new Allpass(400, -4020);
            instance->lfo.SetFreq(B32_1HZ_DELTA>>8);

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
            mystream.manual			= true;					// don't use manual filling
            mmStreamOpen( &mystream );
        }
        return instance;
    };
    Voice voices[3];
    Metro metro;
    Scope scope;
    Allpass* allpass;
    BrownNoise noise;
    Lowpass lowpass;
    SinOsc lfo;
    s16 Process() {
        if(metro.Process()) {
            Sequencer* sequencer = Sequencer::getInstance();
            for(int i=0; i<sequencer->sequence.columns.size(); i++) {
                sequencer->sequence.columns[i].Increment();
                Row row = sequencer->sequence.columns[i].GetRow();
                if(row.key > 0) {
                    switch(Row::KeyToChar(row.key)) {
                        case 'N':
                            voices[0].PlayNote(Sequencer::getInstance()->NoteToFreq(row.value>>4, row.value & 0xF));
                            break;
                        case 'E':
                            voices[0].line.delta = (B32_1HZ_DELTA*8*row.value)>>4;
                            break;
                        case 'F':
                            voices[0].modFreqCoef = row.value;
                            break;
                        case 'M':
                            voices[0].modAmount = row.value;
                            break;
                        case 'T':
                            metro.delta = B32_1HZ_DELTA*(row.value+1);
                            break;
                    }
                } 
            }
        }
        s16 out = 0;
        for(int i=0; i<3; i++) out += voices[i].Process();
        s16 nse = noise.Process();
        s16 verb = allpass->Process(out>>2, 0);
        out += lowpass.Process(verb);
        scope.Process(out);
        return out;
    }
    void RenderAudio() {
        mmStreamUpdate();
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