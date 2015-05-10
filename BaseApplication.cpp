#include "BaseApplication.h"

//-------------------------------------------------------------------------------------
BaseApplication::BaseApplication(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mTrayMgr(0),
    mCameraMan(0),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
    up(0),
    down(0),
    left(0),
    right(0),
    state(GameState::Main),
    play_button(0),
    restart_button(0),
    quit_button(0),
    resume_button(0),
    continue_button(0),
    state_label(0),
    playerState(PlayerState::NoFire),
    last_playerState(PlayerState::NoFire),
    weapon(Weapon0),
    scoreboard(0),
    mx(0),my(0),mz(0),scrollMax(4),scrollMin(0),theta(0),phi(0)
{
  cameraDir=Ogre::Vector3(0,-1,-1);
  cameraPos=Ogre::Vector3(0,0,0);
  mSensitivity=0.05;
}

btVector3 o2bVector3(Ogre::Vector3 in){
  return btVector3(in.x,in.y,in.z);
}

//-------------------------------------------------------------------------------------
BaseApplication::~BaseApplication(void)
{
    if (mTrayMgr) delete mTrayMgr;
    if (mCameraMan) delete mCameraMan;

    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}

Monster* BaseApplication::spawnMonster()
{
    /* NOTE: for movement, ninja, every 'x' frames checks tile_map for a new, valid destination*/

    int player_y = player1->getY();
    float y_pos = player1->getPosbt().getY(); //make this a global constant?
    //cout << "\n\nTILE MAP SIZE " << level->x*5 << " x " << level->y*5 << " x " << level->z*5 << "\n\n";

    //Calculate total 2-D dimensions of level (x, y) and randomly choose (x,y) coordinates to be used to pick random tile
    int tile_x_sp = rand() % level->x*5 + 1;
    int tile_y_sp = rand() % level->y*5 + 1;


    //cout << "\nChosen [x,y]: [" << tile_x_sp << ", " << tile_y_sp << "]\n";
    
    //cout << "CHOSEN TILE: " << (short) level->getTile(tile_x_sp/5, tile_y_sp/5) << "\n";
    Ogre::Vector3 spawn_point;
        
    //Check validity of chosen tile, currently this should always work, should put in while loop when more validity conditions are implemented
    //Further validity checks: no other monster on tile, not near player, only tiles with no walls
    if((short)level->getTile(tile_x_sp/5, tile_y_sp/5) != 0)
    {
        cout << "VALID TILE\n";
        
        //Make spawn point the approximate position of the tile
        //Currently hacked the y coordinate to bring ninja's closer to the ground    
        spawn_point = Ogre::Vector3(tile_x_sp*-1, y_pos, tile_y_sp);
    }   
    
    //create new monster
    Monster* m = new Monster(mSceneMgr, spawn_point);
    return m;
}

void BaseApplication::createScene(void)
{
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3f,0.2f,0.25f));
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);


    Ogre::StringVector scores;
    scores.push_back("Level");
    scores.push_back("----------------");
    scores.push_back("Weapon");
    scores.push_back("Ammunition");
    scores.push_back("Monsters Left: ");
    scores.push_back("Music");

    scoreboard = mTrayMgr->createParamsPanel(OgreBites::TL_TOPLEFT, "Scoreboard", 250, scores);
    scoreboard->setParamValue(0, "0");
    scoreboard->setParamValue(3, "0");
    scoreboard->setParamValue(4, "0");
    scoreboard->setParamValue(5, "On");
    mTrayMgr->moveWidgetToTray(scoreboard, OgreBites::TL_TOPLEFT, 0);
    scoreboard->show();

    //level making
    level->constructLevel();

    //add objects to sim
    sim->addObject(player1);
    sim->addObject(level);

    //Monster Code

    srand(time(0));

    for (int i = 0; i < level->num_monsters; i++)
    {
        Monster* m = spawnMonster();
        m->changeState(Monster::STATE_WANDER, level, player1);
        monster_list.push_back(m);
    }

    //============

    for (int i = 0; i < level->num_monsters; i++)
    {
        sim->addObject(monster_list.at(i));
    }

    //setup music
    bgmusic = new BGMusic();

    //setup weapons

    weapon1 = new Weapon(WeaponState::Weapon0);
    weapon2 = new Weapon(WeaponState::Weapon1);
    weapon3 = new Weapon(WeaponState::Weapon2);

    weapon1->setSMP(mSceneMgr);
    weapon2->setSMP(mSceneMgr);
    weapon3->setSMP(mSceneMgr);


    equippedweapon = &weapon1;
    scoreboard->setParamValue(2, (*equippedweapon)->name);

    //lighting

	Ogre::Light * light = mSceneMgr->createLight("light1");
    light->setType(Ogre::Light::LT_POINT);

	light->setDiffuseColour(1.0, 1.0, 1.0);
	light->setSpecularColour(1.0, 0.0, 0.0);

    light->setPosition(Ogre::Vector3(0,20,20));
}

