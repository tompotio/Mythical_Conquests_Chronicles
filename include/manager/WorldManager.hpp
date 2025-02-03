#pragma once
#ifndef WORLDMANAGER_H
#define WORLDMANAGER_H

#include "../utils/include/enum.hpp"

// Manager
#include "RenderManager.hpp"
#include "IActor.hpp"
#include "BodyFactory.hpp"
#include "NetworkManager.hpp"
#include "SoundManager.hpp"

// Game
#include "../game/Entity.hpp"
#include "../game/Game.hpp"
#include "../game/Map.hpp"
#include "../game/GameEntities.hpp"
#include "ContactListener.hpp"

// Librairies importées
#include "../../libs/Box2d/include/box2d/box2d.h"
#include <SDL_mouse.h>
#include <math.h>

using namespace rapidjson;

class Game;
class Player;
class Enemy;
class EntityFactory;
class Projectile;

/**
 * @brief Acteur de Game qui fait le monde.
 * Étant donné Game, la classe qui orchestre les différents managers.
 * WorldManager, s'occupe de traiter le monde.
 * Il instancie les différentes données du monde dont les joueurs.
 */
class WorldManager : public IActor
{
    public:
        WorldManager(Game* game);
        ~WorldManager();
        void render() override;
        void update(float deltaT) override;
        void updateScreenSize() override;
        void loadWorld(EWorld world);
        void saveGame(string namefile);
        void addAnimationToEntity(Entity* entity, const string& filepath, int nbFrames, int h, int w, EntityState state,float speed);
        void renderProjectiles();
        void makeProjectile(float x, float y,float vX,float vY,float angle);
        void clearWorld();

        Player* getClosestPlayer(Entity* entity);
        Enemy* getClosestEnemy(Entity* entity);
        b2World* getWorld(){return this->world;}
        vector<Player>* getPlayers(){return &(this->players);}
        vector<shared_ptr<Entity>>* getDeletedCache(){return &(this->deletedCache);}
        vector<shared_ptr<Enemy>>* getEnemies(){return &(this->enemies);}
        vector<std::shared_ptr<Projectile>>* getProjectiles(){
            return &(this->projectiles);
        };

    private:
        void initializeDefaultDatas();
        void initializePhysicalWorld();
        void initializeCamera();
        void initializePlayer();
        void loadTest();
        void loadLobby();
        void sendMessage();
        void sendCoordinates();
        void analyseDatas(const char* datas);
        void fpsCounter();
        void changePlayerState(EntityState state);
        void removeMoveInput(SDL_Keycode key);
        void addMoveInput(SDL_Keycode key);
        void removeCommandInput(SDL_Keycode key);
        void addCommandInput(SDL_Keycode key);
        void zoomIn(float deltaT);
        void zoomOut(float deltaT);
        void updateCameraSize();
        void updateCameraPosition();
        void updateInputStates(float deltaT);
        void deleteEntities();
        void updateEntities(float deltaT);
        void updatePlayerVelocity();
        void updateGui(float deltaT);
        void getMouseAngle();
        void renderPlayers();
        void renderEnemies();
        void renderDirectionArrow();
        void chatBox();
        void bulb();
        void SubMenu();
        void UserUI();
        void makeNewMonster();
        void prepareNetworkDatas();
        void updateNetworkDatas();
        void sendNetworkDatas(TCPsocket socket = nullptr, bool ignoreAll = false);
        void casesDatas(Document* document, NetworkCommands commande);
        void parseSave();
        Vec2 centerTextureOnHitBox(Entity* entity, const Vec2& coordonnees, Animation& animation);
        Document donneesReseauJson;
        vector<NetworkCommands> commandes; // Liste des commandes qui réfèrent aux actions envoyées à travers le fichier Json par réseau.
        vector<string> messages;
        char input_buffer[512];

        string popUpMessage;
        bool showPopUP = false; 
        bool debugMode = false;
        bool inGame = false;
        struct nk_context* ctx;
        int yMouse;
        int xMouse;
        int lvlGame;
        Game* game;
        RenderManager* renderManager;
        EntityFactory* entityFactory;
        NetworkManager& networkManager = NetworkManager::getInstance();
        Map* mapWorld;
        Camera* camera;
        BodyFactory* factory;
        int monsterKilled = 0;
        // Vélocité du joueur.
        Vec2* velocity;

        map<SDL_Keycode,bool> debounce;
        vector<SDL_Keycode> moveInputs;
        vector<SDL_Keycode> commandInputs;
        vector<SDL_Keycode> inputs;

        Player* playerZero;
        vector<std::shared_ptr<Enemy>> enemies;
        vector<Player> players;
        vector<std::shared_ptr<Projectile>> projectiles;
        
        vector<shared_ptr<Entity>> deletedCache;

        Texture mapWorldText;
        Texture gameMap;

        b2Body* wall;
        b2World* world;

        bool clicked = false;

        int windowWidth;
        int windowHeight;
        int mapH;
        int mapW;

        int subMenuWidth;
        int subMenuHeight;

        int healthBarWidth;
        int healthBarHeight;

        const char* txtLancer = "Lancer un serveur";
        const char* txtFermer = "Fermer le serveur";
        const char* txtRejoindre = "Rejoindre un serveur";
        const char* txtQuitter = "Quitter le serveur";

        // Vitesse en mètre. (convertir en pixel pour évaluer sur l'écran + le facteur de zoom).
        int fpsCounterWidth;
        int fpsCounterHeight;
        int fps; // current fps displayed on the screen.
        float CDFPS = 0;
        float timeStep = 1.0f/60.0f;
        int velocityIteration = 6;
        int positionIteration = 2;

        Timer correctionTimer = Timer(2);

        map<NuklearImage, struct nk_image> nkImages;

        float zoomfactor = 1.0f;
        bool zoomed = false;
        bool canMove = true;
        bool subMenuOn = false;
        bool pause = false;

        Texture heartimg;
        Texture starimg;
};

#endif
