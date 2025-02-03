#pragma once
#ifndef BODYFACTORY_H
#define BODYFACTORY_H

#include <iostream>

#include "../../libs/Box2d/include/box2d/box2d.h" 
#include "../utils/include/constante.h"
#include "../utils/include/enum.hpp"
#include "../game/Entity.hpp"

// Librairies importées
#include <algorithm>
#include <memory>

class Entity;

using namespace std;

typedef struct STypeInstance {
    void* ptr;
    ETypeInstance instance;
} STypeInstance;

using namespace std;

/**
 * @brief Une entité a besoin d'un body pour fonctionner avec la physique de Box2d.
 * En fonction du type de body (static, kinematic et dynamic), ses interractions physiques sont différentes.
 */
class BodyFactory {
    public:
        static BodyFactory& getInstance(b2World* world);
        static BodyFactory& getInstance();

        b2Body* createStaticBody(float x,float y, float w, float h,float d, float f,float r,b2Shape* shape,uint8 category,void* object,ETypeInstance type,groupIndex index);
        b2Body* createKinematicBody(float x,float y, float w, float h,float d, float f,float r,b2Shape* shape,uint8 category,void* object,ETypeInstance type,groupIndex index);
        b2Body* createDynamicBody(float x,float y, float w, float h,float d, float f,float r,b2Shape* shape,uint8 category,void* object,ETypeInstance type,groupIndex index);
        void addFixture(b2Body* body,float d, float f,b2Shape* shape,float r = 0);
        void setupBorder(int widthMap,int heightMap);
        void addSensor(b2Body* body, float d, float f, b2Shape* shape,uint16 mask,void* object ,ETypeInstance type,groupIndex index,float r = 0);
        void destroyBody(b2Body* body){
            world->DestroyBody(body);
        }
        
    private:
        BodyFactory(b2World* world) : world(world){}

        static BodyFactory* instance;

        b2World* world;  
};

#endif