//-------------------------------------------------------------------------------------
bool BaseApplication::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "Render Window");

        return true;
    }
    else
    {
        return false;
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
  mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
	mOverlaySystem = new Ogre::OverlaySystem();
	mSceneMgr->addRenderQueueListener(mOverlaySystem);
}
//-------------------------------------------------------------------------------------
void BaseApplication::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");
    mCamera->setPosition(cameraPos);
    mCamera->lookAt(cameraPos+cameraDir);
    mCamera->setNearClipDistance(1);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
}
//-------------------------------------------------------------------------------------
void BaseApplication::createFrameListener(void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mInputManager = OIS::InputManager::createInputSystem( pl );

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

  	mInputContext.mMouse = mMouse;
  	mInputContext.mKeyboard = mKeyboard;

    mRoot->addFrameListener(this);
}
//-------------------------------------------------------------------------------------
void BaseApplication::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void BaseApplication::createViewports(void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}
//-------------------------------------------------------------------------------------
void BaseApplication::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::createResourceListener(void)
{

}
//-------------------------------------------------------------------------------------
void BaseApplication::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------
void BaseApplication::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool BaseApplication::setup(void)
{
    mRoot = new Ogre::Root(mPluginsCfg);

    setupResources();

    bool carryOn = configure();
    if (!carryOn) return false;

    chooseSceneManager();
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();
    // Load resources
    loadResources();
    createFrameListener();
    createMenu();

    return true;
};
//-------------------------------------------------------------------------------------
void BaseApplication::createMenu()
{
    // Setup GUI
    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mInputContext, this);
    play_button = mTrayMgr->createButton(OgreBites::TL_CENTER, "Play", "Play");
    quit_button = mTrayMgr->createButton(OgreBites::TL_CENTER, "Exit", "Quit");
}
//-------------------------------------------------------------------------------------
bool BaseApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();


    //need to process input
    if(state==Play)
    {
        if (playerState == PlayerState::Fire)
        {
            int val = (*equippedweapon)->fire();
            if (val == 0)
            {

                playerState = PlayerState::Reload;
                last_playerState = PlayerState::Fire;
            }
            else if (val == 1)
            {
              cout << "firing!\n";
              btVector3 dir = btVector3(cameraDir.x,cameraDir.y,cameraDir.z);
              btVector3 pos = player1->getPosbt() + btVector3(0,1,0) + dir/2.0;
              //bulletVector.push_back((*equippedweapon)->spawnBullet(pos,dir,sim));
              (*equippedweapon)->spawnBullet(pos,dir,sim);
            }
        }
        else if (playerState == PlayerState::Reload)
        {
            if ((*equippedweapon)->reload())
            {
                playerState = last_playerState;
            }
        }

        //Monster Code
        int j;
        for(j = 0; j < level->num_monsters; j++)
        {
            //Monster* m_up = monster_list[j];
            //m_up->m_animState->addTime(evt.timeSinceLastFrame);
            //cout << "updating monster"<<j<<"\n";
            //if(!j)
                //monster_list.at(j)->printpos();
            monster_list.at(j)->updateMonsters(level, evt);
        }
        //cout << "monsters updated\n";
    }
    processInput();// move up?

    //step sim after processing input
    if(state==Play){
      //set player speed
        //cout << player1->playerLV.x()<<","<<player1->playerLV.z()<<"\n";
      player1->getBody()->setLinearVelocity(btVector3(player1->playerLV.x(),0.000001,player1->playerLV.z()));
      //cout << phi << "\n";
      player1->setRotation(btQuaternion(btVector3(0,1,0),phi ));
      //set projectile speeds
      /*for(int i=0; i < bulletVector.size();i++){
        btVector3 lv = (bulletVector[i])->linvel();

        bulletVector[i]->getBody()->setLinearVelocity(lv);

      }*/

        GameObject * go;
        for(int i=0; i <sim->getObjectListSize();i++){
            go=sim->getObjectPtr(i);
            //cout << "object "<< go->getName()<<"\n";
            if((go->getName()).compare("bullet")==0){
                btVector3 lv = ((Bullet*)go)->linvel();
                go->getBody()->setLinearVelocity(lv);
            }else if((go->getName()).compare("ninja")==0){            
                btVector3 lv =o2bVector3(((Monster*)go)->m_directionVector);
                Ogre::Real speed = ((Monster*)go)->m_walkSpeed;
                go->getBody()->setLinearVelocity(speed*lv);
            }
        }

      sim->stepSimulation(evt.timeSinceLastFrame,10,1./60.);

      //GAME WIN
      if (level->num_monsters == 0)
      {
        mTrayMgr->showCursor();
        state_label = mTrayMgr->createLabel(OgreBites::TL_CENTER, "Win", "You Win! :)", 400);
        continue_button = mTrayMgr->createButton(OgreBites::TL_CENTER, "Continue", "Next Level");
        restart_button = mTrayMgr->createButton(OgreBites::TL_CENTER, "Restart", "Restart Level");
        quit_button = mTrayMgr->createButton(OgreBites::TL_CENTER, "Exit", "Quit");
        state = Win;
      }

      else if (player1->player_health == 0)
      {
        mTrayMgr->showCursor();
        state_label = mTrayMgr->createLabel(OgreBites::TL_CENTER, "Lose", "You Died! :(", 400);
        restart_button = mTrayMgr->createButton(OgreBites::TL_CENTER, "Restart", "Try Again");
        quit_button = mTrayMgr->createButton(OgreBites::TL_CENTER, "Exit", "Quit");
        state = Lose;
      }


      player1->playerLV=btVector3(0,0,0);
      float scrollSens=1000;
      if(mz>scrollSens)
        mz=scrollSens;
      if(mz<0)
        mz=0;
      float scrollMod=scrollMax*(mz/scrollSens);
      cameraPos=player1->getPos()-(3+scrollMod)*cameraDir + Ogre::Vector3(0,1,0);
      mCamera->setPosition(cameraPos);
      mCamera->lookAt(cameraPos+cameraDir);
      scoreboard->setParamValue(3, std::to_string((*equippedweapon)->ammo_left()) + "/" + std::to_string((*equippedweapon)->total_ammo_left()));
      scoreboard->setParamValue(4, std::to_string(level->num_monsters));
      //cout <<cameraDir.x<<","<<cameraDir.y<<","<<cameraDir.z<<"\n";
    }

    mTrayMgr->frameRenderingQueued(evt);




    if (!mTrayMgr->isDialogVisible())
    {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
    }

    return true;
}
//-------------------------------------------------------------------------------------

