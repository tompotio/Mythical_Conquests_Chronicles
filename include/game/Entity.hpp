#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include "../manager/RenderManager.hpp"
#include "../manager/BodyFactory.hpp"
#include "Animation.hpp"

class Animation;
class BodyFactory;

class Entity {
    public:
        Entity(float x,float y, float w, float h,float d, float f,float r,b2Shape* shape,BodyType type,void* object,uint8 category,ETypeInstance typeInstance,groupIndex index);
        ~Entity();

        /**
         * @brief Rajoute une animation avec une vitesse.
         */
        void addAnimation(int state, float speed){ animations.emplace(state,speed); }

        /**
         * @brief Rajoute une animation avec un Timer préfait.
         */
        void addAnimation(int state, Timer* timer){ animations.emplace(state,timer); }
        void resetAnimation(EntityState state){animations[state].reset();}

        void setPosition(int x, int y){body->SetTransform(Vec2(x,y),body->GetAngle());}

        /**
         * @brief Renvoie la largeur en mètres.
         */
        float getWidth(){return w;}

        /**
         * @brief Renvoie la hauteur en mètres.
         */
        float getHeight(){return h;}

        Vec2 getSize(){return Vec2(w,h);}
        Vec2 getPosition(){return body->GetPosition();}

        b2Body* getBody(){
            return body;
        }
        
        Animation& getAnimation(int state){return this->animations[state];}
        /**
         * @brief Avec cette fonction on peut savoir si l'entité a au moins une texture.
         * Si elle n'en a pas, la méthode de render qui appelle l'entité, n'a pas besoin de l'afficher.
         * De cette manière, on peut générer par exemple des entités invisibles.
         */
        bool hasAnimation(){return animations.size() > 0;}

        virtual void update(float deltaT)=0;
        virtual void onTouch(b2Contact* contact,float deltaT)=0;
        virtual void BeginContact(b2Contact* contact,float deltaT)=0;
        virtual void EndContact(b2Contact* contact,float deltaT)=0;

    protected:
        b2Body* body;
        map<int, Animation> animations;

        float w;
        float h; 
};

#endif