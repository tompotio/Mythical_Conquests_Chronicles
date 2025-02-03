#include "../../include/manager/WorldManager.hpp"
#include <cmath>

/**
 * @brief Construct a new World Manager:: World Manager object
 * C'est ici que l'on initialise les données constantes du monde. Comme les joueurs.
 * On charge le monde avant le joueur.
 * @param game
 */
WorldManager::WorldManager(Game* game) : game(game) { 
    initializeDefaultDatas();
    initializePhysicalWorld();
    initializeCamera();
    mapWorld = new Map(camera);
    // NB : Orienter en fonction de la souris.
    debounce[SDLK_g] = false;
    debounce[SDLK_1] = false;
    debounce[SDLK_ESCAPE] = false;
}

WorldManager::~WorldManager(){
}

void WorldManager::initializeCamera(){
    camera = renderManager->getCamera();
    camera->h = windowHeight;
    camera->w = windowWidth;
    camera->hM = windowHeight / PPM;
    camera->wM = windowWidth / PPM;
}

struct nk_image sdlTextureToNkImage(SDL_Texture* texture) {
    struct nk_image img;
    img.handle.ptr = texture;

    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);

    img.w = static_cast<nk_ushort>(width);
    img.h = static_cast<nk_ushort>(height);

    img.region[0] = 0;
    img.region[1] = 0;
    img.region[2] = img.w;
    img.region[3] = img.h;
    return img;
   
}

void WorldManager::saveGame(string namefile){
    rapidjson::StringBuffer lStringBuffer;
    rapidjson::Writer lWriter(lStringBuffer);
    playerZero->Serialize(&lWriter);
    //cout << lStringBuffer.GetString() << endl;
    ofstream myfile;
    string path = "../save/";
    path.append(namefile);
    myfile.open(path);
    myfile << lStringBuffer.GetString();
    game->setLoadedSave(namefile);
    myfile.close();
}

/**
 * @brief Chargement des données nécessaires par défaut à WorldManager.
 */
void WorldManager::initializeDefaultDatas(){
    RenderManager& rm = RenderManager::getInstance();
    renderManager = &rm;
    input_buffer[0] = '\0';

    ctx = renderManager->getGuiContext();
    windowWidth = renderManager->getWindowWidth();
    windowHeight = renderManager->getWindowHeight();

    subMenuWidth = windowWidth * .25;
    subMenuHeight = windowHeight * .6;

    healthBarWidth = windowWidth * .25;
    healthBarHeight = windowHeight * .025;

    /* Gui datas */
    fpsCounterWidth = windowWidth * 0.1;
    fpsCounterHeight = windowHeight * 0.03;
    fps = game->getFps();

    heartimg = IMG_LoadTexture(renderManager->getRenderer(), "../assets/widgets/heart.png");
    starimg = IMG_LoadTexture(renderManager->getRenderer(), "../assets/widgets/star.png");

    // Document Json du réseau.
    donneesReseauJson.SetObject();
    players.reserve(4);

    nkImages.emplace(BulbClick,sdlTextureToNkImage(IMG_LoadTexture(renderManager->getRenderer(), "../assets/widgets/bulb_clicked.png")));
    nkImages.emplace(BulbHover,sdlTextureToNkImage(IMG_LoadTexture(renderManager->getRenderer(), "../assets/widgets/bulb_hover.png")));
    nkImages.emplace(Bulb,sdlTextureToNkImage(IMG_LoadTexture(renderManager->getRenderer(), "../assets/widgets/bulb.png")));
}

/**
 * @brief On instancie un monde spécifique box2d où nos entités prendront vie.
 * Ce monde n'a pas besoin d'être recréé entre chaque mission, on peut simplement nettoyer toutes les entités.
 */
void WorldManager::initializePhysicalWorld(){
    const b2Vec2 gravity = b2Vec2(0.0f,0.0f);
    world = new b2World(gravity);
    world->SetDebugDraw(renderManager);
    world->SetAllowSleeping(false);
    
    //world->SetContinuousPhysics(true);
    b2ContactListener* listener = new ContactListener(game->getDeltaT());
    world->SetContactListener(listener); 
    cout << "j'ai initialisé" << endl;
    renderManager->SetFlags(b2Draw::e_shapeBit&&b2Draw::e_jointBit&&b2Draw::e_aabbBit&&b2Draw::e_pairBit&&b2Draw::e_centerOfMassBit);
    factory = &(BodyFactory::getInstance(world));
    entityFactory = &EntityFactory::getInstance(this);
}
void WorldManager::parseSave(){
     std::ifstream file; 
    file.open(game->getLoadedSave());
    string content;
    string line;
    while (getline(file, line)) { 
        content.append(line); 
        content.append("\n"); 
    } 
    Document save;
    save.Parse<0>(content.c_str());
    int niveau = save["lvl"].GetInt(); 
    entityFactory->makePlayer(mapW/(2.0f*PPM),mapH/(2.0f*PPM),niveau); 
    players.back().Deserialize(save);
    this->lvlGame = niveau;
}
/**
 * @brief On instancie le joueur du monde avec sa caméra.
 */
void WorldManager::initializePlayer(){    
    if(game->getLoadedSave() == ""){
        //cout << "j'initialise le joueur au niveau 1" << endl;
        entityFactory->makePlayer(mapW/(2.0f*PPM),mapH/(2.0f*PPM),1);
        this->lvlGame = 1;
        playerZero = &players.back();
        velocity = playerZero->getVelocity();
        mapWorld->setPlayerZero(&players.back());
    }
    
    else{
        parseSave();
    }

    Vec2 positionJ1 = playerZero->getBody()->GetPosition();
    camera->x = max(min((positionJ1.x - (camera->w / PPM + playerZero->getWidth())/2),(float) ((mapW - camera->w) / PPM)), 0.0f);
    camera->y = max(min((positionJ1.y - (camera->h / PPM + playerZero->getHeight())/2),(float) ((mapH - camera->h) / PPM)), 0.0f);
}   

/**
 * @brief Fonction update de WorlManager.
 */
void WorldManager::update(float deltaT)
{
    // Envoie les données.
    if (game->isOnline()){
        if(correctionTimer.getEnded()){
            correctionTimer.reset();
            clicked = false;
        }
        correctionTimer.update(deltaT);
        updateNetworkDatas();
    }

    updateInputStates(deltaT);

    if(!pause){
        updatePlayerVelocity();

        updateEntities(deltaT);

        updateCameraPosition();
    }

    updateGui(deltaT);
}

