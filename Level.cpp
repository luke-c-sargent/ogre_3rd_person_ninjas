#include "Level.h"
#include <stdlib.h>
#include "GameEnums.h"

Level::Level(){}

Level::Level(Ogre::SceneManager* smptr)
{
  x=y=z=0;
  smp=smptr;
  levelNode = smp->getRootSceneNode()->createChildSceneNode("Level_node");
  tileset=new Tile(levelNode, smp);
  cout << "\n\nLEVEL MADE: "<<levelNode<<"\n\n";

}

short Level::getTile(int xi, int yi, int zi)
{
  return tile_map[xi + yi*x + zi*y*x]; //linear to 3d storage
}

Ogre::Vector3 Level::getStart()
{
  return start;
}

//create entities and such from level data
void Level::constructLevel(){
  //Ogre::Vector3 offset;//where to put tile
  //std::string tileID="defaultID";

  cout << "\nCONSTRUCTING LEVEL\n";
  for(int k=0; k < z; k++){
    for(int j=0; j < y; j++){
      for(int i=0; i < x; i++){
        tileset->genTile(getTile(i,j,k),i,j,k);

      }
    }
  }
}//void constructLevel()

/*
  tiles:
    0. no tile  1. no wall  2. north  3. south 4. east  5. west
    6. north-e  7. north-w  8. south-e  9. south-w
    10. nse 11. nsw 12. new 13. sew
*/

void Level::generateRoom(int xi,int yi){
  x=xi;y=yi;z=1;
  if(xi<=1 || yi<=1){
    cout << "Level::generateRoom bounds error\n";
    exit(0);
  }
  cout << "\nGENERATING ROOM\n";
  short tileR=1;

  for(int j=0; j < yi; j++){
    for(int i=0; i < xi; i++){
      if(i==0){
        if(j==0)
          tileR=((short)Tiles::SW);
        else if(j==(yi-1))
          tileR=((short)Tiles::NW);
        else
          tileR=((short)Tiles::W);
      }
      else if(i==xi-1){
        if(j==0)
          tileR=((short)Tiles::SE);
        else if(j==(yi-1))
          tileR=((short)Tiles::NE);
        else
        tileR=((short)Tiles::E);
      }
      else if(j==0){
        tileR=((short)Tiles::S);
      }
      else if(j==yi-1){
        tileR=((short)Tiles::N);
      }
      else
        tileR=((short)Tiles::NOWALL);
      tile_map.push_back(tileR);
      cout << tileR;
      tileR=1;
    }
  }

}

//simple test platform
int Level::generateLevel(int xi, int yi, int zi)
{
  x=xi;y=yi;z=zi;
  cout << "\nGENERATING LEVEL\n";
  for(int i=0; i < xi;i++){
    for(int j=0; j < yi;j++){
      for(int k=0; k < zi;k++){
        short tile = rand()%13+1;
        tile_map.push_back(tile);
  } } }
  cout << "\n\n LEVEL GENERATED \n\n";
}

Level::~Level(){
  delete(tileset);
}

void Level::printLevel(){
  for(int k=0; k<z;k++){
    cout<<"\nLevel:"<<k<<"\n";
    for(int i=0; i<y;i++){
      for(int j=0; j<x;j++){
        cout <<"["<<j<<i<<k<<"="<<getTile(j,i,k)<<"]";
      }
      cout <<"\n";
    }
  }
}
