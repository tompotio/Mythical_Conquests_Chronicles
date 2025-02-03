#include "../../include/game/GameEntities.hpp"

/** ---------------------------------------------------- [Being] -------------------------------------------
 * Ici on gère les fonctions de being.
 */

Being::Being(float x,float y, float w, float h,float d, float f,float r,b2Shape* shape,BodyType type,void* object,uint8 category,ETypeInstance typeInstance,groupIndex index,WorldManager* parent) : Entity(x,y,w,h,d,f,r,shape,type,object,category,typeInstance,index) {
    this->parent = parent;
    this->world = parent->getWorld();
};

/**
 * @brief Positionne le being à un endroit spécifique.
 * 
 * @param x 
 * @param y 
 */
void Being::MoveTo(float x, float y){
    this->body->SetTransform(Vec2(x,y),0);
}

/**
 * @brief Applique une vitesse au being.
 * 
 * @param x 
 * @param y 
 */
void Being::Move(float x,float y){
    this->body->SetLinearVelocity(Vec2(x,y));
}

/** ---------------------------------------------------- [PLAYER] -------------------------------------------
 * Ici on gère les fonctions du joueur.
 */

Player::Player(float x, float y,b2Shape* shape, WorldManager* parent,int lvl) : Being(x, y, PLAYERSIZE_W / PPM, PLAYERSIZE_H / PPM, 10.0f, 1.0f,0.0f, shape, Dynamic,this,PLAYERCATEGORY,PLAYERTYPE,PLAYERINDEX,parent)
{
    skills[HSlashSkill] = std::make_unique<HSlash>(this,world);
    this->parent = parent;
    this->world = parent->getWorld();
    this->currHp = 100;
    this->maxHp = 100;
    this->acceleration = 1.7f;
    this->niveau = lvl;
    this->strength = 20;
    this->magic = 10;
    this->dexterity = 15;
    this->luck = 2;
    this->armor = 10;
    //this->timerHslash.setEnded(true);
    this->timerHslash.setEnd(2.0f);    
    this->items["lifePotion"]=make_unique<LifePotion>();
    

    /* [Ajout de textures pour le joueur] --------------------------------------------------------------------- 
     * Étant donné que l'énum BeingState associe une énumération à un nombre.
     * Il nous suffit de crée les Animations dans l'ordre. De sorte que leur index, dans le vector d'animations de l'instance du joueur,
     * corresponde au numéro de leur énumération.
     *
     * Exemple : Idle = 0, alors on ajoute l'animation idle en première (indexe 0).
     * Walking = 1, donc on rajoute l'animation walking en second (indexe 1).
     */

    // Idle
    const string idle = "../assets/player/IdleFighter.png";
    parent->addAnimationToEntity(this, idle, 6, PLAYERSIZE_H, 42, IdleState, 0.35f);

    // Walking
    const string walking = "../assets/player/WalkingFighter.png";
    parent->addAnimationToEntity(this, walking, 8, PLAYERSIZE_H, PLAYERSIZE_H, WalkingState, 0.1f);

    // Running
    const string running = "../assets/player/RunFighter.png";
    parent->addAnimationToEntity(this, running, 6, PLAYERSIZE_H, PLAYERSIZE_H, RunningState, 0.1f);

    // Slashing
    const string slashing = "../assets/player/FighterSlashAttack.png";
    parent->addAnimationToEntity(this, slashing, 4, PLAYERSIZE_H, PLAYERSIZE_H * 2, HSlashState, 0.2f);
    this->getAnimation(HSlashState).setLoop(false);

    RenderManager& renderManager = RenderManager::getInstance();
    directionArrow = renderManager.loadTexture("../assets/player/directionArrow.png");
    renderManager.getTextureSize(directionArrow, &widthArrow, &heightArrow);
}

void Player::update(float deltaT)
{
    Being::update(deltaT);
    timerHslash.update(deltaT);
    if(invicible){
        timer.update(deltaT);
        if(timer.getEnded()){
            invicible=false;
            //b2Fixture* fixture = &(body->GetFixtureList()[0]);
            //fixture->SetSensor(false);
            return;
        }
        //si les deux composantes du vecteur sont à zéro alors c'est que l'animation est terminée
        if(vectCollide.x!=0.0f || vectCollide.y != 0.0f){
            timerKnockback.update(deltaT);
            if(!timerKnockback.getEnded()){
                Vec2 vect(100.0f,100.0f);
                getBody()->ApplyLinearImpulse(vectCollide,this->getBody()->GetLocalCenter(),true);
            }
            else{
                vectCollide.SetZero();
            }
        }
    }
    if(this->state==WalkingState || this->state == RunningState){
        SoundManager::getInstance().playWalkingGrass();
    }
}

