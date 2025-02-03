#pragma once
#ifndef GAMEENTITIES_H
#define GAMEENTITIES_H

// Manager
#include "../manager/RenderManager.hpp"
#include "../manager/WorldManager.hpp"
#include "../manager/BodyFactory.hpp"

// Game
#include "Entity.hpp"
#include "../utils/include/enum.hpp"
#include "utils/include/Timer.hpp"

// Librairies importées
#include <algorithm>
#include <memory>
#include <math.h>
#include "../../libs/RapidJson/include/rapidjson/document.h"
#include "../../libs/RapidJson/include/rapidjson/writer.h"
#include "../../libs/RapidJson/include/rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;
class Skill;
class WorldManager;
class LastingEffect;
class Player; 
class Item;
class Boots;
class ObstacleRayCastCallBack : public b2RayCastCallback {
    public:
         ObstacleRayCastCallBack()    
        {
            
        }
    
        float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
        {
            m_fixture = fixture;
            m_point = point;
            m_normal = normal;
            m_fraction = fraction;
            
            STypeInstance* entity =  (STypeInstance*)(fixture->GetUserData().pointer);
            if(fixture->GetFilterData().categoryBits == BORDERCATEGORY){
                this->hit = true;
                return 0;
            }
            if(entity->instance == TILEWATER || entity->instance == TILEWALL){
                this->hit = true;
                return 0;
            }
            return 1;
        }
        void setHit(bool hit){
            this->hit = hit;
        }
        bool getHit(){
            return this->hit;
        }
        b2Fixture* m_fixture;
        b2Vec2 m_point;
        b2Vec2 m_normal;
        float m_fraction;

    private:
        bool hit=false;
};

/** ---------------------------------------------------- [Being] -------------------------------------------
 * Ici on gère le header de being.
 */

class Being : public Entity
{
    public:
        Being(float x,float y, float w, float h,float d, float f,float r,b2Shape* shape,BodyType type,void* object,uint8 category,ETypeInstance typeInstance,groupIndex index,WorldManager* parent);

        virtual void acceptSkill(Skill* skill){cout << "touché" << endl;}

        void setCanTakeDame(bool v){CTD = v;}
        void setName(string name){this->name = name;}
        void setWalkSpeed(float speed){this->walkspeed = speed;}
        void setLook(ELook look){this->look = look;}
        void setSimpleLook(ELook look){this->simpleLook= look;}
        void setAcceleration(float acceleration){this->acceleration = acceleration;}
        void setState(EntityState state){this->state = state;}
        void setHp(int hp){this->currHp = hp;}
        void setMaxHp(int hp){this->maxHp = maxHp;}
        void update(float deltaT) override { getAnimation(state).getTimer()->update(deltaT); }
        void takeDamage(int damage);
        void MoveTo(float x, float y);
        void Move(float x, float y);
        
        WorldManager* getParent(){return this->parent;}
        EntityState getState(){return this->state;}
        ELook getLook(){return this->look;}
        ELook getSimpleLook(){return this->simpleLook;}
        Vec2* getVelocity(){return &velocity;}

        int getHp(){return currHp;}
        int getMaxHp(){return maxHp;}
        float getWalkSpeed(){return walkspeed;}
        float getAcceleration(){return this->acceleration;}
        bool canTakeDamage(){return CTD;}
        
    protected:
        WorldManager* parent;
        b2World* world;
        Vec2 velocity;

        // [Propriétés] ---------------------------------------------
        float walkspeed = 200;
        float acceleration = 1;
        string name;
        EntityState state = IdleState;

        // Regard plus complet.
        ELook look = Right;
        // Regard simple droite ou gauche.
        ELook simpleLook = Right;
        // Liste des effets actifs sur le joueur.
        vector<LastingEffect*> effects;

        // [Stats] --------------------------------------------------
        int armor;
        // Points de vie actuels.
        int currHp = 100;
        // Points de vie maximum.
        int maxHp = 100;
        // Can take damage.
        bool CTD;
};
/*--------------------------------------------Item----------------------------------------------------*/

class Object
{
private:
    
public:
    Object();
    virtual void use(Player* player)=0;
    ~Object();
    bool Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer);
    bool Deserialize(const rapidjson::Document& obj);
};