bool isEntityWithinRadius(Vec2 entityPosition, Vec2 playerPosition, float radius) {
    float dx = entityPosition.x - playerPosition.x;
    float dy = entityPosition.y - playerPosition.y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

void WorldManager::render()
{
    
    // Si on zoom modifier le facteur de zoom.
    if(zoomed){
        zoomed = false;
        renderManager->setZoomFactor(zoomfactor);
        updateCameraSize();
    }

    // Si on est en debugDraw.
    if(debugMode){
        world->DebugDraw();
    }

    // Sinon afficher les assets du jeu.
    else
    {

        mapWorld->render();
        
        if(mapWorld->getNight())
        {
            renderManager->draw_BackgroundColor(0,0,0,255);
        }

        renderPlayers();
        renderDirectionArrow();
        renderEnemies();
        renderProjectiles();

        double rad1 = 6 * PPM;
        double rad2 = 9 * PPM;

        if(mapWorld->getNight())
        {
            renderManager->draw_BackgroundColor(0,0,0,255);
            Vec2 position = renderManager->convertPoint(camera,playerZero->getPosition() + Vec2(playerZero->getWidth() / 2, playerZero->getHeight() / 2));
            for (int x = 0; x < camera->w; ++x) 
            {
                for (int y = 0; y < camera->h; ++y) 
                {       
                    double distX = x - position.x;
                    double distY = y - position.y;         
                    double distance = sqrt(distX * distX + distY * distY);

                    if (distance > rad1 && distance < rad2) 
                    {
                        renderManager->draw_Point(x, y);
                    }
                }
            }
        }
    }

    // Partie UI

    // Affiche un compteur de FPS à l'écran.
    if(game->showingFPS()){
        fpsCounter();
    }

    bulb();

    UserUI();
    
    // Affiche une petite chatBox quand on est connecté.
    if(game->isOnline()){
        chatBox();
    }

    if(subMenuOn){
        SubMenu();
    }

}

void WorldManager::UserUI()
{   
    // Affichage de la barre de vie.
    renderManager->draw_filled_rectangle(64,28,healthBarWidth, healthBarHeight,0,0,0,255);
    const float hpperc = (float) playerZero->getHp() / playerZero->getMaxHp();
    renderManager->draw_filled_rectangle(69,33, (int) ((healthBarWidth - 10) * hpperc), healthBarHeight - 10,95,26,21,255);
    renderManager->draw_Image(heartimg,20,10,64,64,255,0);

    // Affichage de la barre d'expérience.
    renderManager->draw_filled_rectangle(64,28 + healthBarHeight + 64, healthBarWidth, healthBarHeight,0,0,0,255);
    const float expperc =  (float)playerZero->getExp() / (float)playerZero->getCapExp();
    renderManager->draw_filled_rectangle(69,33 + healthBarHeight + 64, (int) ((healthBarWidth - 10) * expperc), healthBarHeight - 10, 100, 75, 2, 255);
    renderManager->draw_Image(starimg,20,10 + 60 + healthBarHeight, 64, 64, 255, 0);

    // Affichage des skills.
    renderManager->draw_filled_rectangle(windowWidth / 2 - 143, windowHeight - 74,64,64,0,0,0,255);
    renderManager->draw_filled_rectangle(windowWidth / 2 - 69, windowHeight - 74,64,64,0,0,0,255);
    renderManager->draw_filled_rectangle(windowWidth / 2 + 10, windowHeight - 74,64,64,0,0,0,255);
    renderManager->draw_filled_rectangle(windowWidth / 2 + 84, windowHeight - 74,64,64,0,0,0,255);
}

void WorldManager::bulb()
{
    struct nk_color window_background = nk_rgba(0, 0, 0, 0); // Couleur de fond transparente.
    struct nk_color button_background = nk_rgba(0, 0, 0, 0);
    struct nk_color button_text = nk_rgb(0, 0, 0); // Couleur du texte du bouton.

    // Sauvegarder l'ancien style de fenêtre.
    struct nk_style old_style = ctx->style;

    // Appliquer le style transparent à la fenêtre.
    ctx->style.window.fixed_background = nk_style_item_color(window_background);

    if (nk_begin(ctx, "light", nk_rect(
            windowWidth - 64, 100,
            64, 64),
            NK_WINDOW_NO_SCROLLBAR
    ))
    {
        nk_layout_row_dynamic(ctx, 50, 1);

        // Définir le style pour le bouton.
        ctx->style.button.normal = nk_style_item_color(button_background);
        ctx->style.button.hover = nk_style_item_color(button_background);
        ctx->style.button.active = nk_style_item_color(button_background);
        ctx->style.button.border_color = button_background;
        ctx->style.button.text_background = button_background;
        ctx->style.button.text_normal = button_text;
        ctx->style.button.text_hover = button_text;
        ctx->style.button.text_active = button_text;

        // Si le joueur effectue une interaction avec le bouton.
        if (nk_window_is_hovered(ctx))
        {
            // Si le joueur clique.
            if (nk_input_has_mouse_click(&ctx->input, NK_BUTTON_LEFT))
            {
                nk_button_image(ctx, nkImages[BulbClick]);
                mapWorld->setNight(!mapWorld->getNight());
                // Première fois que je clique (Sinon on affiche l'image quand même pour l'effet de pression).
                if(!clicked){
                    clicked = true;
                    correctionTimer.reset();
                    correctionTimer.setEnd(.3); 
                }
            }
            // Si le joueur hover juste.
            else 
            {
                nk_button_image(ctx, nkImages[BulbHover]);
            }
        }
        // On affiche le bouton par défaut.
        else
        {
            nk_button_image(ctx, nkImages[Bulb]);
        }
    }
    nk_end(ctx);

    // Restaurer l'ancien style de la fenêtre.
    ctx->style = old_style;
}

void WorldManager::getMouseAngle(){
    SDL_GetMouseState((&xMouse), (&yMouse));
    Vec2* positionJoueur = new Vec2(playerZero->getBody()->GetPosition());
    positionJoueur->x += playerZero->getWidth()/2;
    positionJoueur->y += playerZero->getHeight()/2;

    float xMouseInMeter = xMouse/PPM + camera->x / zoomfactor;
    float yMouseInMeter = yMouse/PPM + camera->y / zoomfactor;
    playerZero->setAngle(-1 * atan2(yMouseInMeter-positionJoueur->y, xMouseInMeter-positionJoueur->x)*180/M_PI);
}
void WorldManager::makeNewMonster(){
    int choice = rand() % 2 + 1;
    if(choice == 1){
        float x = 1.0 + static_cast<float> (rand()) / (static_cast<float> (RAND_MAX/(mapW/PPM - 1.0)));
        float y = 1.0 + static_cast<float> (rand()) / (static_cast<float> (RAND_MAX/(mapH/PPM - 1.0))); 
        entityFactory->makeArcherSkeleton(x,y,PLAYERSIZE_W,PLAYERSIZE_H,lvlGame);
    }
    if(choice == 2){
         float x = 1.0 + static_cast<float> (rand()) / (static_cast<float> (RAND_MAX/(mapW/PPM - 1.0)));
        float y = 1.0 + static_cast<float> (rand()) / (static_cast<float> (RAND_MAX/(mapH/PPM - 1.0))); 
        entityFactory->makeSkeleton(x,y,PLAYERSIZE_W,PLAYERSIZE_H,lvlGame);
    }

}

void WorldManager::deleteEntities(){
    int i = 0;
    vector<shared_ptr<Projectile>> deletedProjectile;
    vector<int>indexDelete;
    for(auto projectile:projectiles){
        if(!projectile->getActive()){
            deletedProjectile.push_back(projectile);
            indexDelete.push_back(i);
        }
        i++;
    }
    i = 0;
    for(auto projectile:deletedProjectile){
      world->DestroyBody(projectile->getBody());
      int indexProjectile = indexDelete[i];
      projectiles.erase(projectiles.begin() + indexProjectile);
      i++;
    }
    deletedProjectile.clear();
    indexDelete.clear();

    i = 0;
    vector<shared_ptr<Enemy>> deletedEnemy;
    for(auto enemy:enemies){
        if(!enemy->getActive()){
            deletedEnemy.push_back(enemy);
            indexDelete.push_back(i);
        }
        i++;
    }
    i = 0;
    for(auto enemy:deletedEnemy){
      world->DestroyBody(enemy->getBody());
      int indexEnemy = indexDelete[i];
      enemies.erase(enemies.begin() + indexEnemy);
      i++;
      this->monsterKilled ++;
      makeNewMonster();
    }
    deletedEnemy.clear();
    indexDelete.clear();
}

void WorldManager::makeProjectile(float x, float y,float vX,float vY,float angle){
    entityFactory->makeProjectile(x,y,vX,vY,angle);
}

/**
 * @brief Met à jour les données liées aux inputs du joueur et de ses states.
 */
void WorldManager::updateInputStates(float deltaT){
    // Si le joueur est en train d'attaquer, on ne récupère plus les inputs.
    EntityState currState = playerZero->getState();
    if(currState == VSlashState || currState == HSlashState){
        if(playerZero->getAnimation(currState).isFinished() && playerZero->isSkillFinished()){
            playerZero->getAnimation(currState).reset();
            playerZero->setState(IdleState);
            canMove = true;
        }
    }

    /* [Retrieve Inputs] -------------------------------------------------------------------------------------------------
    * C'est ici que l'on récupère les inputs entrés par le joueur.
    */
    SDL_Event e;
    nk_input_begin(ctx);
    while(SDL_PollEvent(&e)){
        const SDL_Keycode key = e.key.keysym.sym;
        switch (e.type) {
            case SDL_KEYDOWN:
                switch(key){
                    /**
                     * @brief Touche activable une seule fois.
                     * Autrement dit on l'active en direct.
                     * À noter que certaines de ces touches n'ont pas besoin de debounce.
                     */
                    case SDLK_ESCAPE:
                        if(!debounce[key]){
                            debounce[key] = true;
                            subMenuOn = !subMenuOn;
                            if(!(game->isOnline()))
                            {
                                pause = subMenuOn;
                            }
                            debounce[key] = false;
                        }                        
                        break;
                    case SDLK_1:
                        addCommandInput(key);
                        break;
                    /* Touches activables en même temps que d'autres. */
                    case SDLK_g:
                        addCommandInput(key);
                        break;
                    case SDLK_LSHIFT:
                        addCommandInput(key);
                        break;
                    case SDLK_RETURN:
                        addCommandInput(key);
                        break;
                    case SDLK_z:
                        addMoveInput(key);
                        break;
                    case SDLK_q:
                        addMoveInput(key);
                        break;
                    case SDLK_s:
                        addMoveInput(key);
                        break;
                    case SDLK_d:
                        addMoveInput(key);
                        break;
                }
                break;
            case SDL_KEYUP:
                switch(key){
                    case SDLK_g:
                        debounce[key] = false;
                        removeCommandInput(key);
                        break;
                    case SDLK_LSHIFT:
                        if(moveInputs.size() == 0){
                            changePlayerState(IdleState);
                        }else{
                            changePlayerState(WalkingState);
                        }
                        removeCommandInput(key);
                        break;
                    case SDLK_1:
                        debounce[key] = false;
                        removeCommandInput(key);
                        break;
                    case SDLK_RETURN:
                        removeCommandInput(key);
                        break;
                    default:
                        removeMoveInput(key);
                        changePlayerState(IdleState);
                        break;
                }
                break;
            case SDL_MOUSEWHEEL:
                if(e.wheel.y >= 1.0f){
                    zoomIn(deltaT);
                }else{
                    zoomOut(deltaT);
                }
                zoomed = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT){
                    EntityState currState = playerZero->getState();
                    if(currState!=VSlashState && currState != HSlashState){
                        if(playerZero->applySkill(HSlashSkill)){
                            changePlayerState(HSlashState);
                            canMove = false;

                        }
                    }
                    
                }   
                break;
        }
        nk_sdl_handle_event(&e);
    }
    nk_input_end(ctx);

    /* [Execute Inputs] -------------------------------------------------------------------------------------------------
    * C'est ici que l'on exécute les inputs entrés par le joueur.
    */

    /*calcul de l'angle d'attaque*/
    if(canMove && !pause){ // Je teste canMove comme ça, si le joueur hover un wiget, on n'a pas besoin de recalculer l'angle.
        getMouseAngle();
        if(playerZero->getAngle() < 90 && playerZero->getAngle() > (-90)){
            playerZero->setSimpleLook(Right);
        }
        else{
            playerZero->setSimpleLook(Left);
        }
    }

    /* Mouvements. */
    const float walkspeed = playerZero->getWalkSpeed();
    for (SDL_Keycode key : moveInputs){
        switch(key){
        case SDLK_z:
                velocity->y -= walkspeed * deltaT;
                break;
            case SDLK_s:
                velocity->y += walkspeed * deltaT;
                break;
            case SDLK_d:
                velocity->x += walkspeed * deltaT;
                break;
            case SDLK_q:
                velocity->x -= walkspeed * deltaT;
                break;
        }

        // Si le joueur n'est pas déjà en train de courir.
        if(!(playerZero->getState() == RunningState)){
            changePlayerState(WalkingState);
        }
    }
    
    /* Inputs d'autres commandes. */
    for (SDL_Keycode key : commandInputs){
        switch(key){
            case SDLK_g:
                if (!debounce[key]) {
                    debounce[key] = true;
                    debugMode = !debugMode;
                }
                break;
            case SDLK_1:
                if (!debounce[key]) {
                    debounce[key] = true;
                    
                }
                break;
            case SDLK_LSHIFT:
                // Le joueur ne peut courir que s'il est déjà en train de marcher.
                changePlayerState(RunningState);
                break;
        }
    }

    /* [Gestion des states du joueur]  -----------------------------------------------------------------------------
    * C'est ici où traite les données du joueur en fonction des états.
    */

    // Si le player est en train de courir mettre à jour la vélocité.
    if(playerZero->getState() == RunningState){
        *velocity = playerZero->getAcceleration() * (*velocity);
    }
}

