#ifndef LD45_SOUNDS_H
#define LD45_SOUNDS_H

#include "Core/Main.h"
#include "Core/Types.h"
#include "Core/Time/Clock.h"

#include "Gfx/Gfx.h"

#include "IO/IO.h"
#include "LocalFS/LocalFileSystem.h"


#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

class SoundFX {
public:
	SoLoud::Soloud *soloud;

	Oryol::Buffer sfxData;
	SoLoud::Wav sfx;
	bool ready = false;

	void Load( SoLoud::Soloud *sol, Oryol::String resname);
	void Play();
};

class SoundMaker
{
public:
	void Init();
	void LoadAllSounds();

	SoundFX sfxWhoosh;
	SoundFX sfxClick;
	SoundFX sfxErrBloop;
	SoundFX sfxStomp;

	SoLoud::Soloud soloud;

	int musicPlaying = 0;
	Oryol::Buffer musicData;
	SoLoud::WavStream music;
};

#endif