#include <windows.h>

#include <math.h>

#include <vector>
#include <deque>

#include <GL/gl.h>
#include <GL/glu.h>

#include "focus.h"
#include "beat.h"

Beatronome::Beatronome()
{
	// init beat_ticks to 500 ticks (about 120 bpm)
	beat_ticks_left = 0;
	beat_ticks = 500;
	beat_led = 0.0f;

	beat_measure = 0.0f;

	beat_trigger = false;
	user_trigger = false;
	do_trigger = false;

	// half speed
	easy_mode = true;
}

void Beatronome::beat_sample( Uint32 ticks )
{		
	// force a beat
	do_trigger = true;
	user_trigger = true;

	Uint32 beat_sm = ticks - beatsm_last_tick;
	beatsm_last_tick = ticks;

	// if the delay is more than 3s (20 BPM)
	// assume they're starting and don't average the
	// sample
	if (beat_sm < 3000) 
	{
		beatsm_avg.push_back( beat_sm );

		if (beatsm_avg.size() > BEAT_AVG_COUNT) 
		{
			beatsm_avg.pop_front();
		}

		// average last few samples for bpm
		Uint32 beatTot = 0;
		for (size_t i=0; i < beatsm_avg.size(); i++) 
		{
			beatTot += beatsm_avg[i];
		}
		beat_ticks = beatTot / beatsm_avg.size();

		if (easy_mode)
		{			
			beat_ticks *= 4;
		}
		else
		{
			beat_ticks *= 2;
		}

		printf("BPM: %d\n", 60000 / beat_ticks );	
		beat_ticks_left = 0;	
	}

}

void Beatronome::beat_update( Uint32 ticks_elapsed )
{
	// Beat Timing			

	// Update the beat counter		
	beat_ticks_left += ticks_elapsed;
	while ( beat_ticks_left > beat_ticks ) 
	{
		beat_ticks_left -= beat_ticks;

		// a beat has occurred..
		do_trigger = true;		
	}		

	if (do_trigger) {
//		printf("beat...\n" );
		beat_count++;
		beat_led = 1.0f;
	}

	beat_pulse = (float)beat_ticks_left / (float)beat_ticks;
	beat_pulse2 = fabs((1.0f-beat_pulse) - 0.5f) * 2.0f;
	beat_pulse3 = smstep( beat_pulse2 );

	beat_measure = ((beat_count % 4) * 0.25f) +  (beat_pulse * 0.25f);
	
	beat_subcount = (int)(beat_pulse*4);

	beat_trigger = do_trigger;	

	// clear do_trigger and user_trigger
	do_trigger = false;
	user_trigger = false;
}

void Beatronome::beat_sim_update( )
{
	// clear beat trigger
	if (beat_trigger) beat_trigger = false;	

	// dim the beat_led (different than beat pulse, dims at a constant rate)
	if (beat_led > 0.0) {
		beat_led -= 0.2f;

		if (beat_led < 0.0) beat_led = 0.0;
	}


}

void Beatronome::redraw( float x0, float y0 )
{	
	// frame
	if (user_trigger) 
	{
		glColor3f( 1.0f, 1.0f, 0.0f );
	} 
	else 
	{
		glColor3f( 0.0f, beat_led * 0.3f, 0.0f );
	}

	glBegin( GL_QUADS );
	glVertex2i( x0, y0 + 25 );
	glVertex2i( x0 + 100, y0 + 25 );
	glVertex2i( x0 + 100, y0 );
	glVertex2i( x0, y0 );
	glEnd();

	// red indicator
	glColor3f( 0.6f, 0.0f, 0.0f );
	int xpos = (int)((1.0f-beat_pulse2) * 45.0f);

	glBegin( GL_QUADS );
	glVertex2i( x0+45+xpos, y0 + 25 );
	glVertex2i( x0+55+xpos, y0 + 25 );
	glVertex2i( x0+55+xpos, y0 );
	glVertex2i( x0+45+xpos, y0 );

	glVertex2i( (x0+45)-xpos, y0 + 25 );
	glVertex2i( (x0+55)-xpos, y0 + 25 );
	glVertex2i( (x0+55)-xpos, y0 );
	glVertex2i( (x0+45)-xpos, y0 );

	glEnd();

	
	glColor3f( 0.0f, ( beat_led*0.7f)+0.3f, 0.0f );
	
	glBegin( GL_QUADS );
	glVertex2i( (x0+40), y0+20 );
	glVertex2i( (x0+60), y0+20 );
	glVertex2i( (x0+60), y0+5 );
	glVertex2i( (x0+40), y0+5 );
	glEnd();
}