void Player::gainXp(int exp){
    
    this->exp+=exp;
    cout << "j'ai"<< this->exp << "xp" << endl;
    cout << "j'ai besoin de "<< this->capExp << "xp" << endl;
    while(this->exp >= this->capExp){
        cout << "level up" << endl;
        this->exp -= capExp;
        this->niveau ++;
        capExp = 100 *pow((niveau*2),1.5);
        this->strength +=5;
        this->armor +=5;
        this->magic +=5;
        this->luck +=1;
    }
    cout << this ->exp << endl;
}

void Player::onTouch(b2Contact* contact,float deltaT)
{
    //cout << "J'ai touché " << endl;
}

bool Player::Deserialize(const rapidjson::Document& obj){
    this->name = obj["character"]["name"].GetString();
    this->niveau = obj["character"]["lvl"].GetInt();
    this->exp = obj["character"]["experience"].GetInt();
    this->statPoint = obj["character"]["statsPoints"].GetInt();
    this->strength = obj["character"]["statsPoints"].GetInt();
    this->magic = obj["character"]["magic"].GetInt();
    this->luck = obj["character"]["luck"].GetInt();
    this->dexterity = obj["character"]["luck"].GetInt();
    this->armor = obj["character"]["armor"].GetInt();
    return true;
}

bool Player::Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer){
   
    writer->StartObject();
        writer->Key("character");
        writer->StartObject();
        writer->Key("name");
        writer->String(this->getName().c_str());
        writer->Key("lvl");
        writer->Int(this->niveau);
        writer->Key("experience");
        writer->Int(this->exp);
        writer->Key("statsPoints");
        writer->Int(this->statPoint);
        writer->Key("strength");
        writer->Int(this->getStrength());
        writer->Key("magic");
        writer->Int(this->magic);
        writer->Key("dexterity");
        writer->Int(this->dexterity);
        writer->Key("luck");
        writer->Int(this->luck);
        writer->Key("armor");
        writer->Int(this->armor);
        writer->EndObject();
    writer->EndObject();
    
    return true;
}

/**
 * @brief Applique un skill. 
 * 
 * @param skill 
 * @return true Le skill a pu se lancer.
 * @return false Échec de l'envoi.
 */
bool Player::applySkill(const PSkill& skill){
    const bool& val = skills[skill]->use();
    if(val) currentSkill = skill;
    return val;
}

/**
 * @brief Permet de savoir si le skill utilisé s'est terminé.
 * 
 * @return true 
 * @return false 
 */
bool Player::isSkillFinished(){
    return skills[currentSkill]->isFinished();
}

void Player::setAngle(float angle){
    this->angle = angle;
}

float Player::getAngle(){
    return angle;
}

void Player::getHit(STypeInstance* instance){
    if(!invicible){
            Enemy* enemy = ((Enemy*)(instance->ptr));
            Vec2 directionEnemy = enemy->getBody()->GetLinearVelocity();
            directionEnemy.Normalize();
            directionEnemy.operator*=(this->getWalkSpeed()/2);
            vectCollide.x=directionEnemy.x;
            vectCollide.y=directionEnemy.y;
            invicible = true;
            timer.reset();
            timer.setEnd(1.0f);
            timerKnockback.reset();
            timerKnockback.setEnd(0.2f);
           //b2Fixture* fixture =   &(body->GetFixtureList()[0]);
            //fixture->SetSensor(true);
            this->currHp -= enemy->getStrength() * (enemy->getStrength())/(enemy->getStrength()+this->armor);
   
        }
}
void Player::BeginContact(b2Contact* contact,float deltaT) {     
    
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    STypeInstance* instanceA = (STypeInstance*)(fixtureA->GetUserData().pointer);
    STypeInstance* instanceB = (STypeInstance*)(fixtureB->GetUserData().pointer);
    switch (instanceA->instance)
    {
    case KNOCKBACKTYPE:
        this->getHit(instanceA);
        break;
   
    }
    switch (instanceB->instance)
    {
    case KNOCKBACKTYPE:
        this->getHit(instanceB);
        
        break;
    }
}

void Player::EndContact(b2Contact* contact,float deltaT) {

}

/** ---------------------------------------------------- [ENEMY] -------------------------------------------
 * Ici on gère les fonctions de l'ennemi.
 */
