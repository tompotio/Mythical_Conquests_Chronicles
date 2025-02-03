#pragma once
#ifndef TextureManager_H
#define TextureManager_H

#include "RenderManager.hpp"

class TextureManager
{
    public:
        TextureManager();
        void addBackgroundTexture();
        void addTileTextures();
        void freeTextures();

    private:
        map<string,Texture*> tileTextures;
        map<string,Texture*> backgroundTextures;
        map<string,Texture*> charactersTextures;
};

#endif