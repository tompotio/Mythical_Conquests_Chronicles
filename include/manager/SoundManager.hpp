#pragma once
#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H


#include <SDL2/SDL_mixer.h>

class SoundManager{
    public:
        static SoundManager& getInstance();

        SoundManager(){
            printf("j'initialise l'audio");
            if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT,8, 1024) == -1) //Initialisation de l'API Mixer
            {
                printf("%s", Mix_GetError());
            }
            backgroundMusic=Mix_LoadWAV("../assets/sound/backgroundMusic.wav");
            WalkingSkeleton = Mix_LoadWAV("../assets/sound/walkingSkeleton.wav");
            shootingBow = Mix_LoadWAV("../assets/sound/bow_shoot.wav");
            hitBow = Mix_LoadWAV("../assets/sound/arrowHit.wav");
            walkingGrass = Mix_LoadWAV("../assets/sound/walkingGrass.wav");
            monsterSlash = Mix_LoadWAV("../assets/sound/monsterSlash.wav");
            menuMusic = Mix_LoadWAV("../assets/sound/menuMusic.wav");
            Mix_Volume(-1,128);
        }

        void PlayMusicBackground();
        void pauseMusicBackground();
        void resumeMusicBackground();
        void PlayWalkingSkeleton(float distance);
        void playShootingBow(float distance);
        void playHitBow();
        void playWalkingGrass();
        void playMonsterSlash();
        void playMenuMusic();
        void freeMusic();
        void pauseAllMusic();
        void resetAllMusic();

    private:
        static SoundManager* instance;
        
        Mix_Chunk *backgroundMusic;
        Mix_Chunk *WalkingSkeleton;
        Mix_Chunk *shootingBow;
        Mix_Chunk *hitBow;
        Mix_Chunk *walkingGrass;
        Mix_Chunk *monsterSlash;
        Mix_Chunk *menuMusic;
};
#endif