class Item: public Object
{
protected:
    bool used =false;
    int amount = 1;
    Timer cooldown;
public:
    Item();
    ~Item();
    void setUsed(bool used){
        this->used = used;
    }
    bool getUsed(bool used){{
        return used;
    }}
    virtual bool isLifePotion()=0;
    void increaseAmount();
};

class Chest: public Object
{  
private:
    int armor;
    int luck; 
public:
    Chest(int lvl);
    ~Chest();
    void use(Player* player);
};


class Glove:public Object
{
private:
    int dexterity;
    int armor;  
public:
    Glove(int lvl);
    ~Glove();
    void use(Player* player);
};

class Boots:public Object
{
private:
    int armor;
    int dexterity;
public:
    Boots(int lvl);
    ~Boots();
    void use(Player* player); 
};
class Helmet:public Object
{
private:
    int armor;
    int hp;
public:
    Helmet(int lvl);
    ~Helmet();
    void use(Player* player);
};
class Sword:public Object
{
    public:
        Sword(int lvl);
        ~Sword();
        void use(Player* player);
    private:
        int strength;
        int dexterity;
        int luck;
};

class LifePotion:public Item 
{
private:
    int healedHP = 10;
    
public:
    LifePotion();
    ~LifePotion();
    void use(Player* player);
    void increaseAmount();
    bool isLifePotion(){
        return true;
    }
};


/** ---------------------------------------------------- [PLAYER] -------------------------------------------
 * Ici on gère le header du joueur.
 */

class Player : public Being
{
    public:
        Player(float x, float y,b2Shape* shape, WorldManager* parent,int lvl);

        void update(float deltaT) override;
        void onTouch(b2Contact* contact,float deltaT) override;
        void BeginContact(b2Contact* contact,float deltaT) override;
        void EndContact(b2Contact* contact,float deltaT) override;
        void setAngle(float angle);
        void getHit(STypeInstance* instance);
        void setState(EntityState state){this->state = state;}
        void setId(short int id){this->id = id;}
        void gainXp(int exp);

        int getExp(){return exp;}
        int getCapExp(){return capExp;}
        short int getId(){return id;}
        int getWidthArrow(){return widthArrow;}
        int getHeightArrow(){return heightArrow;}
        bool applySkill(const PSkill& skill);
        bool isSkillFinished();
        bool getInvicible(){return this->invicible;}
        float getAngle();
        EntityState getState(){return this->state;}
        Texture getArrow(){return directionArrow;}
        bool Deserialize(const rapidjson::Document& obj);
        bool Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer);
        void addItemToInventory(shared_ptr<Object> obj){
            if(this->inventory.size()<20){
                    this->inventory.push_back(obj);
            }
        }
        Timer getHslashTimer(){
            return timerHslash;
        }
        string getName(){
            return this->name;
        }
        int getStrength(){
            return strength;
        }
        
    private:
        
        // Indique au joueur sur quelle animation il est placé actuellement.
        int animationIndex = 0;
        PSkill currentSkill;
        map<PSkill,std::shared_ptr<Skill>> skills;
        Timer timer;
        Timer timerKnockback;
        Texture directionArrow;
        int widthArrow;
        int heightArrow;
        // Par défaut à 0. 0 Est aussi l'identifiant du joueur hôte.
        short int id = 0;
        Timer timerHslash;

        Vec2 vectCollide;
        float angle = 0;
        bool invicible = false;
        vector<shared_ptr<Object>> inventory;
        map<string,shared_ptr<Item>> items;
        shared_ptr<Item> equippedItem = NULL;
        Boots* boots = NULL;
        Helmet* helmet = NULL;
        Chest* chest = NULL;
        Glove* glove = NULL;
        //Stats spécifiques au joueur.
        string name;
        int niveau=1;
        int exp=0;
        int strength=101;
        int capExp=100;
        int statPoint=0;
        int magic=10;
        int dexterity=10;
        int luck=10;
};

class Projectile : public Entity,public enable_shared_from_this<Projectile> {
    