/**
 * @brief Prépare les données à envoyer depuis le fichier Json.
 * 
 */
void WorldManager::prepareNetworkDatas()
{
    donneesReseauJson.SetObject();
    commandes.clear();
}

/**
 * @brief 
 * 1 - Réceptionne les données et mets à jour via la méthode analyseNetworkDatas()
 * 2 - Envoie les données. 
 * Durant la première étape si le joueur est hôte, vérifie s'il y a un nouveau joueur.
 */
void WorldManager::updateNetworkDatas()
{
    if (game->isHost()){
        short int id = 0;
        TCPsocket socket = nullptr;
        networkManager.checkForNewPlayer(&socket, &id);

        // Vérifie s'il y a un nouveau joueur
        if (id > 0) {
            // Crée un nouveau joueur
            entityFactory->makePlayer(mapW/(2.0f*PPM),mapH/(2.0f*PPM),1);

            // Attribue l'ID au joueur
            players.back().setId(id);

            // Envoie l'ID au nouveau joueur
            commandes.push_back(GiveYouId);
            donneesReseauJson.AddMember("GiveYouId", id, donneesReseauJson.GetAllocator());
            sendNetworkDatas(socket, true);

            // Envoie l'information sur le nouveau joueur aux autres joueurs
            commandes.pop_back(); // Retire GiveYouId
            commandes.push_back(NewPlayer);
            donneesReseauJson.RemoveMember("GiveYouId");
            donneesReseauJson.RemoveMember("Commands");
            donneesReseauJson.AddMember("NewPlayer", id, donneesReseauJson.GetAllocator());
            sendNetworkDatas(socket, false);
        }

        // Sinon envoie juste les données.
        else
        {
            if(donneesReseauJson.MemberCount() > 0){
                sendNetworkDatas();
            }
        }

        // Réceptionne les données.
        std::vector<std::vector<char>> vecdatas = networkManager.Server_ReceiveDatas();
        for (const std::vector<char>& datas : vecdatas) {
            analyseDatas(datas.data());
        }
    }

    // Un client a simplement besoin de réceptionner les données.
    else
    {
        // Envoie les données.
        if(donneesReseauJson.MemberCount() > 0){
            sendNetworkDatas();
        }

        // Réceptionne les données.
        char* datas = networkManager.Client_ReceiveDatas();
        analyseDatas(datas);
    }

    // Nettoie les données
    if(donneesReseauJson.MemberCount() > 0){
        prepareNetworkDatas();
    }
}

