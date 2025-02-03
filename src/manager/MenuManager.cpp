#include "../../include/manager/MenuManager.hpp" 

MenuManager::MenuManager(Game* game) : game(game){
    RenderManager& rm = RenderManager::getInstance();
    renderManager = &rm;
    background = IMG_LoadTexture(renderManager->getRenderer(), "../assets/background/background.png");
    windowWidth = renderManager->getWindowWidth();
    windowHeight = renderManager->getWindowHeight();
    menu_width = windowWidth * .3;
    menu_height = windowHeight * .75;
    options_width = windowWidth * .8;
    options_height = windowHeight * .8;
    ctx = renderManager->getGuiContext();
    showFPS = 1 - game->showingFPS();
    networkManager = NetworkManager::getInstance();
}

/**
 * @brief Met à jour les tailles des éléments graphiques lorsque l'on redimensionne l'écran, en fonction de leur ratio.
 * 
 */
void MenuManager::updateScreenSize(){
    renderManager->getFullWindowSize(&windowWidth, &windowHeight);
    menu_width = windowWidth * .3;
    menu_height = windowHeight * .75;
    options_width = windowWidth * .8;
    options_height = windowHeight * .8;
}

void MenuManager::render(){
    renderManager->draw_BackgroundImage(background);
    // Effectue le rendu final.
    renderManager->renderGui();
    if(showQrCode){
        
        string qrCode = networkManager.generateQrCode("192.168.1.30:1300");
        Texture qrCodeTexture = renderManager->GenerateQrCode(qrCode, renderManager->getRenderer());
        int w;
        int h;
        SDL_QueryTexture(qrCodeTexture,NULL,NULL,&w,&h);
        SDL_SetRenderDrawColor(renderManager->getRenderer(),0,0,0,255);
        renderManager->draw_Image(qrCodeTexture,windowWidth/2-(w*5),windowHeight/2-(h*5),w*10,h*10,125,0);
    }
}

/**
 * @brief Génère le menu avec les widgets.
 * 
 */
void MenuManager::menu(){
   
    if (nk_begin(ctx, "Menu", nk_rect(
        (windowWidth - menu_width)/ 2, 
        (windowHeight - menu_height) / 2, 
        menu_width, menu_height),
        NK_WINDOW_BORDER |
        NK_WINDOW_NO_SCROLLBAR))
    {
        string startGame = "Nouvelle partie";
        float ratio[] = {0.1f, 0.80f,0.1f};
        int h = menu_height / 15;
        nk_layout_row(ctx, NK_DYNAMIC, h, 3, ratio);

        nk_spacer(ctx);nk_spacer(ctx);nk_spacer(ctx);
        
        nk_spacer(ctx);
        if (nk_button_label(ctx, "Nouvelle Partie")) {
            game->setState(InLoadingScreen);
            
        }
        nk_spacer(ctx);
        
        nk_spacer(ctx);nk_spacer(ctx);nk_spacer(ctx);

        nk_spacer(ctx);
        if (nk_button_label(ctx,"sauvegarder partie")){
            if(game->getLoadedSave()==""){
                int nbSave = 0;
                std::string path = "../save/";
                 for (const auto & entry : std::filesystem::directory_iterator(path)){
                    if(entry.path().extension().compare(".json")==0){
                        nbSave ++;
                    }
                 }
                string nameSave ="../save/";
                nameSave.append("save");
                nameSave.append(std::to_string(nbSave+1));
                nameSave.append(".json");
                game->saveGame(nameSave);
            }
            else{
                game->saveGame(game->getLoadedSave());
            }
        }
        nk_spacer(ctx);
        nk_spacer(ctx);nk_spacer(ctx);nk_spacer(ctx);

        nk_spacer(ctx);
        if (nk_button_label(ctx, "Charger Partie")) {
           showLoadingSave = true;   
        }
        nk_spacer(ctx);

        nk_spacer(ctx);nk_spacer(ctx);nk_spacer(ctx);

        nk_spacer(ctx);
        if (nk_button_label(ctx, "Options")) {
            showOptions = true;
        }
        
          nk_spacer(ctx);

        nk_spacer(ctx);nk_spacer(ctx);nk_spacer(ctx);

        nk_spacer(ctx);

        if (nk_button_label(ctx,"Synchronisation")){
            showQrCode = true;
            this->networkManager.startApplicationServer();
        }
        nk_spacer(ctx);

        nk_spacer(ctx);nk_spacer(ctx);nk_spacer(ctx);

        nk_spacer(ctx);
        if (nk_button_label(ctx, "Quitter")) {
            game->stop();
        }
        nk_spacer(ctx);

        nk_spacer(ctx);nk_spacer(ctx);nk_spacer(ctx);
    }
    nk_end(ctx);

    if(showOptions){
        Options();
    }
    if(showLoadingSave){
        loadingScreen();
    }
    if(showQrCode){
        displayQrCode();
    }  
}
void MenuManager::displayQrCode(){
    if (nk_begin_titled(ctx, "Qrcode", "Qrcode", nk_rect(
            (windowWidth - options_width) / 2, 
            (windowHeight - options_height) / 2, 
            options_width, options_height),
            NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE | NK_WINDOW_NO_SCROLLBAR
    )){
        
    }
    else{
        showQrCode = false;
    }
    nk_end(ctx);
}