Enemy::Enemy(float x, float y, int w, int h,int expGift,int lvl, b2Shape* shape, WorldManager* parent) : Being(x, y, w / PPM, h / PPM, 1.0f, 1.0f,0.0f, shape, Dynamic,this,ENNEMYCATEGORY,ENEMYTYPE,ENEMYINDEX,parent){
    this->parent = parent;
    this->world = parent->getWorld();

    timerEtatPassif.setEnd(1);

    b2PolygonShape shapeSensor;
    shapeSensor.SetAsBox(
        PLAYERSIZE*1.5f/(PPM*2.0f),
        PLAYERSIZE*1.5f/(PPM*2.0f),
        Vec2(PLAYERSIZE_W/(2.0f*PPM),PLAYERSIZE/(PPM*2.0f)),
        0.0f
    );
    armor = 10*lvl;
    force = 2*lvl;
    this->expGift = expGift;
    this->lvl = lvl;
    b2CircleShape sensorPlayer;
    sensorPlayer.m_p.Set(getBody()->GetLocalCenter().x,getBody()->GetLocalCenter().y);
    sensorPlayer.m_radius = 6.0f;
    this->expGift = expGift;
    BodyFactory::getInstance().addSensor(this->getBody(),0.0f,1.0f,&sensorPlayer,PLAYERCATEGORY,this,TARGETTYPE,ENEMYINDEX);
    BodyFactory::getInstance().addSensor(this->getBody(),0.0f,1.0f,shape,PLAYERCATEGORY,this,KNOCKBACKTYPE,ENEMYINDEX);
    BodyFactory::getInstance().addSensor(this->getBody(),0.0f,1.0f,shape,BORDERCATEGORY,this,KNOCKBACKTYPE,ENEMYINDEX);
    
}

void Enemy::changeDirection(float deltaT){

}

void Enemy::BeginContact(b2Contact* contact,float deltaT) {
    
   
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    STypeInstance* instanceA = (STypeInstance*)(fixtureA->GetUserData().pointer);
    STypeInstance* instanceB = (STypeInstance*)(fixtureB->GetUserData().pointer);

    switch (instanceA->instance)
    {
        case TARGETTYPE:
        {   
            //cout << instanceB->instance << endl;
            this->potentialTarget.push_back(((Player*)instanceB->ptr));
            break;
        }
        case KNOCKBACKTYPE:
        {       
           
           
                switch(instanceB->instance){
                    case PLAYERTYPE:
                    {
                       
                            Player* player = ((Player*)instanceB->ptr);
                             if(!player->getInvicible()){

                            //player->setHp(player->getHp()-10);
                            //printf("player hp : %d",player->getHp());
                            }
                            break;
                                      
                    }
                    
                }
            
        }
        default:
            break;
        }
        switch (instanceB->instance)
        {
        case TARGETTYPE:
            
            this->potentialTarget.push_back(((Player*)instanceA->ptr));
            break;
        case KNOCKBACKTYPE:
            {
                switch(instanceA->instance){
                    case PLAYERTYPE:
                    {
                            
                            Player* player = ((Player*)instanceB->ptr);
                            if(!player->getInvicible()){

                            //player->setHp(player->getHp()-10);
                            //printf("player hp : %d",player->getHp());
                            }
                            break;
                                      
                    }       
                }
           }
        default:
            break;
    }
}

void Enemy::update(float deltaT){
    Being::update(deltaT);
    if(this->knockback){
       

        getBody()->ApplyLinearImpulse(reverseKnockback,this->getBody()->GetLocalCenter(),true);
        this->knockback = false;
    }
    if(this->currHp<=0){

        this->active = false;
        return;
    }
    if(cible){
        Vec2 direction = Vec2(cible->getBody()->GetPosition().x,cible->getBody()->GetPosition().y);
        direction.operator -= (this->getBody()->GetPosition());
        if(direction.Length() >= maxAggro){
            this->state= IdleState;
            cible = nullptr;
            return;
        }
    }
   
    (velocity.x >= 0) ? simpleLook = Right : simpleLook = Left;
}