/**
 * @brief Envoie les données du Json joueurs.
 * Optionnellement on peut mettre un socket. : 
 *      - Si ignoreAll est à true, ignore tous les joueurs sauf le socket en option.
 *      - Si ignoreAll est à false, ignore le socket optionnel non null.
 *      - Sinon envoie à tout le monde par défaut lorsque le socket est null.
 * Cette option ne s'applique que à la fonction Server_SendDatas() pas à Client_SendDatas, car celle-ci se charge d'envoie les données que au serveur.
 * @param ignoreAll 
 */
void WorldManager::sendNetworkDatas(TCPsocket socket, bool ignoreAll){

    // Avant toute chose rajoute les commandes à envoyer.
    Value tableau(kArrayType);
    for (const auto& commande : commandes) {
        tableau.PushBack(commande, donneesReseauJson.GetAllocator());
    }
    donneesReseauJson.AddMember("Commands",tableau,donneesReseauJson.GetAllocator());

    // Crée le message Json à envoyer sous format char*.
    StringBuffer buffer;
    Writer<rapidjson::StringBuffer> writer(buffer);
    donneesReseauJson.Accept(writer);
    const char* jsonStr = buffer.GetString();
    
    if(game->isHost())
    {
        networkManager.Server_SendDatas(jsonStr,buffer.GetSize(), socket, ignoreAll);
    }

    else
    {
        networkManager.Client_SendDatas(jsonStr,buffer.GetLength());
    }
}

/*

Brouillar de guerre. 1h30 
Menu intermédiaire 1h30 
Faire en sorte d'attaquer en cliquant 30 minutes 
HUD avec barre de vies 1h30 
Un objectif tuer 5 squelettes 1h30 

*/

void WorldManager::analyseDatas(const char* datas){
    // Une erreur est survenue.
    if(datas == nullptr){
        // gérer l'erreur.
    }
    // Les données sont correctes.
    else
    {
        // On a reçu quelque chose.
        if(datas[0] != '\0')
        {
            Document document;
            document.SetObject();
            document.Parse(datas);

            // Vérifie si le parsing a marché.
            if (document.IsObject()) {
                if(document.HasMember("Commands") && document["Commands"].IsArray()){
                    // Je récupère les valeurs des commandes.
                    const Value& commands = document["Commands"];

                    // Je parcours les commandes et effectue l'action qui leur est associée.
                    for (SizeType i = 0; i < commands.Size(); ++i) {
                        if (commands[i].IsInt()) {
                            NetworkCommands com = (NetworkCommands) commands[i].GetInt();
                            casesDatas(&document,com);
                        }
                    }
                }
            }
            // Gérer l'erreur.
            else{

            }
        }
    }
}

/**
 * @brief Fonction qui gère les différents cas associés aux commandes.
 * 
 * @param commande 
 */
