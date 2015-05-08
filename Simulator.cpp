#include "Simulator.h"
#include "GameObject.h"
#include "Monster.h"
#include "Weapon.h"
#include <stdio.h>
#include <vector>

using std::vector;

using std::cout;

struct ContactCallback:public btCollisionWorld::ContactResultCallback
{
  GameObject* ptr, *a,*b;
  ContactCallback(GameObject* ptri) {ptr=ptri;}

  btScalar addSingleResult(btManifoldPoint& cp,
      const btCollisionObjectWrapper* colObj0Wrap,
      int partId0,
      int index0,
      const btCollisionObjectWrapper* colObj1Wrap,
      int partId1,
      int/*t*/ index1)
  {
      //cout << a->getName() << " hit "<<b->getName()<< "\n";
      //return 666;
    b->hit=true;
  }
  void setAB(GameObject* ai,GameObject* bi){
    a=ai;
    b=bi;
  }
};

Simulator::Simulator(){
    //initialize bullet world
    collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfig);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, broadphase, solver,collisionConfig);
    //create world
    dynamicsWorld->setGravity(btVector3(0,-100, 0));//386.09 inches/s^2 = 9.8m/s^2

    //contact callback context
    ccp=new ContactCallback(objList[0]);

}

void Simulator::stepSimulation(const Ogre::Real elapsedTime, int maxSubSteps, const Ogre::Real fixedTimestep) {
    //cout << objList.size()<<"\n";
    //cout << "about to step\n";
    dynamicsWorld->stepSimulation(elapsedTime,maxSubSteps,fixedTimestep);
    //cout << "stepped\n";

    btVector3 start, end;
    GameObject * a = objList[1];//level
    GameObject * p = objList[0];//player
    //p->printpos();
    //update in ogre
    vector<int> deadBullets;
    for(int i=0; i < objList.size();i++){//step through all objects
        if((objList[i]->getName()).compare("bullet")==0){
          ccp->setAB(a,objList[i]);
          dynamicsWorld->contactPairTest(a->getBody(),objList[i]->getBody(),*ccp);

          if(((Bullet*)objList[i])->hit)
            deadBullets.push_back(i);
          else
            objList[i]->updateTransform();
        }
        //Monster Code
        
        else if((objList[i]->getName()).compare("ninja") == 0)
        {
        
          /* PUT THIS IN MONSTER.CPP */
          Monster* m = (Monster*)objList[i];
          
          btVector3 monster_pos = m->getPosbt();
          btVector3 player_pos = p->getPosbt();

          btScalar personal_space = (monster_pos - player_pos).length(); //player's distance from monster
          
          //Based on monster's states, decide what to do
          if(m->m_state != Monster::STATE_ATTACK && personal_space <= m->m_attackRange) 
          {
            //monster currently not in attack state but player is in his attackRange

            //if close enough, monster should attack the player
            m->changeState(Monster::STATE_ATTACK, (Level*) a, (Player*) p);

            //follow the player
            m->changeDestination((Level*) a, (Player*) p);
          }
          else if(m->m_state == Monster::STATE_ATTACK && personal_space <= m->m_attackRange) 
          {
            //monster currently in attack state and player is still in attackRange

            //updates to player's new position
            m->changeDestination((Level*) a, (Player*) p);
          }
          else if (m->m_state == Monster::STATE_ATTACK && personal_space >= m->m_attackRange)
          {            
            //monster currently in attack state but player left his attackRange
           
            //Monster goes back to wandering, picks random destination
            m->changeState(Monster::STATE_WANDER, (Level*) a, (Player*) p);
          }
          else if (m->m_state != Monster::STATE_ATTACK)
          {
            //Monster is wandering around, be checking for walls/obstacles etc.

            //Ray from monster's current position to his destination        
            end = btVector3(m->m_destinationVector.x, m->m_destinationVector.y, m->m_destinationVector.z);
          
            //Create call back object and run rayTest
            btCollisionWorld::ClosestRayResultCallback rayCallBack(monster_pos, end);
            dynamicsWorld->rayTest(monster_pos, end, rayCallBack);

            //Check whether ray hit any obstacle
            if(rayCallBack.hasHit())
            {
            
              btVector3 hit = rayCallBack.m_hitPointWorld; //get position of obstacle in ray's way
              btScalar distance = (monster_pos - hit).length(); //calculate distance to obstacle

              //cout << "\nrayCallBack hit something \n" << distance << " meters away!!\n";
            
              //If ninja is close enough to obstacle, he changes his destination
              if (distance < 5.0f)
              {
                cout << "\nMonster saw an obstacle (wall), must change destination\n";
                m->changeDestination(((Level*)a));
              }
            
            }            

          }          

          objList[i]->updateTransform();
        }
        //===========
        //*/
        else {
          objList[i]->updateTransform();
        }

        //cout << objList[i]->getName()<<":";
        //objList[i]->printpos();

    }
    //cout << "for list one ended\n";
    for(int i=deadBullets.size()-1; i >=0;i--){
      Bullet * go=(Bullet*)objList[deadBullets[i]];
      //delete objList[deadBullets[i]];
      go->getSMP()->destroySceneNode(go->getNode());
      objList.erase(objList.begin()+deadBullets[i]);
    }
    //cout << "end stepsim\n";
}

btDiscreteDynamicsWorld* Simulator::getWorld(){
    return dynamicsWorld;
}

void Simulator::addObject (GameObject* o) {
  //cout << "\nADDING"<< o<<"\n";
    objList.push_back(o);
    //cout << "\ngetting body...   \n";
    dynamicsWorld->addRigidBody(o->getBody());
    //cout << " ... got body\n";
}

GameObject * Simulator::getObjectPtr(int i){
        return objList[i];
}

int Simulator::getObjectListSize(){
  return objList.size();
}