void Enemy::lookForTarget(){
    if(!this->cible){
        if(potentialTarget.size()!=0){ 
            cible = (potentialTarget.back());
            float distance_min = fabs(b2Distance(this->getBody()->GetPosition(),cible->getBody()->GetPosition()));
            float distance;
            for(Player* player:potentialTarget){
                distance = fabs(b2Distance(this->getBody()->GetPosition(),player->getBody()->GetPosition()));
                if(distance_min>distance){
                    distance =distance_min;
                    cible = player;
                }            
            }  
        }
    }
}
void Enemy::acceptSkill(Skill* skill){cout << "wesh" <<endl; skill->applySkillEnemy(this); }
void Enemy::followTarget(float deltaT){
        Vec2 direction = Vec2(cible->getBody()->GetPosition().x,cible->getBody()->GetPosition().y);
        direction.operator -= (this->getBody()->GetPosition());
        
        if(direction.Length() >= maxAggro){
            cible = nullptr;
            return;
        }

        direction.Normalize();
        direction *= walkspeed * acceleration * deltaT;
        Vec2 position = this->getBody()->GetPosition();
        Vec2 point[8];
        this->rayCastCallback.setHit(false);
        point[0] = Vec2(position.x,position.y);
        point[1] = Vec2(position.x+this->getWidth(),position.y);
        point[2] = Vec2(position.x,position.y+this->getHeight());
        point[3] = Vec2(position.x+this->getWidth(),position.y+this->getHeight());
        point[4] = Vec2(position.x,position.y+this->getHeight()/2.0f);
        point[5] = Vec2(position.x+this->getWidth(),position.y+this->getHeight()/2.0f);
        point[6] = Vec2(position.x+this->getWidth()/2.0f,position.y);
        point[7] = Vec2(position.x+this->getWidth()/2.0f,position.y+this->getHeight());

        for(int i = 0; i < 8; i++){
            Vec2 normTest(direction);
            normTest.operator*=(0.8f);
            Vec2 target(point[i]);
            target.x += normTest.x;
            target.y += normTest.y;
            world->RayCast(&(this->rayCastCallback),point[i],target);

        }

        if(rayCastCallback.getHit()){
           
            // On ajuste la direction.
            float angle = (90 * M_PI) / 180.0;
            double newX = direction.x * cos(angle) - direction.y * sin(angle);
            double newY = direction.x * sin(angle) + direction.y * cos(angle);
            direction.x = newX;
            direction.y = newY;
        }
        else{
            
        }
        
        velocity = direction;
    }


void Enemy::onTouch(b2Contact* contact,float deltaT){
    
}

// [Skeleton] ----------------------------------------------------------------------------------------------

// NB : Comme il y aura plusieurs classes d'ennemis, faire un vector de pointeurs intelligents.

Skeleton::Skeleton(float x, float y, int w, int h,int xpGift,int lvl, b2Shape* shape, WorldManager* parent) : Enemy(x, y, w, h,xpGift,lvl,shape, parent){
    currHp = 100;
    maxHp = 100;
    armor = 0;
    acceleration = 1.4f;
    maxAggro = 10;
    
    // Idle
    const string idle = "../assets/enemies/SkeletonWarrior/IdleSkeleton.png";
    parent->addAnimationToEntity(this, idle, 7, 60, 54, IdleState, 0.35f);

    // Walking
    const string walking = "../assets/enemies/SkeletonWarrior/WalkingSkeleton.png";
    parent->addAnimationToEntity(this, walking,7,65,128,WalkingState, 0.1f);

    // Running
    const string running = "../assets/enemies/SkeletonWarrior/RunningSkeleton.png";
    parent->addAnimationToEntity(this, running, 8, 60, 128, RunningState, 0.1f);

    // Running Plus attacking
    const string runPattack = "../assets/enemies/SkeletonWarrior/Run+attack.png";
    parent->addAnimationToEntity(this,runPattack,7, 69, 128, RunPlusAttackState, 0.1f);
    animations[RunPlusAttackState].setLoop(false);
    attackCoolDown.setEnd(animations[RunPlusAttackState].getNbFrames() * 0.1f + 1);

}