    public:
        Projectile(float x,float y,b2Shape* shape,WorldManager* parent);
        void update(float deltaT) override;
        void onTouch(b2Contact* contact,float deltaT) override;
        void BeginContact(b2Contact* contact,float deltaT) override;
        void EndContact(b2Contact* contact,float deltaT) override;
        void setvX(float vX){
            this->vX = vX;
        }
        void setVy(float vY){
            this->vY = vY;
        }
        void setAngle(float angle){
            this->angle = angle;
        }
        bool getActive(){
            return active;
        }
        void setSimpleLook(ELook look){
            this->look = look;
        }
        ELook getSimpleLook(){
            return this->look;
        }
        int getEarnedXp(){
            return earnedXP;
        };

    protected:
        WorldManager* parent;
        bool active = true;
        float vX;
        float vY;
        float angle;
        int level = 0;
        int earnedXP = 150;
        ELook look;
};

/** ---------------------------------------------------- [ENEMY] -------------------------------------------
 * Ici on gère les fonctions de l'ennemi.
 */

class Enemy : public Being
{
    public:
        Enemy(float x, float y, int w, int h,int expGift,int lvl, b2Shape* shape, WorldManager* parent);
        
        void update(float deltaT) override;
        void onTouch(b2Contact* contact,float deltaT) override;
        void changeDirection(float deltaT);
        void setKilledBy(Player player){
            this->killedBy =&player;
        }
        void EndContact(b2Contact* contact) {}
        void BeginContact(b2Contact* contact, float deltaT) override;
        void acceptSkill(Skill* skill);
        void setKnockbackVelocity(Vec2 vec){
            this->reverseKnockback = vec;
        }
        int getStrength(){
            return force;
        }
        int getArmor(){
            return armor;
        }
        bool getActive(){
            return active;
        }
        int getGiftXp(){
            return this->expGift;
        }
        void setKnockback(bool knockback){
            this->knockback = knockback;
        }
        void EndContact(b2Contact* contact,float deltaT) {
            b2Fixture* fixtureA = contact->GetFixtureA();
            b2Fixture* fixtureB = contact->GetFixtureB();
            STypeInstance* instanceA = (STypeInstance*)(fixtureA->GetUserData().pointer);
            STypeInstance* instanceB = (STypeInstance*)(fixtureB->GetUserData().pointer);
            switch (instanceA->instance)
            {
                case TARGETTYPE:
                {
                    auto it = std::remove(potentialTarget.begin(), potentialTarget.end(), ((Player*)instanceB->ptr));

                    // Erase the matching elements from the end of the vector
                    potentialTarget.erase(it, potentialTarget.end());
                    break;
                }
                case KNOCKBACKTYPE:
                {
                    Player* player = ((Player*)instanceB->ptr);
                    player->setHp(player->getHp()-10);
                    //printf("player hp : %d",player->getHp());
                    break;
                }
            }
            switch (instanceB->instance)
            {
                case TARGETTYPE:
                {
                    auto it = std::remove(potentialTarget.begin(), potentialTarget.end(), ((Player*)instanceA->ptr));
                    
                    // Erase the matching elements from the end of the vector
                    potentialTarget.erase(it, potentialTarget.end());
                    break;
                }
                case KNOCKBACKTYPE:
                {
                    Player* player = ((Player*)instanceA->ptr);
                    player->setHp(player->getHp()-10);
                    printf("player hp : %d",player->getHp());
                    break;
                }
            }
        }

    protected:
        void followTarget(float deltaT);
        void lookForTarget();

        Player* cible = nullptr;
        vector<Player*> potentialTarget;
        b2Shape* overLapingShape = nullptr;

        // Timer qui s'occupe du temps en état passif.
        Timer timerEtatPassif;
        Timer attackCoolDown;

        Vec2 destination;
        Vec2 reverseKnockback;
        const float tolerance = 0.1f;
        float maxAggro = 20.0f;
        float range = 3;

        //Stats spécifiques à l'ennemi.
        int force;
        int armor;
        ObstacleRayCastCallBack rayCastCallback;
        bool collide=false;
        bool active = true;
        bool knockback;
        Vec2 knockbackVelocity;
        int expGift;
        int lvl;
        Player* killedBy; 
};

class Skeleton : public Enemy {
    public: 
        Skeleton(float x, float y, int w, int h,int expGift,int lvl, b2Shape* shape, WorldManager* parent);

        void update(float deltaT) override;

    private:
        
};

class SkeletonArcher : public Enemy {
    public: 
        SkeletonArcher(float x, float y, int w, int h,int expGift,int lvl, b2Shape* shape, WorldManager* parent);

