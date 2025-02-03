#include "../../include/manager/SoundManager.hpp"

SoundManager* SoundManager::instance = nullptr;

SoundManager& SoundManager::getInstance(){
    if(!instance){
        instance = new SoundManager();
    }
    return *instance;
}

void SoundManager::PlayMusicBackground(){
    Mix_Volume(0,25);
    int channel = Mix_PlayChannel(0,this->backgroundMusic,-1);
}

void SoundManager::playMenuMusic(){
    Mix_Volume(0,25);
    int channel = Mix_PlayChannel(0,this->menuMusic,-1);
}

void SoundManager::freeMusic(){
    Mix_FreeChunk(this->backgroundMusic);
    Mix_FreeChunk(this->shootingBow);
    Mix_FreeChunk(this->WalkingSkeleton);
    Mix_FreeChunk(this->hitBow);
    Mix_FreeChunk(this->walkingGrass);
    Mix_FreeChunk(this->menuMusic);
}
void SoundManager::resumeMusicBackground(){
    Mix_Resume(0);
}
void SoundManager::pauseMusicBackground(){  
    Mix_Pause(0);
}

void SoundManager::pauseAllMusic(){  
    Mix_Pause(-1);
}

void SoundManager::resetAllMusic(){
    Mix_HaltChannel(-1);
}

void SoundManager::PlayWalkingSkeleton(float distance){
    Mix_PlayChannel(-1,WalkingSkeleton,0);
}

void SoundManager::playShootingBow(float distance){
    if(distance<0.2f){
        Mix_PlayChannel(-1,shootingBow,0);
    }
}
void SoundManager::playHitBow(){
    Mix_PlayChannel(-1,this->hitBow,0);
}

void SoundManager::playWalkingGrass(){
    Mix_PlayChannel(-1,this->walkingGrass,0);
}

void SoundManager::playMonsterSlash(){
    Mix_PlayChannel(-1,this->monsterSlash,0);
}