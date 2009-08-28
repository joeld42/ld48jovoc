#ifndef TWEAKVAL_H
#define TWEAKVAL_H

// Note: gcc supports __COUNTER__ in 4.3+
#ifndef NDEBUG
#  define _TV(Val) TweakValue( __FILE__, __LINE__, __COUNTER__, Constant )
#else
#  define _TV(Val) Val
#endif



#endif
