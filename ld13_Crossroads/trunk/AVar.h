#ifndef AVAR_H
#define AVAR_H

// An avar is an "animated varible". It acts like the underlying type
// in code, but also has a "displayed" value that can do things
// like ease-in, blend, etc..
// That way you can use the same variable for logic (where you want to 
// ignore the fancy animations) and for display (where you want to see it)
//
// The word comes from "AL" steve may's animation language

template <typename T>
class AVar
{
public:
	AVar( T val ) : m_value( val ), m_animValue( val )  {}

	operator T () const {  return m_value; }
	T animValue() const { return m_animValue; }	

	// todo more operators
	

protected:
	T m_value;     // The "actual" value this holds
	T m_animValue; // The animated value
};

typedef AVar<float> anim_float;
typedef AVar<double> anim_double;

#endif