void WorldManager::casesDatas(Document* document, NetworkCommands commande){
    switch(commande){
        case GiveYouId:
        {
            const Value& id = (*document)["GiveYouId"];

            if (id.IsInt()) 
            {
                playerZero->setId(id.GetInt());
                entityFactory->makePlayer(mapW/(2.0f*PPM),mapH/(2.0f*PPM),1);
            }

        }
            break;
        case NewPlayer: 
        {
            const Value& id = (*document)["NewPlayer"];

            if (id.IsInt())
            {
                entityFactory->makePlayer(mapW/(2.0f*PPM),mapH/(2.0f*PPM),1);
                players.back().setId(id.GetInt());
            }
        }
            break;
        case Speed: 
        {
            if ((*document).HasMember("Speed")) {
                const Value& speed = (*document)["Speed"];
                if (speed.IsObject()) {
                    if (
                        speed.HasMember("id") && speed["id"].IsInt() &&
                        speed.HasMember("x") && speed["x"].IsFloat() && 
                        speed.HasMember("y") && speed["y"].IsFloat()) 
                    {
                        auto player_it = std::find_if(
                            players.begin(), 
                            players.end(), 
                            [&](Player& p) { 
                                return p.getId() == (short int) (speed["id"].GetInt()); 
                            }
                        );

                        if (player_it != players.end()) {
                            Player& player = *player_it;
                            player.Move(
                                speed["x"].GetFloat(),
                                speed["y"].GetFloat()
                            );
                        } 
                        // L'id est corrompu...
                        else {
                            
                        }

                    } 
                    // Gérer l'erreur.
                    else {
                        
                    }
                } 

                // Gérer l'erreur.
                else {
                    
                }
            } 

            // Gère les erreurs.
            else {
                
            }
        }
            break;
        case Position: 
        {
            if ((*document).HasMember("Position")) {
                const Value& position = (*document)["Position"];
                if (position.IsObject()) {
                    if (
                        position.HasMember("id") && position["id"].IsInt() &&
                        position.HasMember("x") && position["x"].IsFloat() && 
                        position.HasMember("y") && position["y"].IsFloat()) 
                    {
                        auto player_it = std::find_if(
                            players.begin(), 
                            players.end(), 
                            [&](Player& p) { 
                                return p.getId() == (short int) (position["id"].GetInt()); 
                            }
                        );

                        if (player_it != players.end()) {
                            Player& player = *player_it;
                            player.MoveTo(
                                position["x"].GetFloat(),
                                position["y"].GetFloat()
                            );
                        } 
                        // L'id est corrompu...
                        else {

                        }

                    } 
                    // Gérer l'erreur.
                    else {
                        
                    }
                } 

                // Gérer l'erreur.
                else {
                    
                }
            } 

            // Gère les erreurs.
            else {
                
            }
        }
            break;
        case Message:
        {
            const Value& messageObj = (*document)["Message"];
            string message;

            // Vérification et récupération du membre "id"
            if (messageObj.HasMember("id") && messageObj["id"].IsInt()) {
                int id = messageObj["id"].GetInt();
                message = "joueur_" + to_string(id) + " : ";
            } 

            // Gérer l'erreur.
            else {
                
            }

            // Vérification et récupération du membre "message"
            if (messageObj.HasMember("message") && messageObj["message"].IsString()) {
                message += messageObj["message"].GetString();
            } 

            // Gérer l'erreur.
            else {
                
            }

            messages.push_back(message);
        }
            break;
        // La commande est corrompue.
        default:
            // Gérer l'erreur.
            break;
    }
}

/**
 * @brief Sous-menu. Met le jeu en pause si on n'est pas en ligne.
 */
void WorldManager::SubMenu(){
    if (nk_begin(ctx, "SubMenu", nk_rect(
        (windowWidth - subMenuWidth) / 2, 
        (windowHeight - subMenuHeight) / 2, 
        subMenuWidth, subMenuHeight),
        NK_WINDOW_BORDER |
        NK_WINDOW_NO_SCROLLBAR))
    {
        float ratio[] = {0.1f, 0.80f,0.1f};
        int h = subMenuHeight / 12;
        nk_layout_row(ctx, NK_DYNAMIC, h, 3, ratio);

        nk_spacer(ctx);nk_spacer(ctx);nk_spacer(ctx);

        nk_spacer(ctx);
      
        if (nk_button_label(ctx, "Reprendre")) {
            pause = false;
            subMenuOn = false;
        }
        nk_spacer(ctx);

        nk_spacer(ctx); nk_spacer(ctx); nk_spacer(ctx);

        nk_spacer(ctx);

        // Gère le cas où on est déjà en ligne et on souhaite quitter.
        if (game->isOnline()) {
            if(game->isHost())
            {
                if(nk_button_label(ctx, txtFermer))
                {
                    
                }
            }

            else 
            {
                if (nk_button_label(ctx, txtQuitter))
                {
                    
                }
            }
            nk_spacer(ctx);
        }

        // Gère le cas où on se met en mode en ligne.
        else {
            if (nk_button_label(ctx, txtLancer)) {
                // Créer le socket serveur et met Online a true si tout s'est bien passé.
                networkManager.startGameServer();
                game->setHost(true);
                game->setOnline(true);
                subMenuOn = false;
                pause = false;
            }
            nk_spacer(ctx);

            nk_spacer(ctx);nk_spacer(ctx);nk_spacer(ctx);

            nk_spacer(ctx);
            if (nk_button_label(ctx, "Rejoindre serveur")) {
                // Crée un socket client et met Online a true si on s'est bien connecté ! :D
                game->setOnline(true);
                networkManager.startGameClient();
                subMenuOn = false;
                pause = false;
            }
            nk_spacer(ctx);
        }

        nk_spacer(ctx); nk_spacer(ctx); nk_spacer(ctx);

        nk_spacer(ctx);
        if (nk_button_label(ctx, "Menu Principal")) {
            clearWorld();
            pause = false;
            subMenuOn = false;
            showPopUP = false;
            game->setState(InMenu);
        }
        nk_spacer(ctx);

        nk_spacer(ctx); nk_spacer(ctx); nk_spacer(ctx);
    }
    nk_end(ctx);
}

/**
 * @brief Génère une petite chatBox pour discuter avec les joueurs connectés.
 * 
 */