bool BaseApplication::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up


    if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }

    if (state == Play)
    {
    /*    else if(arg.key == OIS::KC_9)
            {
                sounds->pauseMusic();
            }
        else if(arg.key == OIS::KC_1)
            {
                sounds->enableSound();
            }*/
        if (arg.key == OIS::KC_P)
        {
            state = Pause;
            mTrayMgr->showCursor();
            state_label = mTrayMgr->createLabel(OgreBites::TL_CENTER, "Pause", "Game Paused", 400);
            resume_button = mTrayMgr->createButton(OgreBites::TL_CENTER, "Resume", "Resume");
            restart_button = mTrayMgr->createButton(OgreBites::TL_CENTER, "Restart", "Restart Level");
            quit_button = mTrayMgr->createButton(OgreBites::TL_CENTER, "Exit", "Quit");
        }

        else if(arg.key == OIS::KC_M)
            {
                bgmusic->playOrPause();
                if (scoreboard->getParamValue(5) == "Off")
                    scoreboard->setParamValue(5, "On");
                else
                    scoreboard->setParamValue(5, "Off");
            }

        else if (arg.key == OIS::KC_R)
        {
            playerState = PlayerState::Reload;
        }

        else if(arg.key == OIS::KC_W){
             up=true;
        }
        else if(arg.key == OIS::KC_A){
            left=true;
        }
        else if(arg.key == OIS::KC_S){
            down=true;
        }
        else if(arg.key == OIS::KC_D){
            right=true;
        }
        else if(arg.key == OIS::KC_1 && weapon != WeaponState::Weapon0){
            weapon = WeaponState::Weapon0;
            playerState = PlayerState::NoFire;
            (*equippedweapon)->cancel_reload();
            equippedweapon = &weapon1;
            (*equippedweapon)->switch_weapon();
            scoreboard->setParamValue(2, (*equippedweapon)->name);
        }
        else if(arg.key == OIS::KC_2 && weapon != WeaponState::Weapon1){
            weapon = WeaponState::Weapon1;
            playerState = PlayerState::NoFire;
            (*equippedweapon)->cancel_reload();
            equippedweapon = &weapon2;
            (*equippedweapon)->switch_weapon();
            scoreboard->setParamValue(2, (*equippedweapon)->name);
        }
        else if(arg.key == OIS::KC_3 && weapon != WeaponState::Weapon2){
            weapon = WeaponState::Weapon2;
            playerState = PlayerState::NoFire;
            (*equippedweapon)->cancel_reload();
            equippedweapon = &weapon3;
            (*equippedweapon)->switch_weapon();
            scoreboard->setParamValue(2, (*equippedweapon)->name);
        }
        //Monster Code
        //============
        /*
        else if(arg.key == OIS::KC_SPACE)
        {

            if (judgement_day > num_monsters)
            {
                judgement_day = 0;
            }

            monster_list[judgement_day++]->killMonster();
            num_monsters--;
        }
        */

        //============
    }

    else if (state == Pause)
    {
        if (arg.key == OIS::KC_P)
        {
            mTrayMgr->clearTray(OgreBites::TL_CENTER);
            mTrayMgr->destroyWidget(state_label);
            mTrayMgr->destroyWidget(resume_button);
            mTrayMgr->destroyWidget(restart_button);
            mTrayMgr->destroyWidget(quit_button);
            mTrayMgr->hideCursor();
            state = Play;
        }
    }

    //mCameraMan->injectKeyDown(arg);
    return true;
}

