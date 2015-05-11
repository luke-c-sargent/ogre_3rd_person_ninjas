#pragma once

#include "GameObject.h"
// #include "Sound.h"
#include <SDL/SDL_mixer.h>
#include <SDL/SDL.h>

using std::cout;

class MonsterSound{
public:  
	MonsterSound();
	virtual ~MonsterSound();
	bool loadMedia();
	void closeMedia();
	void monster_aggro();

protected:	
	int x = 0;

private:
	//Loading success flag
	bool success = true;

	//The music that will be played
	Mix_Chunk *aggro_sound = NULL;
};