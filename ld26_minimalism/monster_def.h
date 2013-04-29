//
//  monster_def.h
//  ld48jovoc
//
//  Created by Joel Davis on 4/28/13.
//
//

#ifndef __ld48jovoc__monster_def__
#define __ld48jovoc__monster_def__

#include <map.h>

#include "actor.h"

class MonsterDefinition
{
public:
    std::string m_id;
    std::string m_name;
    
    std::string m_meshName;
    std::string m_skinName;
    
    int m_baseHP;
    vec3f m_tintColor;
    int m_align;
    int m_behavior;
};

typedef map<std::string,MonsterDefinition> MonsterLookup;

class MonsterDB
{
public:
    static MonsterDB *sharedDB();
    
    MonsterDB();
    
    MonsterDefinition getMonster( const std::string &id );
    
protected:
    void _initMonsters();
    MonsterLookup m_monsters;
};

#endif /* defined(__ld48jovoc__monster_def__) */