void Skeleton::update(float deltaT){
    Enemy::update(deltaT);
    timerEtatPassif.update(deltaT);
    attackCoolDown.update(deltaT);

    velocity.x = 0; velocity.y = 0;

    // L'ennemi court s'il a trouvé une cible. Sinon si la cible a été perdu de vue, il s'arrête.    
    if (cible) {
        goto court;
    } 
    
    else {
        if(state == RunningState) state = IdleState;
        lookForTarget();
    }

    // Si l'ennemi est en Idle, génère une probabilité de 1/4 qu'il se déplace vers une position aléatoire autour de lui.
    if(state == IdleState){
        // Regarde d'abord si le chrono de l'état passif est terminé. 
        if(timerEtatPassif.getEnded()){
            
            const int randomValue = (rand() % 3) + 1;

            // Si = 1 génère une destination aléatoire.
            if(randomValue == 1){
                this->state = WalkingState;

                double randomDoubleX = static_cast<double>(rand()) / RAND_MAX;
                double x = randomDoubleX * 100.0;
                double randomDoubleY = static_cast<double>(rand()) / RAND_MAX;
                double y =  randomDoubleY * 100.0;

                destination = Vec2(x, y);
             }
        }
    }
    
    // Si l'ennemi marche, on met à jour les calculs de déplacement.
    if(state == WalkingState){
        Vec2 currentPosition = body->GetPosition();
        Vec2 distanceToTarget = destination - currentPosition;
        Vec2 position = this->getBody()->GetPosition();
        Vec2 point[8];
        
        this->rayCastCallback.setHit(false);
        point[0] = Vec2(position.x,position.y);
        point[1] = Vec2(position.x+this->getWidth(),position.y);
        point[2] = Vec2(position.x,position.y+this->getHeight());
        point[3] = Vec2(position.x+this->getWidth(),position.y+this->getHeight());
        point[4] = Vec2(position.x,position.y+this->getHeight()/2.0f);
        point[5] = Vec2(position.x+this->getWidth(),position.y+this->getHeight()/2.0f);
        point[6] = Vec2(position.x+this->getWidth()/2.0f,position.y);
        point[7] = Vec2(position.x+this->getWidth()/2.0f,position.y+this->getHeight());
        Vec2 direction = distanceToTarget;
        direction.Normalize();
        for(int i = 0; i < 8; i++){
            Vec2 normTest(direction);
            normTest.operator*=(0.8f);
            Vec2 target(point[i]);
            target.x += normTest.x;
            target.y += normTest.y;
            world->RayCast(&(this->rayCastCallback),point[i],target);
        }
        // Arrête le mouvement lorsque l'objet est proche de la cible.
        if (distanceToTarget.Length() < tolerance || this->rayCastCallback.getHit()) {
            velocity.x = 0; velocity.y = 0;
            state = IdleState;
            timerEtatPassif.reset();
            timerEtatPassif.setEnd(rand() % 3 + 1 * 5.0f);
        }

        // Continue de se diriger vers la cible.
        else {
            
            
            velocity = walkspeed * deltaT * direction;
        }
    }

    goto fin;

    // Court vers le joueur.
    court:
    {   
        if(state != RunPlusAttackState) state = RunningState;

        // Si l'ennemi n'attaque plus.
        if(state == RunPlusAttackState && animations[RunPlusAttackState].isFinished()){
            animations[RunPlusAttackState].reset();
            state = RunningState;
        }

        followTarget(deltaT);

        Vec2 targetPosition = cible->getBody()->GetPosition();
        Vec2 currentPosition = this->getBody()->GetPosition();
        float distance = b2Distance(targetPosition,currentPosition);

        if (distance <= range) {
            // Check s'il n'a pas encore attaqué et qu'il peut le faire. 
            if(state != RunPlusAttackState){
                if(attackCoolDown.getEnded()){
                   // cout << "est en train d'attaquer" << endl;
                    attackCoolDown.reset();
                    state = RunPlusAttackState;
                }
            }

            // Sinon c'est qu'il touche le joueur.
            else {
                if(animations[RunPlusAttackState].getFrame()== 5){
                   STypeInstance instance;
                   instance.instance=ENEMYTYPE;
                   instance.ptr = this;
                   this->cible->getHit(&instance);
                   SoundManager::getInstance().playMonsterSlash();
                }
            }
        }
    }
    
    fin:

    body->SetLinearVelocity(velocity);
}

// [SkeletonArcher] ----------------------------------------------------------------------------------------------

SkeletonArcher::SkeletonArcher(float x, float y, int w, int h,int expGift,int lvl, b2Shape* shape, WorldManager* parent) : Enemy(x, y, w, h,expGift,lvl,shape, parent) {
   currHp = 100;
    maxHp = 100;
    armor = 0;
    acceleration = 1.4f;
    
    // Idle
    const string idle = "../assets/enemies/Skeleton_Archer/Idle.png";
    parent->addAnimationToEntity(this, idle, 7, 128, 128, IdleState, 0.35f);

    // Walking
    const string walking = "../assets/enemies/Skeleton_Archer/Walk.png";
    parent->addAnimationToEntity(this, walking,8,128,128,RunningState, 0.1f);

    // Running
    /*const string running = "/assets/enemies/Skeleton_Archer/Shot_2.png";
    parent->addAnimationToEntity(this, running, 8, 60, 128, RunningState, 0.1f);*/

    // SHOOT
    const string shotAttack = "../assets/enemies/Skeleton_Archer/Shot_2.png";
    parent->addAnimationToEntity(this,shotAttack,15, 128, 128, SHOTSTATE, 0.05f);
    animations[SHOTSTATE].setLoop(false);
    
    
}

