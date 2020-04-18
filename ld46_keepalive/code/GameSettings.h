//
//  GameSettings.hpp
//  civclicker
//
//  Created by Joel Davis on 11/6/19.
//

#ifndef TK_GAMESETTINGS_H
#define TK_GAMESETTINGS_H

#include <stdio.h>


struct GameSettings
{
    bool useSimpleBackground = true; // TODO: look at this
    int renderPassMultisample = 1;
       
    // Sound settings
    float musicVolume = 0.25f;
    float sfxVolume = 1.0f;
    bool musicEnabled = true;
    bool sfxEnabled = true;
    
    void WriteConfigFile( const char *filename );
    
    static GameSettings DefaultLowSpecSettings();
    static GameSettings DefaultHighSpecSettings();
    
    static GameSettings FromConfigFile( const char *iniFilename );
    static GameSettings FromConfigFileWithDefaults( GameSettings defs, const char *iniFilename );
};

// Helper parsing functions, can be used elsewhere
int ParseConfigInt( const char *strval, int expectedMin, int expectedMax, int defaultValue );
float ParseConfigFloat( const char *strval, float expectedMin, float expectedMax, float defaultValue );
bool ParseConfigBool( const char *strval, int defaultValue );

#endif // TK_GAMESETTINGS_H
