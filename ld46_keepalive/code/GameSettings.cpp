//
//  GameSettings.cpp
//  civclicker
//
//  Created by Joel Davis on 11/6/19.
//
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#include <Core/Log.h>

#include "GameSettings.h"

#include "ini.h"

#ifdef WIN32
#define strcasecmp stricmp
#endif

using namespace Oryol;

static int ini_configline_handler(void* user, const char* section, const char* name, const char* value);

GameSettings GameSettings::DefaultLowSpecSettings()
{
    GameSettings settings = GameSettings();
    settings.renderPassMultisample = 1;
    settings.useSimpleBackground = true;
    
    return settings;
}

GameSettings GameSettings::DefaultHighSpecSettings()
{
    GameSettings settings = GameSettings();
    
    settings.renderPassMultisample = 4;
    settings.useSimpleBackground = false;
    
    return settings;
}

GameSettings GameSettings::FromConfigFile( const char *iniFilename )
{
    GameSettings settings = GameSettings();
    return GameSettings::FromConfigFileWithDefaults( settings, iniFilename );
}

GameSettings GameSettings::FromConfigFileWithDefaults( GameSettings defs, const char *iniFilename )
{
    GameSettings settings = defs;
    
    // TODO: On iOS, choose defaults from a predetermined list
    // based on device.
#ifndef ORYOL_IOS
    if (ini_parse( iniFilename, ini_configline_handler, &settings )) {
        Log::Error("Error loading INI File!" );
    }
#endif
    
//    int test1 = ParseConfigInt( "zz42", 0, 100, 99 );
//    Log::Info("Test ParseConfigInt: test %d\n", test1 );
    
    return settings;
}

void GameSettings::WriteConfigFile( const char *filename )
{
    FILE *fp = fopen( filename, "wt");
    if (!fp) {
        Log::Error( "Couldn't write settings file: %s\n", filename );
        return;
    }
    
    fprintf( fp, "[Graphics]\n");
    fprintf( fp, "MultisampleCount = %d\n", renderPassMultisample );
    fprintf( fp, "UseSimpleBackground = %s\n", useSimpleBackground?"true":"false" );
    
    fprintf( fp, "MusicEnabled = %s\n", musicEnabled?"true":"false" );
    fprintf( fp, "MusicVolume = %3.2f\n", musicVolume );

    fprintf( fp, "SfxEnabled = %s\n", sfxEnabled?"true":"false" );
    fprintf( fp, "SfxVolume = %3.2f\n", sfxVolume );
    
    fclose(fp );
}

static int ini_configline_handler(void* userdata, const char* section, const char* name, const char* value)
{
    GameSettings *settings = (GameSettings*)userdata;
    
    Log::Info("Ini Configfile handler: sectin %s, name %s, value %s\n",
              section, name, value );
    
    // Graphics options
    if (!strcmp( section, "Graphics")) {
        if ( !strcasecmp( name, "MultisampleCount" ) ) {
            settings->renderPassMultisample = ParseConfigInt( value, 1, 8, settings->renderPassMultisample );
        }
        else if ( !strcasecmp( name, "UseSimpleBackground" ) ) {
            settings->useSimpleBackground = ParseConfigBool( value, settings->useSimpleBackground );
        }
        //else if ( !strcasecmp( name, "TileSubdivLevel" ) ) {
        //    settings->tileSubdivLevel = ParseConfigInt( value, 1, 5, settings->tileSubdivLevel );
        //}
 
    }
    // Audio Options
    else if (!strcmp( section, "Audio")) {
        
        if ( !strcasecmp( name, "MusicEnabled" ) ) {
            settings->musicEnabled = ParseConfigBool( value, settings->musicEnabled );
        } else if ( !strcasecmp( name, "MusicVolume" ) ) {
            settings->musicVolume = ParseConfigFloat( value, 0.0f, 1.0f, settings->musicVolume );
        } else if ( !strcasecmp( name, "SfxEnabled" ) ) {
            settings->sfxEnabled = ParseConfigBool( value, settings->sfxEnabled );
        } else if ( !strcasecmp( name, "SfxVolume" ) ) {
            settings->sfxVolume = ParseConfigFloat( value, 0.0f, 1.0f, settings->sfxVolume );
            printf("SFX Volume is %f\n", settings->sfxVolume );
        }
    }
    
    // 1 means we handled the value
    return 1;
}

int ParseConfigInt( const char *strval, int expectedMin, int expectedMax, int defaultValue )
{
    int result = defaultValue;
    
    char *endptr = NULL;
    int parsed = strtol( strval, &endptr, 10 );
    if ((endptr) && (*endptr=='\0')) {
        if ((parsed >= expectedMin) && (parsed <= expectedMax)) {
            result = parsed;
        } else {
            Log::Error("Parse error, value %d out of range %d..%d\n", parsed, expectedMin, expectedMax );
        }
    } else {
        Log::Error("Parse error, expected int but got %s\n", strval );
    }
    return result;
}

float ParseConfigFloat( const char *strval, float expectedMin, float expectedMax, float defaultValue )
{
    float result = defaultValue;
    
    char *endptr = NULL;
    float parsed = strtof( strval, &endptr );
    if ((endptr) && (*endptr=='\0')) {
        if ((parsed >= expectedMin) && (parsed <= expectedMax)) {
            result = parsed;
        } else {
            Log::Error("Parse error, value %3.2f out of range %3.2f..%3.2f\n", parsed, expectedMin, expectedMax );
        }
    } else {
        Log::Error("Parse error, expected int but got %s\n", strval );
    }
    return result;
    
}

static const char *trueStrings[] = { "1", "true", "t", "yes" };
static const char *falseStrings[] = { "0", "false", "f", "no" };

bool ParseConfigBool( const char *strval, int defaultValue )
{
    for (int i=0; i <  (int)(sizeof(trueStrings)/sizeof(trueStrings[0])); i++) {
        if (!strcasecmp( trueStrings[i], strval)) {
            return true;
        }
    }
    
    for (int i=0; i <  (int)(sizeof(falseStrings)/sizeof(falseStrings[0])); i++) {
        if (!strcasecmp( falseStrings[i], strval)) {
            return false;
        }
    }
    
    return defaultValue;
}

