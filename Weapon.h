#pragma once

#include <Ogre.h>
#include <GameObject.h>
#include "GameEnums.h"
#include <stdio.h>

using std::cout;

class Weapon{
public:
    Weapon(const int weapon);
    ~Weapon(void);

    void fire();
    void reload();
    void cancelReload();
    int ammo_left();
    int total_ammo_left();

protected:
    Ogre::Timer* fireTimer;
    Ogre::Timer* reloadTimer;
    Ogre::Real firetime;
    Ogre::Real reloadtime;
    // Ogre::Real lastCPUtime;
    Ogre::Real power;
    int ammo_cap;
    int total_ammo_cap;
    int ammo;
    int total_ammo;
    bool reloadBool;

};