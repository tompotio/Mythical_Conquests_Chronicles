#pragma once
#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include "RenderManager.hpp"
#include "IActor.hpp"
#include "../game/Game.hpp"
#include "./NetworkManager.hpp"
#include <filesystem>
#include "../utils/include/utils.hpp"
class Game;

class MenuManager : public IActor
{
    public:
        MenuManager(Game* game);
        
        void render() override;
        void update(float deltaT) override;
        void updateScreenSize() override;

    private:
        void menu();
        void Options();
        void displayQrCode();
        void loadingScreen();
        string getTitleFromFile(std::filesystem::path filePath);
        RenderManager* renderManager;
        Texture background;
        struct nk_context *ctx;

        int windowWidth;
        int windowHeight;
        int menu_width;
        int menu_height;
        int options_width;
        int options_height;

        nk_bool showFPS;
        nk_bool afficherEffet = 1;

        bool showOptions = false; // j'initialise pour éviter le garbage value.
        bool showQrCode = false;
        bool showLoadingSave =false;
        NetworkManager networkManager;
        Game* game;

        bool clicking;
};

#endif