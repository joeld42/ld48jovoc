#include "Sounds.h"

#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

#include "glm/gtc/random.hpp"

#include "GameSettings.h"

using namespace Oryol;

void SoundSet::LoadSound( SoLoud::Bus *bus, Oryol::String resname )
{
    SoundFX *sfx = Memory::New<SoundFX>();
    sfx->Load( bus, resname );
    sounds.Add( sfx );
}

void SoundSet::PlayAny()
{
    if (sounds.Size()>0 ) {
        int ndx = glm::linearRand( 0, sounds.Size()-1 );
        sounds[ndx]->Play();
    }
}

void SoundFX::Load(SoLoud::Bus* bus, Oryol::String resname)
{
    busSfx = bus;
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
		busSfx->play(sfx);
	}
}


void SoundMaker::Init( GameSettings *_settings )
{
    settings = _settings;
    
	// Initialize SoLoud (automatic back-end selection)
	soloud.init();
}

void SoundMaker::LoadAllSounds()
{
    if (settings->sfxEnabled)
    {
        busSfx.setVolume( settings->sfxVolume );
        printf("Sfx volume %f\n", settings->sfxVolume);
        soloud.play(busSfx);
    }
    
	sfxYoink.Load(&busSfx, "gamedata:yoink.wav");
	sfxHuh.Load(&busSfx, "gamedata:huh.wav");
	sfxFwoosh.Load(&busSfx, "gamedata:fwoosh.wav");
	sfxFireOut.Load(&busSfx, "gamedata:fireout.wav");
#if 0
    sfxFarm.LoadSound(&busSfx, "sfx:farm_1.wav");
    sfxFarm.LoadSound(&busSfx, "sfx:farm_2.wav");
#endif        
    	
    IO::Load("gamedata:home_fires.ogg", [this](IO::LoadResult loadResult) {
        Log::Info("Music loaded, playing\n");
		this->musicData = std::move(loadResult.Data);
		music.loadMem(musicData.Data(), musicData.Size(), true, false);
		music.setLooping(true);

            if (settings->musicEnabled)
            {
                busMusic.setVolume( settings->musicVolume );
                soloud.play(busMusic);  // TODO: alwyas start the music bus?
                
                hmusic = soloud.play( music );
                musicPlaying = 1;
            }
		});

}
