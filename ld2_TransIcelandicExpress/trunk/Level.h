#ifndef TIE_LEVEL_H
#define TIE_LEVEL_H

#include "TIE.h"

class LevelInfo {
public:
	char *name;
	char *desc;
	int final;
	VictoryType victory;
};

#endif