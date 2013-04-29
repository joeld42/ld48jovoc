//
//  monster_def.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 4/28/13.
//
//

#include "monster_def.h"
#include "world.h"

MonsterDB *MonsterDB::sharedDB()
{
    MonsterDB *monsterDB = NULL;
    if (!monsterDB)
    {
        monsterDB = new MonsterDB();
    }
    return monsterDB;
}


MonsterDB::MonsterDB()
{
    _initMonsters();
}

// TODO: read this from an xml file
void MonsterDB::_initMonsters()
{
    MonsterDefinition monster;

    // ------- Rats -------------------
    
    monster.m_id = "brn_rat";
    monster.m_name = "Brown Rat";
    monster.m_tintColor = World::parseColor("e39d58");
    monster.m_baseHP = 6;
    monster.m_behavior = Behavior_WANDER;
    monster.m_align = Alignment_EVIL;
    monster.m_meshName = "critter";
    monster.m_skinName = "skn_rat";    
    m_monsters[monster.m_id] = monster;
    
    monster.m_id = "dire_rat";
    monster.m_name = "Dire Rat";
    monster.m_tintColor = World::parseColor("1f1724");
    monster.m_baseHP = 12;
    monster.m_behavior = Behavior_WANDER;
    monster.m_align = Alignment_EVIL;
    monster.m_meshName = "critter";
    monster.m_skinName = "skn_rat";
    m_monsters[monster.m_id] = monster;

    // ------- Skeletons -------------------
    
    monster.m_id = "skelly";
    monster.m_name = "Skelly";
    monster.m_tintColor = World::parseColor("deffca");
    monster.m_baseHP = 20;
    monster.m_behavior = Behavior_SEEKPLAYER;
    monster.m_align = Alignment_EVIL;
    monster.m_meshName = "person";
    monster.m_skinName = "skn_skelly";
    m_monsters[monster.m_id] = monster;

    monster.m_id = "crs_skelly";
    monster.m_name = "Cursed Skelly";
    monster.m_tintColor = World::parseColor("db6357");
    monster.m_baseHP = 30;
    monster.m_behavior = Behavior_SEEKPLAYER;
    monster.m_align = Alignment_EVIL;
    monster.m_meshName = "person";
    monster.m_skinName = "skn_skelly";
    m_monsters[monster.m_id] = monster;
    
}

MonsterDefinition MonsterDB::getMonster( const std::string &id )
{
    MonsterDefinition monster;
    
    auto MonsterLookup::iterator mi;
    mi = m_monsters.find( id );
    if (mi != m_monsters.end())
    {
        monster = (*mi).second;
    }
    return monster;
}
