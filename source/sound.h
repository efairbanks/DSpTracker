#include <nds.h>
#include <stdio.h>
#include <maxmod9.h>

mm_word on_stream_request( mm_word length, mm_addr dest, mm_stream_formats format ) {
//----------------------------------------------------------------------------------
	s16 *target = (s16*)dest;
	
	int len = length;
	for( ; len; len-- )
	{
		int sample = rand()%2000 - 1000;
		
		*target++ = sample;
		*target++ = -sample;
	}
	
	return length;
}

class SoundEngine {
public:
  static SoundEngine * getInstance() {
    if(nullptr == instance) {
      instance = new SoundEngine();
    }
    return instance;
  };
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
        mystream.sampling_rate	= 25000;					// sampling rate = 25khz
        mystream.buffer_length	= 1200;						// buffer length = 1200 samples
        mystream.callback		= on_stream_request;		// set callback function
        mystream.format			= MM_STREAM_16BIT_STEREO;	// format = stereo 16-bit
        mystream.timer			= MM_TIMER0;				// use hardware timer 0
        mystream.manual			= false;					// don't use manual filling
        mmStreamOpen( &mystream );
  }
  ~SoundEngine() = default;
private:
  static SoundEngine * instance;
  SoundEngine() = default;
  SoundEngine(const SoundEngine&)= delete;
  SoundEngine& operator=(const SoundEngine&)= delete;
};
SoundEngine * SoundEngine::instance = nullptr;