#include "../../include/manager/BodyFactory.hpp"

BodyFactory* BodyFactory::instance = nullptr;

BodyFactory& BodyFactory::getInstance(b2World* world)
{
    if (!instance) {
        instance = new BodyFactory(world);
    }
    return *instance;
}

BodyFactory& BodyFactory::getInstance()
{
    if (!instance) {
        cerr << "Trying to get instance without initialising it first" << endl;
        exit(1);
    }
    return *instance;
}

b2Body* BodyFactory::createStaticBody(float x,float y, float w, float h,float d, float f,float r,b2Shape* shape,uint8 category,void* object,ETypeInstance type,groupIndex index){
  b2BodyDef bodyDef;
  bodyDef.type = b2_staticBody;
  bodyDef.position.Set(x, y);
  b2Body* body = (*world).CreateBody(&bodyDef);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = shape;
  fixtureDef.density = d;
  fixtureDef.friction = f;
  fixtureDef.restitution = r;
  fixtureDef.filter.categoryBits = category;
  //  fixtureDef.filter.groupIndex = index;
  auto userData = new STypeInstance();
    userData->ptr = object;
    userData->instance = type;
    fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(userData);
    
  body->SetFixedRotation(true);
  body->CreateFixture(&fixtureDef);
  return body;
}

b2Body* BodyFactory::createKinematicBody(float x,float y, float w, float h,float d, float f,float r,b2Shape* shape,uint8 category,void* object,ETypeInstance type,groupIndex index){
    b2BodyDef bodyDef;
    bodyDef.type = b2_kinematicBody;
    bodyDef.position.Set(x, y);
    b2Body* body = (*world).CreateBody(&bodyDef);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = shape;
    fixtureDef.density = d;
    fixtureDef.friction = f;
    fixtureDef.friction = r;
    fixtureDef.filter.categoryBits = category;
    //fixtureDef.filter.groupIndex = index;

    auto userData = new STypeInstance();
    userData->ptr = object;
    userData->instance = type;
    fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(userData);
    
    body->SetFixedRotation(true);
    body->CreateFixture(&fixtureDef);
    
    return body;
}

b2Body* BodyFactory::createDynamicBody(float x,float y, float w, float h,float d, float f,float r,b2Shape* shape,uint8 category,void* object,ETypeInstance type,groupIndex index){
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(x, y);
    b2Body* body = (*world).CreateBody(&bodyDef);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = shape;
    fixtureDef.density = d;
    fixtureDef.friction = f;
    fixtureDef.restitution = r;
    
    auto userData = new STypeInstance();
    userData->ptr = object;
    userData->instance = type;
    fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(userData);
    fixtureDef.filter.categoryBits = category;
    fixtureDef.filter.groupIndex = index;
    body->SetFixedRotation(true);
    body->CreateFixture(&fixtureDef);
    body->SetLinearDamping(2.0f);
    return body;
}

/**
 * @brief Rajoute une fixture qui défini le body.
 * @param body le body à définir.
 * @param d la densité.
 * @param f le facteur de friction.
 * @param shape la forme géométrique.
 * @param r l'élasticité [0,1].
 */
void BodyFactory::addFixture(b2Body* body,float d, float f,b2Shape* shape,float r){
  b2FixtureDef fixtureDef;
  fixtureDef.shape = shape;
  fixtureDef.density = d;
  fixtureDef.friction = f;
  fixtureDef.restitution = r;
  fixtureDef.isSensor=true;
  body->CreateFixture(&fixtureDef);
}

void BodyFactory::setupBorder(int widthMap,int heightMap){
 float widthMapInMeter = widthMap/PPM;
  float heightMapInMeter = heightMap/PPM;
  b2BodyDef bodyDef;
  bodyDef.type = b2_staticBody;
  bodyDef.position.Set(widthMapInMeter/2.0f, heightMapInMeter/2.0f);
  b2Body* body = (*world).CreateBody(&bodyDef);
  
  b2FixtureDef fixtureDef;
  b2PolygonShape murGauche;
  murGauche.SetAsBox(1/PPM,heightMapInMeter/2.0f,b2Vec2(-widthMapInMeter/2.0f,0.0f),0.0f);
  fixtureDef.shape = &murGauche;
  fixtureDef.density = 1.0f;
  fixtureDef.friction = 1.0f;
  fixtureDef.filter.categoryBits = BORDERCATEGORY;
  fixtureDef.filter.groupIndex = BORDERINDEX;
  
  body->CreateFixture(&fixtureDef);

  b2FixtureDef fixtureDef2;
  b2PolygonShape murDroit;
  murDroit.SetAsBox(1/PPM,heightMapInMeter/2.0f,b2Vec2(widthMapInMeter/2.0f,0.0f),0.0f);
  fixtureDef2.shape=&murDroit;
  fixtureDef2.density = 1.0f;
  fixtureDef2.friction =1.0f;
  fixtureDef2.filter.categoryBits = BORDERCATEGORY;
  fixtureDef2.filter.groupIndex = BORDERINDEX;

  
  body->CreateFixture(&fixtureDef2);
  
  b2FixtureDef fixtureDef3;
  b2PolygonShape murBas;
  murBas.SetAsBox(widthMapInMeter/2.0f,1/PPM,b2Vec2(0.0f,heightMapInMeter/2.0f),0.0f);
  fixtureDef3.shape=&murBas;
  fixtureDef3.density = 1.0f;
  fixtureDef3.friction =1.0f;
  fixtureDef3.filter.categoryBits = BORDERCATEGORY;
   fixtureDef3.filter.groupIndex = BORDERINDEX;
  
  body->CreateFixture(&fixtureDef3);
  
  b2FixtureDef fixtureDef4;
  b2PolygonShape murHaut;
  murHaut.SetAsBox(widthMapInMeter/2.0f,1/PPM,b2Vec2(0.0f,-heightMapInMeter/2.0f),0.0f);
  fixtureDef4.shape=&murHaut;
  fixtureDef4.density = 1.0f;
  fixtureDef4.friction =1.0f;
  fixtureDef4.filter.categoryBits = BORDERCATEGORY;
   fixtureDef4.filter.groupIndex = BORDERINDEX;
  body->CreateFixture(&fixtureDef4);
}

void BodyFactory::addSensor(b2Body* body, float d, float f, b2Shape* shape, uint16 mask,void* object,ETypeInstance type,groupIndex index,float r){
    b2FixtureDef sensor;
    sensor.isSensor = true;
    sensor.density = d;
    sensor.friction = f;
    sensor.restitution = r;
    sensor.filter.maskBits = mask;
    auto userData = new STypeInstance();
    userData->ptr = object;
    userData->instance = type;
    sensor.userData.pointer = reinterpret_cast<uintptr_t>(userData);
    sensor.filter.groupIndex = index;
    sensor.shape=shape;
    body->CreateFixture(&sensor);
}