#ifndef FOCUS_BEAT_H
#define FOCUS_BEAT_H

#include <SDL.h>
#include <SDL_endian.h>

#include <vector>
#include <deque>

#define smstep(t)  (t*t * (3-2*t))

#define BEAT_AVG_COUNT (4)

class Beatronome
{
public:
	Beatronome();

	Uint32 beat_count;         // current beat count
	Uint32 beat_subcount;	   // 16th notes

	float beat_pulse;    // for things that "pulse" with the beat /|/|/|/|
	float beat_pulse2;    // Like beat_pulse but sawtooth /\/\/\/\/
	float beat_pulse3;    // like beat_pulse but smoother
	float beat_led;		  // like beat_pulse but with a constant time decay
	float beat_measure;   // like beat pulse but over 4 beats	

	bool beat_trigger;  // true (in sim step) if a beat was just triggers
	bool user_trigger;  // true (in sim step) if user triggered this beat
	bool do_trigger;

	Uint32 beat_ticks;
	Uint32 beat_ticks_left;	

	Uint32 beatsm_last_tick;
	std::deque<Uint32> beatsm_avg;

	bool easy_mode;

	// user triggered beat sample to set tempo
	void beat_sample( Uint32 ticks );

	void beat_update( Uint32 ticks_elapsed );	
	void beat_sim_update(); // fixed rate sim update

	// draw metronome graphic
	void redraw( float x, float y );
};

#endif