bool BaseApplication::keyReleased( const OIS::KeyEvent &arg )
{
    //mCameraMan->injectKeyUp(arg);

    if(arg.key == OIS::KC_W){
      up=false;
    }
    else if(arg.key == OIS::KC_A){
        left=false;

    }
    else if(arg.key == OIS::KC_S){
        down=false;
    }
    else if(arg.key == OIS::KC_D){
        right=false;
    }

    return true;
}
void BaseApplication::buttonHit (OgreBites::Button *button)
{
    if (button == play_button)
    {
        // Create the scene
        mTrayMgr->clearTray(OgreBites::TL_CENTER);
        mTrayMgr->destroyAllWidgets();
        cout << "\n\nLEVEL GEN\n\n";
        //generate level
        level=new Level(mSceneMgr);
        //level->generateRoom(4,3);
        level->proceduralLevelGen(1);
        //level->testLevelGen();
        cout << "\nprinting level:\n";
        level->printLevel();

        //create player
        player1= new Player(mSceneMgr);

        //physics
        sim = new Simulator();

        // Create the scene
        createScene();
        mTrayMgr->hideCursor();
        state = Play;
    }
    else if (button == resume_button)
    {
        mTrayMgr->clearTray(OgreBites::TL_CENTER);
        mTrayMgr->destroyWidget(state_label);
        mTrayMgr->destroyWidget(resume_button);
        mTrayMgr->destroyWidget(quit_button);
        mTrayMgr->hideCursor();
        state = Play;
    }
    else if (button == continue_button)
    {
        // Destroy current scene, generate next level

        mTrayMgr->clearTray(OgreBites::TL_CENTER);
        mTrayMgr->destroyWidget(state_label);
        mTrayMgr->destroyWidget(restart_button);
        mTrayMgr->destroyWidget(continue_button);
        mTrayMgr->destroyWidget(quit_button);
        mTrayMgr->hideCursor();
        state = Play;
    }
    else if (button == restart_button)
    {
        // Destroy current scene, regenerate level

        mShutDown = true;
    }
    else if (button == quit_button)
    {
        mShutDown = true;
    }
}
bool BaseApplication::mouseMoved( const OIS::MouseEvent &arg )
{
    if(state==Play){
    mx=(float)arg.state.X.rel;
    my=(float)arg.state.Y.rel;
    mz-=(float)arg.state.Z.rel;
    }
    if (state == Main || state == Pause || state == Win || state == Lose){
        mTrayMgr->injectMouseMove(arg);
      }
      //else
      //mCameraMan->injectMouseMove(arg);
    return true;
}

