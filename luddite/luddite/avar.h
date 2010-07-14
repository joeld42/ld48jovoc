#ifndef AVAR_H
#define AVAR_H

// An avar is an "animated varible". It acts like the underlying type
// in code, but also has a "displayed" value that can do things
// like ease-in, blend, etc..
// That way you can use the same variable for logic (where you want to 
// ignore the fancy animations) and for display (where you want to see it)
//
// The word comes from "AL" steve may's animation language

#include <cmath>
#include <list>

template <typename T>
class Avar
{
public:

    // These are glommed together because there are often cases where 
    // you want an avar to do more than one behavior, for example a
    // menu where the selected item pulses slightly, but when you move
    // up or down, it fades from the selected color
    enum AvarBehavior 
    {
        Avar_STEADY,     // Don't animate, wait for further orders
        Avar_INTERP,     // Smoothly transition to new values
        Avar_PULSE,      // Pulse
    };

    // Sets Interpolation type
    enum InterpType
    {
        Interp_LINEAR,
        Interp_SMOOTHSTEP,
    };
    

    Avar( T val = T() ) : 
        m_behavior( Avar_STEADY ),
        m_value( val ), 
        m_animValue( val ),
        m_interpType( Interp_LINEAR ),
        m_frequency( 1.0 ),
        m_minVal( 0.0 ),
        m_maxVal( 1.0 )
    {
        _avarList.push_back( this );        
    }

    ~Avar()
    {
        _avarList.remove( this );        
    }
    
    
    // Access value .. todo more
	operator T () const {  return m_value; }
	T animValue() const { return m_animValue; }	

    // Interp from current to target
    void animate( const T &targVal, float totalTime=1.0, InterpType interpType=Interp_SMOOTHSTEP )
    {
        animate2( m_animValue, targVal, totalTime, interpType );        
    }
    

    // Interp from A to B
    void animate2( const T &a, const T &b,
                   float totalTime=1.0, InterpType interpType=Interp_SMOOTHSTEP )
    {
        m_behavior = Avar_INTERP;
        m_interpType = interpType;        
        m_currTime = 0.0;
        m_totTime = totalTime;
        m_aVal = a;
        m_bVal = b;
    }

    // Pulse 
    void pulse( const T &a, const T &b,
                float frequency = 1.0f, float offset = 0.0f )
    {
        m_behavior = Avar_PULSE;
        m_frequency = frequency;
        m_currTime = offset;
        m_aVal = a;
        m_bVal = b;
    }
    

    // Call this once per sim step to update
    static void updateAvars( float dt )
    {
        for( typename std::list<Avar<T>*>::iterator ai =  _avarList.begin();
             ai != _avarList.end(); ++ai )
        {
            (*ai)->update( dt );            
        }

    }

protected:

    void update( float dt )
    {
        switch (m_behavior)
        {
        case Avar_STEADY:
            break;
            
        case Avar_INTERP:
            updateInterp( dt );
            break;

        case Avar_PULSE:
            updatePulse( dt );
            break;            
        }        
    }
    
    void updateInterp( float dt )
    {
        m_currTime += dt;
        if (m_currTime >= m_totTime)
        {
            // done with interp
            m_behavior = Avar_STEADY;
            m_value = m_bVal;
            m_animValue = m_bVal;
            return;            
        }

        // not done, update
        float tval = m_currTime / m_totTime;

        // Smoothstep function
        if (m_interpType == Interp_SMOOTHSTEP)
        {
            tval = tval*tval*(3.0-2.0*tval);            
        }

        // apply interp
        m_animValue = ((1.0-tval)*m_aVal) + (tval*m_bVal);        
        
    }
    
    void updatePulse( float dt )
    {
        m_currTime += dt;

        // keep range reasonable
        if (m_currTime > 1000.0f) m_currTime -= 1000.0f;
        
        float tval = (sin( (m_currTime*M_PI*2.0) / m_frequency ) *0.5)+0.5;
        m_animValue = ((1.0-tval)*m_aVal) + (tval*m_bVal);
    }
    

protected:
    AvarBehavior m_behavior;

    // Common stuff
    T m_aVal, m_bVal;    

    // Stuff for Interp vars
    InterpType m_interpType;
    float m_currTime, m_totTime;

    // Stuff for Pulse vars
    float m_frequency, m_offset;
    T m_minVal, m_maxVal;    
    
    // The data
	T m_value;     // The "actual" value this holds
	T m_animValue; // The animated value

    // List of all avars
    // Note you'll have to provide this somewhere,
    // see the USE_AVAR(T) macro
    static std::list<Avar<T> *> _avarList;
};

// Shortcut because this syntax can be confusing. Make sure to call
// this once for each type you want to use with avar.
//
// Example: USE_AVAR(float);
//
// It goes at the toplevel, with other statics
//
#define USE_AVAR(T) \
    template<> std::list<Avar<T> *> Avar<T>::_avarList = std::list<Avar<T> *>()

typedef Avar<float> AnimFloat;
typedef Avar<double> AnimDouble;

#endif
