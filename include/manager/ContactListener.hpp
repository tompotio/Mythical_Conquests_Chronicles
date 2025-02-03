#pragma ONCE
#ifndef CONTACT_LISTENER_H
#define CONTACT_LISTENER_H

#include "stdio.h"
#include "../../libs/Box2d/include/box2d/box2d.h"
#include "../game/Entity.hpp"

class ContactListener : public b2ContactListener
{
    public:
        ContactListener(float* deltaT){
            this->deltaT = deltaT;
        }

        void BeginContact(b2Contact* contact)
        { 
            
            b2Fixture* fixtureA = contact->GetFixtureA();
            b2Fixture* fixtureB = contact->GetFixtureB();
            if(!fixtureA->GetUserData().pointer  || !fixtureB->GetUserData().pointer ){
                return;
            }
            if(!fixtureA->IsSensor() && !fixtureB->IsSensor()){
                return;
            }
            if(fixtureA->IsSensor() && fixtureB->IsSensor()){
                return;
            }
            // Entity entityA = *((Entity*)(fixtureA->GetUserData().pointer));
            // Entity entityB = *((Entity*)(fixtureB->GetUserData().pointer));
            STypeInstance* instanceA = (STypeInstance*)(fixtureA->GetUserData().pointer);
            STypeInstance* instanceB = (STypeInstance*)(fixtureB->GetUserData().pointer);
            /*if(instanceA->instance == TILESTATIQUE){
                cout << "hello" << endl;
                //((Tile*)(instanceA->ptr))->BeginContact(contact,*deltaT);
                if(instanceA->ptr){
                    cout << "c'est nul" << endl;
                }
            }*/
            if(fixtureA->GetFilterData().categoryBits!=BORDERCATEGORY && instanceA->instance!=TILESTATIQUE){
                ((Entity*)(instanceA->ptr))->BeginContact(contact,*deltaT);
            }
            if(fixtureB->GetFilterData().categoryBits!=BORDERCATEGORY && instanceB->instance!=TILESTATIQUE){
                ((Entity*)(instanceB->ptr))->BeginContact(contact,*deltaT);
            }
            //((Entity*)(fixtureB->GetUserData().pointer))->onTouch(contact,*deltaT);
            
        }
        
        void EndContact(b2Contact* contact)
        { 

            b2Fixture* fixtureA = contact->GetFixtureA();
            b2Fixture* fixtureB = contact->GetFixtureB();
            if(!fixtureA->GetUserData().pointer  || !fixtureB->GetUserData().pointer ){
                return;
            }
            if(!fixtureA->IsSensor() && !fixtureB->IsSensor()){
                return;
            }

            STypeInstance* instanceA = (STypeInstance*)(fixtureA->GetUserData().pointer);
            STypeInstance* instanceB = (STypeInstance*)(fixtureB->GetUserData().pointer);
            
            if(fixtureB->GetFilterData().categoryBits!=BORDERCATEGORY && instanceA->instance!=TILESTATIQUE){
                static_cast<Entity*>(instanceA->ptr)->EndContact(contact,*deltaT);
            }

            if(fixtureB->GetFilterData().categoryBits!=BORDERCATEGORY && instanceB->instance!=TILESTATIQUE){
                static_cast<Entity*>(instanceB->ptr)->EndContact(contact,*deltaT);
            }

            //((Entity*)(fixtureB->GetUserData().pointer))->onTouch(contact,*deltaT);
         }
        
        void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
        { /* handle pre-solve event */ }
        
        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
        { /* handle post-solve event */ }
        
    private:
        float* deltaT;
};

#endif