void WorldManager::chatBox(){  
    if (nk_begin(ctx, "Chatbox", nk_rect(0, windowHeight - 300, 400, 300), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 150, 1);
        // Zone de texte défilante pour les messages
        if (nk_group_begin(ctx, "Messages", NK_WINDOW_BORDER)) {
            nk_layout_row_dynamic(ctx, 20, 1);
            for (const string& message : messages) {
                nk_label(ctx, message.c_str(), NK_TEXT_LEFT);
            }
            nk_group_end(ctx);
        }
        if (nk_group_begin(ctx, "Envoi", NK_WINDOW_BORDER)) {
            // Entrée de texte pour taper les nouveaux messages
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, input_buffer, sizeof(input_buffer), nk_filter_default);

            // Si le joueur appuie sur la touche Entrer et était en train d'écrire, envoie le message.
            if(
                nk_window_has_focus(ctx) && 
                find(commandInputs.begin(), commandInputs.end(), SDLK_RETURN) != commandInputs.end()
            ){
                if (input_buffer[0] != '\0') {
                    messages.push_back(input_buffer);
                    // Avant de réinitialiser le buffer, envoie le message à l'hôte.
                    sendMessage();
                    input_buffer[0] = '\0';
                }
            }
        }
    }
    nk_end(ctx);
}

/**
 * @brief Ajoute dans le document Json le message à envoyer aux autres joueurs.
 * [int][char]
 */
void WorldManager::sendMessage() {
    commandes.push_back(Message);

    string messageText = input_buffer;

    Value message(kObjectType);
    message.AddMember("message", Value(messageText.c_str(), donneesReseauJson.GetAllocator()).Move(), donneesReseauJson.GetAllocator());
    message.AddMember("id", playerZero->getId(), donneesReseauJson.GetAllocator());

    donneesReseauJson.AddMember("Message", message, donneesReseauJson.GetAllocator());
}

/**
 * @brief Ajoute au document Json les coordonnées à envoyer aux autres joueurs.
 */
void WorldManager::sendCoordinates() {
    // Envoie la vitesse du joueur.
    commandes.push_back(Speed);
    Value speed(kObjectType);
    speed.AddMember("id", playerZero->getId(), donneesReseauJson.GetAllocator());
    speed.AddMember("x", velocity->x, donneesReseauJson.GetAllocator());
    speed.AddMember("y", velocity->y, donneesReseauJson.GetAllocator());
    donneesReseauJson.AddMember("Speed", speed, donneesReseauJson.GetAllocator());

    // Envoie la position du joueur.
    commandes.push_back(Position);
    Value coordonnees(kObjectType);
    const Vec2 pos = playerZero->getPosition();
    coordonnees.AddMember("id", playerZero->getId(), donneesReseauJson.GetAllocator());
    coordonnees.AddMember("x", pos.x, donneesReseauJson.GetAllocator());
    coordonnees.AddMember("y", pos.y, donneesReseauJson.GetAllocator());
    donneesReseauJson.AddMember("Position", coordonnees, donneesReseauJson.GetAllocator());
}

/**
 * @brief Dessine une petite flèche qui indique le regard du joueur.
 * 
 */
void WorldManager::renderDirectionArrow(){
    // Affiche la flèche.
    double angle = - playerZero->getAngle();
    float length = 60;
    float x1 = playerZero->getBody()->GetPosition().x - playerZero->getWidth()/2 + .1;
    float y1 =  playerZero->getBody()->GetPosition().y + playerZero->getWidth()/2 - .1;
    Vec2 premierPoint = renderManager->convertPoint(camera,Vec2(x1,y1)); 
    float x2 = premierPoint.x + length * cos(angle * M_PI / 180.0);
    float y2 = premierPoint.y + length * sin(angle * M_PI / 180.0);
    renderManager->draw_Image(playerZero->getArrow(),x2,y2,36,36,255,angle);
}

/**
 * @brief Affiche les ennemis.
 * 
 */
void WorldManager::renderEnemies(){
    for(std::shared_ptr<Enemy> enemy : enemies){
        if(
            !mapWorld->getNight() ||
            isEntityWithinRadius(
                dynamic_pointer_cast<Entity>(enemy)->getPosition(),
                playerZero->getPosition(),
                7
            ))
        {
            const Vec2 coordonnees = renderManager->convertPoint(camera,enemy->getBody()->GetPosition());
            Animation& animation = enemy->getAnimation(enemy->getState());
            const Vec2 newcoordinates = centerTextureOnHitBox(enemy.get(),coordonnees,animation);

            if(enemy->getSimpleLook() == Left){
                renderManager->drawImageByVerticalAxisSymmetry(
                    animation.getCurrentFrame(),
                    newcoordinates.x,
                    newcoordinates.y,
                    animation.getTextureWidth(), animation.getTextureHeight(), 
                    255,
                    0
                );
            }

            else{
                renderManager->draw_Image(
                    animation.getCurrentFrame(),
                    newcoordinates.x,
                    newcoordinates.y,
                    animation.getTextureWidth(), animation.getTextureHeight(), 
                    255,
                    0
                );
            }
        }
    }
}

void WorldManager::renderProjectiles(){
    for(std::shared_ptr<Projectile> projectile : projectiles){
        if(
            !mapWorld->getNight() || 
            isEntityWithinRadius(
                dynamic_pointer_cast<Entity>(projectile)->getPosition(),
                playerZero->getPosition(),
                7
            ))
        {
            const Vec2 coordonnees = renderManager->convertPoint(camera,projectile->getBody()->GetPosition());
            Animation& animation = projectile->getAnimation(IdleState);
            const Vec2 newcoordinates = centerTextureOnHitBox(projectile.get(),coordonnees,animation);
            // cout << projectile->getBody()->GetAngle() << endl;
            if(projectile->getSimpleLook() == Left){
                renderManager->drawImageByVerticalAxisSymmetry(
                    animation.getCurrentFrame(),
                    newcoordinates.x,
                    newcoordinates.y,
                    animation.getTextureWidth(), animation.getTextureHeight(), 
                    255,
                    projectile->getBody()->GetAngle()*180/M_PI
                );
            }
            
            else{
                renderManager->draw_Image(
                    animation.getCurrentFrame(),
                    newcoordinates.x,
                    newcoordinates.y,
                    animation.getTextureWidth(), animation.getTextureHeight(), 
                    255,
                    projectile->getBody()->GetAngle()*180/M_PI
                );
            }
        }
    }
}
/**
 * @brief Affiche les joueurs (Pour l'instant que un lol).
 * 
 */
void WorldManager::renderPlayers(){
    for(Player& player : players){
        if(!mapWorld->getNight() || isEntityWithinRadius(player.getPosition(),playerZero->getPosition(),7)){
            Vec2 coordonnees = renderManager->convertPoint(camera,player.getBody()->GetPosition());
            Animation& animation = player.getAnimation(player.getState());
            const Vec2 newcoordinates = centerTextureOnHitBox(&player,coordonnees,animation);
            uint alpha = player.getInvicible() ? 150 : 255;

            if(player.getSimpleLook() == Left){
                renderManager->drawImageByVerticalAxisSymmetry(
                    animation.getCurrentFrame(),
                    newcoordinates.x,
                    newcoordinates.y,
                    animation.getTextureWidth(), animation.getTextureHeight(), 
                    alpha,
                    0
                );
            }
            
            else{
                renderManager->draw_Image(
                    animation.getCurrentFrame(),
                    newcoordinates.x,
                    newcoordinates.y,
                    animation.getTextureWidth(), animation.getTextureHeight(), 
                    alpha,
                    0
                );
            }
        }
    }
}