void SkeletonArcher::update(float deltaT){
    Enemy::update(deltaT);
    timerEtatPassif.update(deltaT);
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    
    switch (this->state)
        {
            case IdleState:{
                lookForTarget();
                if(this->cible != nullptr){
                     if(b2Distance(this->getBody()->GetPosition(),cible->getBody()->GetPosition())<this->attackRange && this->state != SHOTSTATE){
                        const int randomValue = (rand() % 100) + 1;
                        if(randomValue ==1 || randomValue == 2){
                            this->state = SHOTSTATE;
                        }
                        else{
                            this->state = IdleState;
                        }
                    }else{
                        this->state = RunningState;
                    }
                    //cout << "je me mets à courir" << endl;
                }
                break;
            }
            case SHOTSTATE:{
                //cout << "je vais tirer" << endl;
                if(this->animations[SHOTSTATE].isFinished()){
                    //cout << "je tire" << endl;
                    this->animations[SHOTSTATE].reset();
                    Vec2 positionCible = cible->getBody()->GetPosition();
                    positionCible.x+= cible->getWidth()/2;
                    positionCible.y+= cible->getHeight()/2;
                    Vec2 positionEnemy = getBody()->GetPosition();
                    positionEnemy.x+=getWidth()/2;
                    positionEnemy.y+=getHeight()/2;
                    float angle = ( 1 * atan2(positionCible.y-positionEnemy.y, positionCible.x-positionEnemy.x));
                    float xVelocity = 800.0f * cos(angle);
                    float yVelocity = 150.0f * sin(angle);
                    float xArrive = positionEnemy.x + (cos(angle));
                    float yArrive = positionEnemy.y + (sin(angle));
                    parent->makeProjectile(xArrive,yArrive,xVelocity,yVelocity,angle);
                    this->state = IdleState;
                    SoundManager::getInstance().playShootingBow(b2Distance(this->body->GetPosition(),this->cible->getBody()->GetPosition()));
                }
                break;
            }
            case RunningState:{
                if(this->cible){
                    if(b2Distance(this->getBody()->GetPosition(),cible->getBody()->GetPosition())>this->attackRange){
                        followTarget(deltaT);
                    } 
                     else{
                        this->state = IdleState;
                    }
                }
                else{
                    this->state = IdleState;
                }
               
               
            }
        }
    this->getBody()->SetLinearVelocity(velocity);
}

// [SkeletonSpearman] ----------------------------------------------------------------------------------------------

SkeletonSpearman::SkeletonSpearman(float x, float y, int w, int h,int expGift,int lvl, b2Shape* shape, WorldManager* parent) : Enemy(x, y, w, h,expGift,lvl, shape, parent) {

}

void SkeletonSpearman::update(float deltaT){
    Being::update(deltaT);
}


/** ---------------------------------------------------- [SKILLS] -------------------------------------------
 * Ici on gère les fonctions des skills.
 */

bool HSlash::use(){
    if(this->parent->getHslashTimer().getEnded()){
     
        this->parent->getHslashTimer().reset();
        HSlashRayCast callback = HSlashRayCast(this, 5);
        Vec2 pPos = parent->getPosition();
        Vec2 pSize = parent->getSize();

        const float& step = 2.5*pSize.x;
        int dir = 1;

        if(parent->getSimpleLook() == Left){
            dir = -1;
        }
        Vec2 point1(
            pPos.x + (pSize.x / 2.0f), 
            pPos.y + (pSize.y / 2.0f)
           
        );
        Vec2 point2(
            point1.x + step * dir, 
            point1.y
        );

        //cout << point1.x << " : " << point1.y << endl;
        //cout << "position joueur : " << pPos.x << " : " << pPos.y << endl;
        //cout << "position raycast p2 : " << point2.x << " : " << point2.y << endl;

        world->RayCast(&callback, point1, point2);

        return true;
    }  
      
    return false;                                                      
}
void HSlash::applySkillEnemy(Enemy* enemy){
    
    int degats;
    enemy->setKnockback(true);
    Vec2 reverseVelocity(enemy->getBody()->GetLinearVelocity());
    if(reverseVelocity.x == 0.0f && reverseVelocity.y == 0.0f){
        if(enemy->getSimpleLook() == Left){
            reverseVelocity.x += 1.0f;
        }
        else{
            reverseVelocity.x -= 1.0f;
        }
    }
    float angleToRadian = (90*M_PI)/180.0f;
    reverseVelocity.x = reverseVelocity.x * cos(angleToRadian) - sin(angleToRadian) * reverseVelocity.y;
    reverseVelocity.y = reverseVelocity.y * sin(angleToRadian) - cos(angleToRadian) * reverseVelocity.x;
    enemy->setKnockbackVelocity(reverseVelocity);
    if(this->parent->getStrength()*2<=enemy->getArmor()){
        degats=1;
    }
    else{
        degats = this->parent->getStrength() * ((this->parent->getStrength()-enemy->getArmor()/this->parent->getStrength()));
 
        
    }
    enemy->setHp(enemy->getHp()-degats);
    if(enemy->getHp()<=0){

        this->parent->gainXp(enemy->getGiftXp());
        int dropchance = (rand() % 100)+1;
        if(dropchance >=1 && dropchance<=20){
            int drop = (rand() % 5) +1;
            switch (drop)
            {
            case 1:
                {
                Helmet helmet(this->level);
                shared_ptr<Object> obj(&helmet);
                this->parent->addItemToInventory(obj);
                }
                break;
            case 2:
            {
                Chest chest(this->level);
                shared_ptr<Object> obj(&chest);
                this->parent->addItemToInventory(obj);
                break;
            }
            case 3:
            {
                Glove glove(this->level);
                shared_ptr<Object> obj(&glove);
                this->parent->addItemToInventory(obj);
                break;
            }
            case 4:
            {
                Boots boots(this->level);
                shared_ptr<Object> obj(&boots);
                this->parent->addItemToInventory(obj);
                break;
            }
            case 5:
            {
              /* LifePotion potion;
                shared_ptr<Item> obj(&potion);*/
                break;
            }
            }
            
        }
    }
}