bool BaseApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (state==Play){
        if (id == OIS::MB_Left)
        {
            last_playerState = PlayerState::Fire;
            if (playerState != PlayerState::Reload)
            {
                playerState = PlayerState::Fire;
            }
        }
    }
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    //mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool BaseApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (state==Play){
        if (id == OIS::MB_Left)
        {
            last_playerState = PlayerState::NoFire;
            if (playerState != PlayerState::Reload)
            {
                playerState = PlayerState::NoFire;
            }
        }
    }
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    //mCameraMan->injectMouseUp(arg, id);
    return true;
}

//Adjust mouse clipping area
void BaseApplication::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void BaseApplication::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

void BaseApplication::processInput(){
  //move camera with mouse
  phi-=mx*mSensitivity;
  theta-=my*mSensitivity;//to not invert mouse

  float r90=3.1400/2.0; // just shy of 90*
  if(theta>r90){
      theta=r90;}
  else if(theta<=-1*r90){
      theta=-1*r90;}

  if(phi>3.14159*2)
    phi-=3.14159*2;
  if(phi<0*2)
    phi+=3.14159*2;


  cameraDir=Ogre::Vector3(sin(phi)*cos(theta),sin(theta),cos(phi)*cos(theta));

  //move camera with keyboard
  float cameraSpeed=5;
  if(up){
    //cameraPos+=cameraSpeed*cameraDir;
    //player1->setLV((cameraSpeed*cameraDir));
    player1->playerLV.setX(cameraSpeed*sin(phi));
    player1->playerLV.setZ(cameraSpeed*cos(phi));
  }
  if(down){
    //cameraPos-=cameraSpeed*cameraDir;
    player1->playerLV.setX(-1*cameraSpeed*sin(phi));
    player1->playerLV.setZ(-1*cameraSpeed*cos(phi));
  }
  if(left){
    //cameraDir.x*xi+cameraDir.z*zi=0
    player1->playerLV.setX(player1->playerLV.x()+cameraSpeed*cos(phi));
    player1->playerLV.setZ(player1->playerLV.z()-cameraSpeed*sin(phi));
    //cameraPos.x+=cameraSpeed*cos(phi);
    //cameraPos.z-=cameraSpeed*sin(phi);
  }
  if(right){
    player1->playerLV.setX(player1->playerLV.x()-cameraSpeed*cos(phi));
    player1->playerLV.setZ(player1->playerLV.z()+cameraSpeed*sin(phi));
    //  cameraPos.x-=cameraSpeed*cos(phi);
      //cameraPos.z+=cameraSpeed*sin(phi);
  }

  //mCamera->setPosition(cameraPos);
  //mCamera->lookAt(cameraPos+cameraDir);

  //cout << "t,p: " << theta << " " << phi << "\n";
  // cout << "pos: {"<<cameraPos.x<<","<<cameraPos.y<<","<<cameraPos.z<<"}\n";
   //cout << "dir: {"<<cameraDir.x<<","<<cameraDir.y<<","<<cameraDir.z<<"}\n";

  mx=0;
  my=0;
  //mz=0;

}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        BaseApplication app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