        void update(float deltaT) override;
    
    private:
        float attackRange = 10.0f;
};

class SkeletonSpearman : public Enemy {
    public: 
        SkeletonSpearman(float x, float y, int w, int h,int expGift,int lvl, b2Shape* shape, WorldManager* parent);
        void update(float deltaT) override;

    private:
};

/** ---------------------------------------------------- [SKILLS] -------------------------------------------
 * Ici on gère le header des skills.
 */

class LastingEffect {
    public: 
        LastingEffect();

    private: 
        Timer timer;
};

class DizzyEffect : public LastingEffect {
    public:


    private:

};

/**
 * @brief Classe parente des skills.
 * Un skill peut avoir deux conditions d'arrêt, une condition liée au temps de l'animation,
 * qui est testée dans le worldmanager avant de switcher sur un nouvel état.
 * Ou alors une condition intrinsèque au skill, par exemple projectile qui meurt.
 * Cette condition dépend de la variable isFinished qui est par défaut à true. 
 */
class Skill {
    public:
        Skill(Player* player, b2World* world) : parent(player), world(world) {}

        virtual ~Skill() = default;

        Player* getParent(){return parent;}

        /**
         * @brief Lance le skill.
         * 
         * @return true Si le skill s'est lancé.
         * @return false Si le skill ne s'est pas lancé.
         */
        virtual bool use() {return false;}

        /**
         * @brief La durée de l'attaque de HSlash c'est la durée de l'animation.
         * Donc le critère pour changer d'état dans le worldManager c'est de tester,
         * que l'animation se soit terminée.
         * @return true 
         * @return false 
         */
        virtual bool isFinished() = 0;

        virtual void applySkillPlayer(Player* player){};
        virtual void applySkillEnemy(Enemy* enemy){};
        
    protected:
        Texture icon;
        Timer coolDown;
        bool finished = true;
        bool unlocked;
        Player* parent;
        int level = 0;
        b2World* world;
};

class HSlash : public Skill {
    public:
        HSlash(Player* player, b2World* world) : Skill(player,world) {}

        bool use() override;
        bool isFinished() override {return finished;}
        void applySkillEnemy(Enemy* enemy);
        void applySkillPlayer(Player* player){};

    private:
        
};

class HSlashRayCast : public b2RayCastCallback {
    public:
        HSlashRayCast(HSlash* parent, int maxNbHit) : maxNbHit(maxNbHit), parent(parent)
        {
            m_fixture = NULL;
        }
    
        float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
        {
            m_fixture = fixture;
            m_point = point;
            m_normal = normal;
            m_fraction = fraction;
            
            uintptr_t udata = m_fixture->GetUserData().pointer;
            /*Enemy* enemy = reinterpret_cast<Enemy*>(udata);
            enemy->acceptSkill(parent);*/

            STypeInstance* typeInstance = reinterpret_cast<STypeInstance*>(udata);         
          
            switch(typeInstance->instance){
                case ENEMYTYPE:
                    Enemy* enemy = reinterpret_cast<Enemy*>(typeInstance->ptr);
                    enemy->acceptSkill(parent);
                    
                    break;
            }

            ++nb;
            if(nb == maxNbHit) return 1;
            return 0;
        }

        b2Fixture* m_fixture;
        b2Vec2 m_point;
        b2Vec2 m_normal;
        float m_fraction;

    private:
        int maxNbHit;
        int nb = 0;
        HSlash* parent;
};

/**
 * @brief Une entité a besoin d'un body pour fonctionner avec la physique de Box2d.
 * En fonction du type de body (static, kinematic et dynamic), ses interractions physiques sont différentes.
 */
class EntityFactory {
    public:
        static EntityFactory& getInstance(WorldManager* parent);
        static EntityFactory& getInstance();

        void makePlayer(float x,float y,int lvl);
        void makeSkeleton(float x,float y,float w,float h,int playerLvl);
        void makeArcherSkeleton(float x,float y,float w,float h,int playerLvl);
        void makeProjectile(float x,float y,float vX,float vY,float angle);
    protected:
        EntityFactory(WorldManager* parent);

        static EntityFactory* instance;

        
        WorldManager* parent;
        
};


#endif