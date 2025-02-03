#include "../../include/game/Entity.hpp"

Entity::Entity(float x,float y, float w, float h,float d, float f,float r,b2Shape* shape,BodyType type,void* object,uint8 category,ETypeInstance typeInstance,groupIndex index) 
{
    BodyFactory bf = BodyFactory::getInstance();
    this->w = w; 
    this->h = h;
    Entity* entity = this;
    switch(type){
        case Static: 
            body = bf.createStaticBody(x,y,w,h,d,f,r,shape,category,object,typeInstance,index);
            break;
        case Dynamic: 
            body = bf.createDynamicBody(x,y,w,h,d,f,r,shape,category,object,typeInstance,index);
            break;
        case Kinematic: 
            body = bf.createKinematicBody(x,y,w,h,d,f,r,shape,category,object,typeInstance,index);
            break;
    }
}

Entity::~Entity()
{
    
}   