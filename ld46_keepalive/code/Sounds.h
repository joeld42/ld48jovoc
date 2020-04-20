#ifndef CIVCLICKER_SOUNDS_H
#define CIVCLICKER_SOUNDS_H

#include "Core/Main.h"
#include "Core/Types.h"
#include "Core/Time/Clock.h"

#include "Gfx/Gfx.h"

#include "IO/IO.h"
#include "LocalFS/LocalFileSystem.h"


#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

class GameSettings;

class SoundFX {
public:
    SoLoud::Bus *busSfx;

	Oryol::Buffer sfxData;
	SoLoud::Wav sfx;
	bool ready = false;

	void Load( SoLoud::Bus *bus, Oryol::String resname);
	void Play();
};

class SoundSet {
public:
    Oryol::Array<SoundFX*> sounds;
    void LoadSound( SoLoud::Bus *bus, Oryol::String resname );
    void PlayAny();    
};

class SoundMaker
{
public:
	void Init( GameSettings *settings );
	void LoadAllSounds();

	SoundFX sfxHuh;
	SoundFX sfxYoink;
	SoundFX sfxFwoosh;
	SoundFX sfxFireOut;
	
    
    SoundSet sfxFarm;
    SoundSet sfxExplore;

	SoLoud::Soloud soloud;

	int musicPlaying = 0;
    int hmusic;
	Oryol::Buffer musicData;
	SoLoud::WavStream music;
    
    SoLoud::Bus busMusic;
    SoLoud::Bus busSfx;
    
    GameSettings *settings;
};

#endif
