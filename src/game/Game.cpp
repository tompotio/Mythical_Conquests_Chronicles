#include "../../include/game/Game.hpp"

Game::Game(char gameTitle[], int width, int height, bool fullscreen)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        cerr << "Error initializing SDL: " << SDL_GetError() << endl;
        exit(1);
    }
    //SDL_AudioInit("waveout");
    RenderManager& rm = RenderManager::getInstance("My Window", 800, 600,fullscreen);
    renderManager = &rm;
    
    ctx = renderManager->getGuiContext();
     
    menuManager = new MenuManager(this);
    worldManager = new WorldManager(this);

    actors[InMenu] = menuManager;
    actors[InGame] = worldManager;

    branches = IMG_LoadTexture(renderManager->getRenderer(), "../assets/background/branches.png");
}

Game::~Game(){
    delete renderManager;
    delete worldManager;
    delete menuManager;
}

float* Game::getDeltaT(){
    return &deltaT;
}

void Game::gameLoop()
{
    Uint32 previous = SDL_GetTicks();

    while(isRunning)
    {

        update();

       if(state == InLoadingScreen){
            //renderManager->draw_BackgroundColor(0,0,0,255);
            renderManager->renderClear();
            renderManager->renderPresent();
            renderManager->renderGui();
            worldManager->loadWorld(Test);
            state = InGame;
            continue;
        }

        tick(previous);

        render();
    }
    SoundManager::getInstance().freeMusic();
}

void Game::saveGame(string namefile){
    worldManager->saveGame(namefile);
}

void Game::update()
{
    actors[state]->update(deltaT);
}

/**
 * @brief Coordonne le rendu final en sous-traitant les tâches au renderManager.
 * Effectue un appel de rendu à l'acteur concerné (MenuManager ou WorldManager).
 * L'acteur concerné choisit ce qu'il souhaite afficher à l'écran.
 */
void Game::render()
{          

    renderManager->draw_BackgroundColor(0,0,0,255);

    renderManager->renderClear();    

    // Sélectionne la méthode de rendu de l'acteur.
    actors[state]->render();

    // Petit effet sympa.
    if(specialEffect && state == InMenu){
        renderManager->draw_BackgroundImage(branches);
    }

    renderManager->renderGui();
    
    renderManager->renderPresent();
}

void Game::setState(GameState s){
    state = s;

    if(resized){
        resized = false;
        actors[state]->updateScreenSize();
    }
}

void Game::tick(Uint32& previous)
{
    Uint32 current = SDL_GetTicks();
    Uint32 elapsed = current - previous;
    previous = current;
    deltaT = elapsed * 0.001;
    fps = 1 / deltaT;
    if (elapsed < 16) SDL_Delay(16 - elapsed);
}