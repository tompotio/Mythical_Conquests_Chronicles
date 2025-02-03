#pragma once
#ifndef MAP_H
#define MAP_H

#include <vector>
#include <map>
#include <algorithm>
#include <tuple>
#include <fstream>
#include <sstream>

// Game
#include "Tile.hpp"

// Manager
#include "../manager/RenderManager.hpp"

// Librairies importées
#include "../../libs/RapidJson/include/rapidjson/document.h"
#include "../../libs/RapidJson/include/rapidjson/writer.h"
#include "../../libs/RapidJson/include/rapidjson/stringbuffer.h"
#include "../../libs/PerlinNoise-master/PerlinNoise.hpp"

using namespace std;
using namespace rapidjson; // Ajouter l'espace de noms rapidjson

/**
 * @brief Classe qui sous-traite la génération de map.
 * La classe Map est une extension de WorldManager.
 */
class Map
{
    public: 
        Map(Camera* camera);
        ~Map(){delete waterTimer;}

        void render();
        void update(float deltaT);
        void loadMap(const string& fileName);
        void createRandomMap(bool withperlin, const string& fileName, int rows, int cols);
        void getMapDimension(int* w, int* h);
        void setNight(bool val){nightOn = val;}
        void setPlayerZero(Entity* player){playerZero = player;}
        
        bool getNight(){return nightOn;}

    private:
        Document extractJsonDataForMap(const string& fileName);
        Document extractJsonDataFromTileInfos();
        void getMatriceInfosFromDocument(std::vector<std::tuple<Texture, TileType, std::tuple<bool, int, float>>>& tilesInfos, Document& document);
        void getTilesInfosFromDocument(std::vector<std::tuple<Texture, TileType, std::tuple<bool, int, float>>>& tilesInfos, Document& document);        

        // Changer en shared_ptr
        vector<TileDynamique*> dynamicTiles;
        vector<TileStatique> staticTiles;
        vector<Tile> tiles;
        Entity* playerZero;

        Timer* waterTimer = nullptr;

        RenderManager* renderManager;
        SDL_Renderer* renderer;
        Camera* camera;

        bool nightOn = false;
        int lightRadius = 7;

        int w;
        int h;
};

#endif