Projectile::Projectile(float x,float y,b2Shape* shape,WorldManager* parent): Entity(x,y,48/(2*PPM),1/(2*PPM),1.0f,1.0f,1.0f,shape,Dynamic,this,PROJECTILECATEGORY,PROJECTILETYPE,PROJECTILEINDEX){

    b2Fixture* fixture = &(body->GetFixtureList()[0]); 
    fixture->SetSensor(true);                                                                          
    BodyFactory::getInstance().addSensor(this->getBody(),0.0f,0.0f,shape,TILECATEGORY,this,PROJECTILETYPE,PROJECTILEINDEX);
    BodyFactory::getInstance().addSensor(this->getBody(),0.0f,0.0f,shape,PLAYERCATEGORY,this,PROJECTILETYPE,PROJECTILEINDEX);
    BodyFactory::getInstance().addSensor(this->getBody(),0.0f,0.0f,shape,BORDERCATEGORY,this,PROJECTILETYPE,PROJECTILEINDEX);
    body->SetFixedRotation(false);
    

    this->parent = parent;
    const string idle = "../assets/enemies/Arrow.png";
    parent->addAnimationToEntity(this, idle, 6, 48, 48, IdleState, 0.35f);
}

void Projectile::update(float deltaT){
            vY +=9.81f*deltaT*1.5;
            body->SetTransform(body->GetPosition(),atan2(vY,vX));
            Vec2 velocite(vX,vY);
            velocite *= deltaT;
            body->SetLinearVelocity(velocite);
        
}

void Projectile::onTouch(b2Contact* contact,float deltaT){}
void Projectile::BeginContact(b2Contact* contact,float deltaT){
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    STypeInstance* instanceA = (STypeInstance*)(fixtureA->GetUserData().pointer);
    STypeInstance* instanceB = (STypeInstance*)(fixtureB->GetUserData().pointer);
    if(fixtureA->GetFilterData().categoryBits == BORDERCATEGORY ||fixtureB->GetFilterData().categoryBits == BORDERCATEGORY){
        active = false;
        return;
    }
    switch (instanceA->instance)
    {
    case PLAYERTYPE:
    {
        printf("je touche un joueur");
        Player* player = ((Player*)(instanceA->ptr));
        SoundManager::getInstance().playHitBow();
        if(player->getInvicible()){
            player->setHp(player->getHp()-1);
        }
        printf("je ne suis plus active");
        this->active = false;
        break;
    }
    case TILEDYNAMIQUE:
    {
        printf("je touche une tile");
        parent->getDeletedCache()->push_back(shared_from_this());
        active = false;
    }
    case BORDERCATEGORY:
        {
         
        active = false;
        }
    }
    switch (instanceB->instance)
    {
    case PLAYERTYPE:
    {
        printf("je touche un joueur");
        SoundManager::getInstance().playHitBow();
        Player* player = ((Player*)(instanceB->ptr));
        if(player->getInvicible()){
            player->setHp(player->getHp()-1);
        }
        this->active = false;
        break;
    }
    case TILEDYNAMIQUE:
    {
        printf("je touche une tile");
        this->active = false;
    }
    case BORDERCATEGORY:
    {
        active = false;
    }
    
    }
}