string MenuManager::getTitleFromFile(std::filesystem::path filePath){
    std::ifstream file;
    file.open(filePath.string());
    string content;
    string line;
    while (getline(file, line)) { 
        content.append(line); 
        content.append("\n"); 
    } 
    cout << content << endl;
    Document save;
    save.Parse<0>(content.c_str());
    if(save.HasParseError()){
        return "";
    }
    string name(save["character"]["name"].GetString());

    int niveau = save["character"]["lvl"].GetInt(); 
    string saveName;
    saveName.append(name);
    saveName.append("   ");
    saveName.append("lvl");
    saveName.append(std::to_string(niveau));
    file.close();
    return saveName;
   /* vector<string> tokens = getSplitString(content,'\n');
    
    vector<string> infoPlayer; 
    map<string, string> mp;
  
    for (string part : tokens) { 
       vector<string> token = getSplitString(part,':');
       cout << token[0] << ":" << token[1]<<endl;
       if(token[0]!="nom" && !is_number(token[1])){
        cout << "il y a un probleme" <<endl;  
       }
       string index = token[0];
       string value = token[1];
       mp[index]=value;

    } 

    string saveName;
    saveName.append(mp["nom"]);
    saveName.append("   ");
    saveName.append("lvl");
    saveName.append(mp["niveau"]);
    file.close(); 
    return saveName;*/
}
void MenuManager::loadingScreen(){
    if (nk_begin_titled(ctx, "Chargement", "Chargement", nk_rect(
            (windowWidth - options_width) / 2, 
            (windowHeight - options_height) / 2, 
            options_width, options_height),
            NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE
    )){
        float ratio[] = {0.5f, 0.5f};
        
        std::string path = "../save/";
        
        for (const auto & entry : std::filesystem::directory_iterator(path)){
        nk_layout_row_dynamic(ctx,50,1);
        if(entry.path().extension().compare(".json")==0){
            cout << "fichier" << endl;
             string name = getTitleFromFile(entry.path());
             if(name!=""){
                if(nk_button_label(ctx,name.c_str())){
                    game->setLoadedSave(entry.path().string());
                    showLoadingSave = false;
                };
             }

        }
        //ouvrir un fichier et renvoie une string contenant
       
        }
    }
    else{
        showLoadingSave=false;
    }
    nk_end(ctx);
}
void MenuManager::Options(){
    if (nk_begin_titled(ctx, "Options", "Options", nk_rect(
            (windowWidth - options_width) / 2, 
            (windowHeight - options_height) / 2, 
            options_width, options_height),
            NK_WINDOW_BORDER  |NK_WINDOW_CLOSABLE| NK_WINDOW_NO_SCROLLBAR
    ))
    {
       
        float ratio[] = {0.5f, 0.5f};
        nk_layout_row(ctx, NK_DYNAMIC, options_height, 2, ratio);
        
        if (nk_group_begin_titled(ctx,"Ad","Add-ons",NK_WINDOW_TITLE))
        {
            
            nk_layout_row_dynamic(ctx,35,1);
            
            
            if (nk_checkbox_label(ctx, "Afficher les FPS",&showFPS)) {
                game->setShowFPS(1 - showFPS);
            }
            
            nk_spacer(ctx);
            nk_group_end(ctx);
        }
        if (nk_group_begin_titled(ctx,"effet","Effet des branches",NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(ctx,35,1);
            if (nk_checkbox_label(ctx, "Afficher Effet",&afficherEffet)) {
                game->setSpecialEffect(1 - afficherEffet);
            }
            nk_spacer(ctx);
            nk_group_end(ctx);
        }
    }
    else{
        showOptions = false;
    }
    nk_end(ctx);
}

/**
 * @brief Met à jour la logique du menu.
 * Génère les widgets et donne les instructions aux boutons.
 */
void MenuManager::update(float deltaT){
    SDL_Event e;
    nk_input_begin(ctx);    
    while (SDL_PollEvent(&e)) {
        switch(e.type){
            case SDL_QUIT:
                SDL_DestroyWindow(renderManager->getWindow());
                SDL_Quit();
                exit(EXIT_SUCCESS);
                break;
            case SDL_WINDOWEVENT:
                //resized = true;
                break;
        }
        nk_sdl_handle_event(&e);
    }
    nk_input_end(ctx);
    networkManager.acceptApplicationClient();
    menu();
}