/**
 * @brief Charge un monde spécifique. Normalement appelé par l'instance de Game.
 *
 * @param world
 */
void WorldManager::loadWorld(EWorld world)
{
    SoundManager::getInstance().playMenuMusic();
    switch(world){
        case Lobby:
            loadLobby();
            break;
        case Test:
            loadTest();
            break;
    }
    inGame=true;
    SoundManager::getInstance().PlayMusicBackground();
}

/**
 * @brief Fonction qui automatise le changement d'état du joueur. Reset les animations liées à cet état.
 * 
 * @param state 
 */
void WorldManager::changePlayerState(EntityState state){
    if(state != playerZero->getState()){
        Animation& animation = playerZero->getAnimation(playerZero->getState());
        animation.reset();
        playerZero->setState(state);
    }
}

void WorldManager::updateScreenSize()
{
    cout << &windowWidth << endl;
    cout << &windowHeight << endl;
    renderManager->getFullWindowSize(&windowWidth, &windowHeight);
    fpsCounterWidth = windowWidth * 0.25;
    fpsCounterHeight = windowWidth * 0.1;
    subMenuWidth = windowWidth * .25;
    subMenuHeight = windowHeight * .7;
    healthBarWidth = windowWidth * .25;
    healthBarHeight = windowHeight * .025;
}

void WorldManager::removeMoveInput(SDL_Keycode key){
    auto it = find(moveInputs.begin(), moveInputs.end(), key);
    if (it != moveInputs.end()) {
        moveInputs.erase(it);
    }
}

void WorldManager::addMoveInput(SDL_Keycode key){
    if(canMove){
        auto it = find(moveInputs.begin(), moveInputs.end(), key);
        if (it == moveInputs.end()) {
            moveInputs.push_back(key);
        }
    }
}

void WorldManager::removeCommandInput(SDL_Keycode key){
    auto it = find(commandInputs.begin(), commandInputs.end(), key);
    if (it != commandInputs.end()) {
        commandInputs.erase(it);
    }
}

void WorldManager::addCommandInput(SDL_Keycode key){
    auto it = find(commandInputs.begin(), commandInputs.end(), key);
    if (it == commandInputs.end()) {
        commandInputs.push_back(key);
    }
}

void WorldManager::zoomIn(float deltaT){
    zoomfactor += deltaT * ZOOMSPEED;
    zoomfactor = min(zoomfactor, ZOOMMAX);
}

void WorldManager::zoomOut(float deltaT){
    zoomfactor -= deltaT * ZOOMSPEED;
    zoomfactor = max(zoomfactor,1.0f);
}

void WorldManager::fpsCounter()
{
    // Applique une couleur transparente.
    //nk_style_item_hide();
    if (nk_begin(ctx, "fps", nk_rect(
            windowWidth - fpsCounterWidth, 0,
            fpsCounterWidth, fpsCounterHeight),
            NK_WINDOW_NO_SCROLLBAR
    ))
    {
        string str = to_string(fps);
        const char* fps = str.c_str();
        nk_layout_row_dynamic(ctx,50,1);
        nk_label(ctx,fps,NK_WINDOW_BORDER);
    }
    nk_end(ctx);
    //nk_style_pop_style_item(ctx);
}

void WorldManager::updateEntities(float deltaT){
    // Nettoie les entités.
    deleteEntities();
    
    for(Player& player : players){
        player.update(deltaT);
    }
    
    for(auto enemy : enemies){
        enemy->update(deltaT);
    }

    for(auto projectile:projectiles){
        projectile->update(deltaT);
    }

    mapWorld->update(deltaT);
    world->Step(timeStep,velocityIteration,positionIteration);
}

void WorldManager::updateGui(float deltaT){
    // Si une window est hovered le joueur ne doit pas bouger.
    if (nk_window_is_any_hovered(ctx)){
        canMove = false;
    }
    else {
        canMove = true;
    }

    // change la valeur des fps toutes les 2s. Changer en utilisant un timer ? :D
    if((CDFPS >= 2)){CDFPS = 0; fps = (int) game->getFps();}
    CDFPS += deltaT;
}

void WorldManager::updatePlayerVelocity(){
    playerZero->getBody()->SetLinearVelocity(*velocity);
    // Si le joueur est en ligne et qu'il bouge, on notifie de sa nouvelle position.
    if(game->isOnline() && (velocity-> x > 0.0f || velocity->y > 0.0f)){
        sendCoordinates();
    }
    *velocity = Vec2(0,0);
}

void WorldManager::updateCameraPosition(){
    Vec2 positionJ1 = playerZero->getBody()->GetPosition();
    camera->x = max(min((positionJ1.x - (camera->w / PPM - playerZero->getWidth())/2),(float) ((mapW - camera->w) / PPM)), 0.0f);
    camera->y = max(min((positionJ1.y - (camera->h / PPM - playerZero->getHeight())/2),(float) ((mapH - camera->h) / PPM)), 0.0f);
}

/**
 * @brief Effectue les calculs pour redéfinir la caméra lors d'un zoom.
 *
 */
void WorldManager::updateCameraSize(){
    const float widthBefore = windowWidth;
    const float heightBefore = windowHeight;
    const float widthNow = widthBefore / zoomfactor;
    const float heigthNow = windowHeight / zoomfactor;

    Vec2 positionJ0 = playerZero->getBody()->GetPosition();
    camera->w = widthNow;
    camera->h = heigthNow;
    camera->x = max(min((positionJ0.x - (camera->w / PPM - playerZero->getWidth())/2),(float) ((mapW - camera->w) / PPM)) , 0.0f);
    camera->y = max(min((positionJ0.y - (camera->h / PPM - playerZero->getHeight())/2),(float) ((mapH - camera->h) / PPM)), 0.0f);
}

#include <thread>

/**
 * @brief Nettoie le monde en prévision du chargement d'un nouveau monde.
 * 
 */
