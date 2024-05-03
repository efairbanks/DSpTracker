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
    Scope scope;
    Synth synth;
    s16 Process() {
        if(synth.GetTick()) {
            Sequencer* sequencer = Sequencer::getInstance();
            sequencer->ProcessTick(synth);
        }
        s16 out = synth.Process();
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
		s16 sample = instance->Process();
		
		*target++ = sample;
		*target++ = sample;
	}
	
	return length;
}
};
SoundEngine * SoundEngine::instance = nullptr;

#endif