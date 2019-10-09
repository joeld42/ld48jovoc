#include "Sounds.h"

#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

using namespace Oryol;

void SoundFX::Load(SoLoud::Soloud* sol, Oryol::String resname)
{
	soloud = sol;
	IO::Load( resname, [this, resname](IO::LoadResult loadResult) {
		this->sfxData = std::move(loadResult.Data);
		sfx.loadMem(sfxData.Data(), sfxData.Size(), true, false);
		Log::Info("SFX %s loaded...\n", resname.AsCStr() );
		ready = true;
		});

}

void SoundFX::Play()
{
	if (ready) {
		soloud->play(sfx);
	}
}


void SoundMaker::Init()
{
	// Initialize SoLoud (automatic back-end selection)
	soloud.init();
}

void SoundMaker::LoadAllSounds()
{

	sfxClick.Load( &soloud, "gamedata:sfx/UI_Click_Distinct_mono.wav");
	sfxWhoosh.Load( &soloud, "gamedata:sfx/WHOOSH_Air_Slow_RR9_mono.wav" );
	sfxErrBloop.Load(&soloud, "gamedata:sfx/UI_Error_Double_Note_Down_Muffled_Short_stereo.wav");
	sfxStomp.Load(&soloud, "gamedata:sfx/AUDIENCE_Stomp_Stomp_Clap_loop_stereo.wav");

	IO::Load("gamedata:sfx/StringsAssemble-full.ogg", [this](IO::LoadResult loadResult) {
		this->musicData = std::move(loadResult.Data);
		music.loadMem(musicData.Data(), musicData.Size(), true, false);
		music.setLooping(true);

		int hmusic = soloud.play( music );
		soloud.setVolume(hmusic, 0.3f);

		musicPlaying = 1;
		});

}