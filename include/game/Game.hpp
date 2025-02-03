#pragma once
#ifndef GAME_H
#define GAME_H

#include "../manager/RenderManager.hpp"
#include "../manager/MenuManager.hpp"
#include "../manager/WorldManager.hpp"
  
#include <chrono>
#include <iostream>
#include <time.h>

using namespace std::chrono;
using namespace std;

class MenuManager;
class WorldManager;

/**
 * @brief 
 *  
 */
class Game
{
    public:
        Game(char gameTitle[], int width, int height,bool fullscreen);
        ~Game();

        void gameLoop();
        void setState(GameState s);
        void stop(){isRunning = false;}
        void setSpecialEffect(bool a){specialEffect = a;}
        void setShowFPS(bool a){showFPS = a;}
        void setOnline(bool a){online = a;}
        void setHost(bool a){host = a;}
        void saveGame(string filename);
        double getFps(){return fps;}
        bool showingFPS(){return showFPS;}
        bool isEffect(){return specialEffect;}
        bool isHost(){return host;}
        bool isOnline(){return online;}
        string getLoadedSave(){
            return loadedSave;
        }
        
        void setLoadedSave(string path){
            this->loadedSave = path;
        }
        
        float* getDeltaT();
     
    private:
        void input();        
        void update();
        void render();

        void tick(Uint32& previous);
        float deltaT;
        double fps;
        struct nk_context *ctx;

        bool isRunning = true;
        bool resized = false;
        bool specialEffect = false;
        bool debugMode = false;
        bool showFPS = true;  
        bool online = false;  
        bool host = false;

        map<GameState,IActor*> actors;
        
        RenderManager* renderManager;
        MenuManager* menuManager;
        WorldManager* worldManager;

        GameState state = InMenu;
        string loadedSave;
        Texture branches;
};

#endif