void WorldManager::clearWorld(){
    
    for(Player& player : players){
        BodyFactory::getInstance().destroyBody(player.getBody());
    }

    for(auto enemy : enemies){
        BodyFactory::getInstance().destroyBody(enemy->getBody());
    }

    for(auto projectile : projectiles){
        BodyFactory::getInstance().destroyBody(projectile->getBody());
    }

    players.clear();
    enemies.clear();
    projectiles.clear();
    deletedCache.clear();

    SoundManager::getInstance().resetAllMusic();
}

/**
 * @brief Monde test.
 * 
 */
void WorldManager::loadTest()
{
    //this->enemies.clear();
    //world = new b2World(Vec2(0.0,0.0));
    Texture bg = IMG_LoadTexture(renderManager->getRenderer(), "../assets/background/skeletonsBanner.jpg");
    int bg_w;
    int bg_h;

    renderManager->getTextureSize(bg,&bg_w,&bg_h);

    int gapw = windowWidth - bg_w;
    int gaph = windowHeight - bg_h;

    int nbbloc = 0;
    int maxbloc = 40;

    int blocWidth = windowWidth / maxbloc;
    int blocHeight = windowHeight * .1;
    int gradient = 150 / maxbloc;

    /**
     * @brief Charge le jeu avec une barre de petits blocs.
     * À chaque itération, un nouveau bloc apparaît.
     */
    while(nbbloc < maxbloc + 1)
    {
        renderManager->draw_BackgroundColor(0,0,0,255);
        renderManager->renderClear();

        renderManager->draw_Image(bg, gapw/2, gaph/2,bg_w,bg_h,255,0);
    
        if (nk_begin(ctx, "Tips", nk_rect((windowWidth - 800) / 2, gaph/2 + bg_h, 800, 50),
            NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR
        )) 
        {
            nk_layout_row_static(ctx, 50, 800, 1);
            if (nbbloc < 20)
            {
                nk_label(ctx, "S'éloigner suffisamment loin des ennemis permet de les semer.", NK_TEXT_CENTERED);
            }
            else if (nbbloc < maxbloc)
            {
                nk_label(ctx, "Les données se sauvargent uniquement à la fin de la partie.", NK_TEXT_CENTERED);
            }   
        }

        nk_end(ctx);

        // Dessine les petits blocs de chargement. 
        for(int i = 0; i < nbbloc; ++i){
            renderManager->draw_filled_rectangle(10 + (i * blocWidth), windowHeight - blocHeight, 30,55,50,100 + i * gradient,50,255);
        }

        // Création du monde.
        if (nbbloc == 10)
        {
            const string mapName = "../maps/test.json";
            mapWorld->loadMap(mapName);
            mapWorld->getMapDimension(&mapW,&mapH);
        }

        // Mise en place de la bordure.
        else if (nbbloc == 20)
        {
            factory->setupBorder(mapW,mapH);
        }

        // Ajout des entités.
        else if (nbbloc == 30)
        {
            initializePlayer();
            const float playerWidth = PLAYERSIZE_W;
            const float playerHeigth = PLAYERSIZE_H;
            /* Instanciation des instances statiques du monde (comme le joueur) */
            int numberArcher = 30 + rand() % 21;
            int numberSkeleton = 30 + rand() % 21;
            for(int i = 0;i<numberArcher;i++){
                float x = 1.0 + static_cast<float> (rand()) / (static_cast<float> (RAND_MAX/(mapW/PPM - 1.0)));
                float y = 1.0 + static_cast<float> (rand()) / (static_cast<float> (RAND_MAX/(mapH/PPM - 1.0)));
                entityFactory->makeArcherSkeleton(x,y,playerWidth,playerHeigth,this->lvlGame);
            }
            for(int i = 0;i<numberSkeleton;i++){
                float x = 1.0 + static_cast<float> (rand()) / (static_cast<float> (RAND_MAX/(mapW/PPM - 1.0)));
                float y = 1.0 + static_cast<float> (rand()) / (static_cast<float> (RAND_MAX/(mapH/PPM - 1.0)));
                entityFactory->makeSkeleton(x,y,playerWidth,playerHeigth,this->lvlGame);
            }
        }

        // À chaque itération il y a un nouveau bloc.
        nbbloc += 1;

        renderManager->renderGui();
        renderManager->renderPresent();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

/**
 * @brief Chargement du lobby. 
 * Lors de la première itération du jeu, les joueurs sont propulsés dans le lobby.
 */
void WorldManager::loadLobby()
{
    
}

/**
 * @brief Rajoute une animation à l'entité.
 * 
 * @param filepath Chemin vers la texture avec l'extension.
 * @param nbFrames nombre de frames de la texture.
 * @param h Height.
 * @param w Width.
 * @param state L'état associé à l'animation.
 * @param speed Vitesse entre les frames.
 */
void WorldManager::addAnimationToEntity(Entity* entity, const string& filepath, int nbFrames, int h, int w, EntityState state,float speed){
    entity->addAnimation(state, speed);
    Animation& animation = entity->getAnimation(state);
    animation.setTextureWidth(w);
    animation.setTextureHeight(h);

    vector<Texture> animTextures;
    renderManager->parseTexture(
        animTextures,
        IMG_LoadTexture(renderManager->getRenderer(), filepath.c_str()),
        w,h,
        nbFrames
    );

    for(Texture frame : animTextures){
        SDL_SetTextureBlendMode(frame, SDL_BLENDMODE_BLEND);
        animation.addFrame(frame);
    }
}

/**
 * @brief Récupère le joueur le plus proche d'une certaine entité.
 * 
 * @param entity 
 * @return Player* 
 */
Player* WorldManager::getClosestPlayer(Entity* entity){
    float distance = INFINITY;
    float distanceBefore = INFINITY;
    Player* playerToReturn = nullptr;
    for(Player player : players){
        distance = min(fabs(b2Distance(player.getBody()->GetPosition(),player.getBody()->GetPosition())),distance);
        if(distance > distanceBefore) {
            distanceBefore = distance; 
            playerToReturn = &player;
        }
    }
    return playerToReturn;
}

/**
 * @brief Calcul pour recentrer l'image par rapport à la hitbox.
 * 
 * @param entity 
 * @param coordonnees 
 * @param animation 
 * @return Vec2 
 */
Vec2 WorldManager::centerTextureOnHitBox(Entity* entity, const Vec2& coordonnees, Animation& animation){
    int w0 = entity->getWidth() * PPM;
    int w1 = animation.getTextureWidth();
    
    float dw = (w1 - w0) * 0.5;
    int h0 = entity->getHeight() * PPM;
    int h1 = animation.getTextureHeight();
   
    float dh = (h1 - h0) * 0.5;
    return Vec2(coordonnees.x - dw, coordonnees.y - dh);
}