void Projectile::EndContact(b2Contact* contact,float deltaT){

}

/** ---------------------------------------------------- [Factory] -------------------------------------------
 * Ici on gère les fonctions de factory.
 */

EntityFactory* EntityFactory::instance = nullptr;

EntityFactory::EntityFactory(WorldManager* parent) : parent(parent){
    this->parent = parent;
} 

EntityFactory& EntityFactory::getInstance(WorldManager* parent)
{
    if (!instance) {
        instance = new EntityFactory(parent);
    }
    return *instance;
}

EntityFactory& EntityFactory::getInstance()
{
    if (!instance) {
        cerr << "Trying to get instance without initialising it first" << endl;
        exit(1);
    }
    return *instance;
}


void EntityFactory::makeSkeleton(float x,float y,float w,float h,int playerLvl){
    b2PolygonShape eshape;
    const float enemySize_w = w / PPM;
    const float enemySize_h = h / PPM;
    eshape.SetAsBox(
        enemySize_w/2.0f,
        enemySize_h/2.0f,
        Vec2(enemySize_w/2.0f,enemySize_h/2.0f),
        0.0f
    );
    int xpGift = playerLvl *100;
    std::shared_ptr<Enemy> skeleton = std::make_unique<Skeleton>(
        x,
        y,
        w,
        h,
        xpGift,
        playerLvl,
        &eshape,
        parent
    );
    parent->getEnemies()->push_back(skeleton);
}

void EntityFactory::makeArcherSkeleton(float x,float y,float w,float h,int playerLvl){
    b2PolygonShape eshape;
    const float enemySize_w = w / PPM;
    const float enemySize_h = h / PPM;
    eshape.SetAsBox(
       enemySize_w/2.0f,
        enemySize_h/2.0f,
        Vec2(enemySize_w/2.0f,(enemySize_h/2.0f)+32/PPM),
        0.0f
    );
    int xpGift = 100 * playerLvl;
    std::shared_ptr<Enemy> skeleton = std::make_unique<SkeletonArcher>(
        x,
        y,
        w,
        h,
        xpGift,
        playerLvl,
        &eshape,
        parent
    );
    parent->getEnemies()->push_back(skeleton);
}

void EntityFactory::makePlayer(float x,float y,int lvl){
    const float playerWidth = PLAYERSIZE_W / PPM;
    const float playerHeigth = PLAYERSIZE_H / PPM;
    b2PolygonShape shape;
    shape.SetAsBox(
        playerWidth/2.0f,
        playerHeigth/2.0f,
        Vec2(playerWidth/2.0f,playerHeigth/2.0f),
        0.0f
    );
    
    parent->getPlayers()->emplace_back(x,y,&shape,parent,lvl);      
}

void EntityFactory::makeProjectile(float x,float y,float vX,float vY,float angle){
    b2PolygonShape projectileShape;
    projectileShape.SetAsBox(48/(2*PPM),1/(2*PPM),Vec2(48/(2*PPM),1/(2*PPM)),0.0f);
     std::shared_ptr<Projectile> projectile = std::make_unique<Projectile>(x,y,&projectileShape,parent);
     projectile->getBody()->SetTransform(projectile->getBody()->GetPosition(),(angle));
     projectile->setvX(vX);
     projectile->setVy(vY);
     projectile->setAngle(angle);
     vX >= 0 ? projectile->setSimpleLook(Left):projectile->setSimpleLook(Right);
    this->parent->getProjectiles()->push_back(projectile);
}

Object::Object()
{

}

Object::~Object()
{

}

Item::Item()
{

}

Item::~Item()
{

}

Chest::Chest(int lvl)
{

}

void Chest::use(Player* player){

}
Chest::~Chest()
{
}

Glove::Glove(int lvl)
{

}
void Glove::use(Player* player){

}

Glove::~Glove()
{
    
}

Helmet::Helmet(int lvl)
{
}
void Helmet::use(Player* player){
    
}
Helmet::~Helmet()
{
}
Boots::Boots(int lvl){

}
Boots::~Boots(){

}
void Boots::use(Player* player){

}
LifePotion::LifePotion():Item()
{
}

LifePotion::~LifePotion()
{
}
void LifePotion::use(Player* player){
    if(amount>0){
        player->setHp(player->getHp()+this->healedHP);
    }
}
void LifePotion::increaseAmount(){
    amount++;
}
Sword::Sword(int lvl){

}
Sword::~Sword(){

}
void Sword::use(Player* player){

}
