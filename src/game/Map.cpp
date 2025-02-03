#include "../../include/game/Game.hpp"


/**
 * Dans un premier temps l'instance Map parcourt les différents fichiers csv qui composent les informations du monde : 
 * 
 * Un premier fichier CSV contient une matrice des tiles du monde.
 *  -> Ces tiles peuvent être dynamiques ou statiques ou sans type.
 * 
 * Un second fichier CSV contient les tiles qui s'afficheront par dessus les entités du monde.
 *  -> Ces tiles sont soient statiques ou normales mais pas dynamiques !
 * 
 * Un troisième fichier qui contient des tiles dynamiques invisibles.
 *  -> Soit une tile dynamique Wall ou alors une tile normale invisible.
 * 
 * Un troisième fichier CSV qui contient les informations des entités du monde (joueur etc).
 * Les infos des tiles sont contenues dans le fichier tiles.Json.
 * 
 * Pour l'instant on va se contenter que de 3 fichiers CSV pour décrire le monde.
 * 
 * 
 * 
 * Donc basiquement : 
 * 
 * La nature d'une map c'est de contenir des tiles.
 * Elle se distingue en 3 layers de tiles.
 * 
 * 
 * 
 * Un json pour les tiles dynamiques, ce sont des tiles dont on veut un comportement.
 * Lorsqu'il y a  par exemple une potentielle collision avec un joueur ou autre chose...
 * 
 * Un json pour les tiles normales. Ce sont juste des tiles qui s'affichent à l'écran.
 * Elles ont juste besoin de renseigner sur une texture.
 * 
 * Ensuite deux id 0 et 1 qui permettent de générer deux tiles uniques. Une dynamique car elle consiste en un mur invisible.
 * Et une tile normale qui est simplement un carré invisible.
 */

Map::Map(Camera* camera) : camera(camera){
    RenderManager& rm = RenderManager::getInstance();
    renderManager = &rm;
    renderer = renderManager->getRenderer();
}

/**
 * @brief Crée une map aléatoire.
 * Si withperlin est faux, alors génère une mosaïque.
 * @param withperlin 
 */
void Map::createRandomMap(bool withperlin, const string& fileName, int rows, int cols)
{
    /**
     * @brief Définition du tableau de tuples contenant les informations sur chaque type de tuile.
     * Un tuple d'information a la forme suivante :
     *      {NomTexture, type, {animation, nbFrames}}
     * NomTexture = nom de la texture avec le format .png.
     * type = Enum TileType.
     * {animation,nbFrames}, si animation est true alors on regarde le nombre de frames qui compose l'animation.
     * Si une tile est de type Ground sans animation alors par défaut elle n'est pas une entité et compose simplement le fond d'écran.
     * Par contre si une tile est de type ground avec une animation ou tout autre type, elle devient donc une entité
     * dynamique du monde et doit être instanciée en tant que telle.
     * Les tiles dynamiques seront réaffichées à chaque frame par dessus la texture statique que l'on a initialisé au début du monde par la classe Map.
     */
    std::vector<std::tuple<std::string, TileType, std::tuple<bool, int, float>>> tilesInfos = {
        {"grass.png", Ground, {false, 0,0}},
        {"rock.png", Wall, {false, 0,0}},
        {"sand.png", Ground, {false, 0,0}},
        {"water.png", Water, {true, 8,0.25f}},
    };

    // Matrice 2D
    std::vector<std::vector<int>> tileMatrix(rows, std::vector<int>(cols));

    // Génère la carte 2D avec deux modes possibles :
    if(withperlin){ 
        // Génération avec perlin (doit être configurée à la main).
        const siv::PerlinNoise::seed_type seed = 123456u;
        const siv::PerlinNoise perlin{ seed };
        for (int x = 0; x < rows; ++x)
        {
            for (int y = 0; y < cols; ++y)
            {
                const double noise = perlin.normalizedOctave2D(x * .1, y * .1, 4);
                if(noise <= -0.5){
                    tileMatrix[x][y] = 1;
                }else if(noise > -0.5 && noise <= -0.3){
                    tileMatrix[x][y] = 3;
                }else if(noise > -0.3 && noise <= 0){
                    tileMatrix[x][y] = 2;
                }else if(noise > 0){
                    tileMatrix[x][y] = 0;
                }
            }
        }
    }
    else{
        std::srand(std::time(nullptr));       
        for (int x = 0; x < rows; ++x)
        {
            for (int y = 0; y < cols; ++y)
            {
                int randomValue = rand() % tilesInfos.size();
                tileMatrix[x][y] = randomValue;
            }
        }
    }

    Document document;
    document.SetObject();
    Document::AllocatorType& allocator = document.GetAllocator();

    // Ajout de la matrice 2D au document JSON
    Value matrixArray(kArrayType);
    for (int i = 0; i < rows; ++i) {
        Value row(kArrayType);
        for (int j = 0; j < cols; ++j) {
            row.PushBack(tileMatrix[i][j], allocator);
        }
        matrixArray.PushBack(row, allocator);
    }
    document.AddMember("tileMatrix", matrixArray, allocator);

    // Ajout du tableau de tuples au document JSON
    Value tileInfoArray(kArrayType);
    for (const auto& info : tilesInfos) {
        Value tile(kObjectType);
        tile.AddMember("textureName", Value(get<0>(info).c_str(), allocator).Move(), allocator);
        tile.AddMember("tileType", std::get<1>(info), allocator);

        Value animatedArray(kArrayType);
        animatedArray.PushBack(std::get<0>(std::get<2>(info)), allocator);
        animatedArray.PushBack(std::get<1>(std::get<2>(info)), allocator);
        animatedArray.PushBack(std::get<2>(std::get<2>(info)), allocator);
        tile.AddMember("animated", animatedArray, allocator);
        tileInfoArray.PushBack(tile, allocator);
    }
    document.AddMember("tilesInfos", tileInfoArray, allocator);

    // Création du writer JSON
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    document.Accept(writer);

    // Écriture du JSON dans un fichier avec fopen
    FILE* outFile = std::fopen((fileName).c_str(), "w");
    if (outFile != nullptr) {
        std::fprintf(outFile, "%s", buffer.GetString());
        std::fclose(outFile);
        cout << "Succès de la création du monde !" << endl;
    } 
    else {
        cout << "Erreur : Impossible d'ouvrir le fichier pour l'écriture." << endl;
    }
}

/**
 * @brief Génère une matrice 2D de tiles pour créer la map.
 * La fonction est un peu longue mais en gros elle fonctionne de la manière suivante :
 * Dans un premier temps on ouvre les fichiers Json.
 * Ensuite on extrait les données.
 * Puis enfin on parcourt la matrice 2D des entiers qui représente la carte du monde. 
 * Chaque entier représente un indexe de tilesInfos. Grâce à cet indexe on sait de quoi est composé la tile.
 * On procède alors par distinguer une tile dynamique d'une tile statique. 
 * Si la tile est statique, on l'applique bêtement à mapWorldText. Sinon on crée un instance Tile.
 * @param fileName 
 */
void Map::loadMap(const string& fileName) {
    cout << "document TileInfo" << endl;
    // Dans un premier temps on charge le fichier tileInfos.
    Document documentTileInfo = extractJsonDataFromTileInfos();

    cout << "document Map" << endl;
    // Puis dans un second temps on charge le fichier Json de la map.
    Document documentMap = extractJsonDataForMap(fileName);

    // Ensuite on extrait les données.
    std::vector<std::tuple<Texture, TileType, std::tuple<bool, int, float>>> tilesInfos;
    cout << "Extrait les documents" << endl;
    getTilesInfosFromDocument(tilesInfos, documentTileInfo);
    cout << "Extrait les infos du document" << endl;
    getMatriceInfosFromDocument(tilesInfos, documentMap);

    cout << "Succès du chargement du monde !" << endl;
}

void Map::getTilesInfosFromDocument(std::vector<std::tuple<Texture, TileType, std::tuple<bool, int, float>>>& tilesInfos, Document& document) {
    if (document.HasMember("tilesInfos") && document["tilesInfos"].IsArray()) {
        const Value& tileInfoArray = document["tilesInfos"];
        for (SizeType i = 0; i < tileInfoArray.Size(); ++i) {
            // Valeurs à extraire
            TileType tileType;
            Texture texture;
            bool animated = false;
            int nbFrames = 0;
            float speed = 0.0f;

            const Value& tile = tileInfoArray[i];

            // La texture.
            if (tile.HasMember("textureName") && tile["textureName"].IsString()) {
                string cheminTexture = tile["textureName"].GetString();
                texture = IMG_LoadTexture(renderer, cheminTexture.c_str());
                if (!texture) {
                    cerr << "Erreur lors du chargement de la texture : " << cheminTexture << endl;
                    continue;
                }
            } else {
                cerr << "textureName corrompu !" << endl;
                continue;
            }

            // Le type de tile.
            if (tile.HasMember("tileType") && tile["tileType"].IsInt()) {
                tileType = static_cast<TileType>(tile["tileType"].GetInt());
            } else {
                cerr << "tileType corrompu !" << endl;
                continue;
            }

            // Animated.
            if (tile.HasMember("animated") && tile["animated"].IsArray()) {
                const Value& array = tile["animated"].GetArray();
                if (array.Size() == 3 && array[0].IsBool() && array[1].IsInt() && array[2].IsFloat()) {
                    animated = array[0].GetBool();
                    nbFrames = array[1].GetInt();
                    speed = array[2].GetFloat();
                } else {
                    cerr << "animated array corrompu !" << endl;
                    continue;
                }
            } else {
                cerr << "animated array corrompu !" << endl;
                continue;
            }

            tilesInfos.push_back(make_tuple(texture, tileType, make_tuple(animated, nbFrames, speed)));
        }
    } else {
        cerr << "Erreur : 'tilesInfos' est manquant ou n'est pas un tableau dans le JSON." << endl;
    }
}

void Map::getMatriceInfosFromDocument(std::vector<std::tuple<Texture, TileType, std::tuple<bool, int, float>>>& tilesInfos, Document& document) {
    // Garde le renderer de côté.
    Texture rtex = SDL_GetRenderTarget(renderer);

    if (document.HasMember("tileMatrix") && document["tileMatrix"].IsArray()) {
        const Value& matrixArray = document["tileMatrix"];
        if (matrixArray.Size() == 0 || !matrixArray[0].IsArray()) {
            cerr << "Erreur : 'tileMatrix' est vide ou mal formaté." << endl;
            return;
        }
        this->w = matrixArray.Size() * TILESIZE;
        this->h = matrixArray[0].GetArray().Size() * TILESIZE;

        for (SizeType x = 0; x < matrixArray.Size(); ++x) {
            const Value& row = matrixArray[x];
            for (SizeType y = 0; y < row.GetArray().Size(); ++y) {

                if (y >= row.GetArray().Size() || !row[y].IsInt()) {
                    cerr << "Erreur : valeur dans 'tileMatrix' est manquante ou non entière." << endl;
                    continue;
                }

                const int val = row[y].GetInt();
                if (val < 0 || static_cast<size_t>(val) >= tilesInfos.size()) {
                    cerr << "Erreur : index de tile invalide dans 'tileMatrix'." << endl;
                    continue;
                }

                const Texture tileTexture = std::get<0>(tilesInfos[val]);
                const bool animated = std::get<0>(std::get<2>(tilesInfos[val]));
                const int nbFrames = std::get<1>(std::get<2>(tilesInfos[val]));
                const float speed = std::get<2>(std::get<2>(tilesInfos[val]));
                TileType tileType = std::get<1>(tilesInfos[val]);

                // Si cas tuile statique
                if (tileType == TileType::Ground && !animated) {
                    tiles.emplace_back(x, y, tileTexture);
                    continue;
                }

                // Sinon on prépare une tuile dynamique
                vector<Texture> animTextures;

                // Découper la texture en plusieurs sous-textures si nbFrames > 1
                if (nbFrames > 1) {
                    renderManager->parseTexture(animTextures, tileTexture, TILESIZE, TILESIZE, nbFrames);
                } 
                else 
                {
                    animTextures.push_back(tileTexture);
                }

                Animation* animation = nullptr;

                // Ensuite on génère notre tile.                
                if (tileType == TileType::Ground) { // Création de la tile dynamique
                    TileStatique newTile(x, y);
                    staticTiles.push_back(newTile);
                    newTile.addAnimation(speed);
                    animation = &(newTile.getAnimation());
                } else {
                    b2PolygonShape shape;
                    shape.SetAsBox(TILESIZE / (PPM * 2.0f), TILESIZE / (PPM * 2.0f), Vec2(TILESIZE / (PPM * 2.0f), TILESIZE / (PPM * 2.0f)), 0.0f);
                    TileDynamique* newTile = nullptr;
                    // Appelle le bon constructeur en fonction de l'Enum 
                    switch (tileType) {
                        case Wall:
                            newTile = new TileWall(x, y, TILESIZE, TILESIZE, 0.0f, 0.0f, &shape, Static);
                            break;
                        case Water:
                            newTile = new TileWater(x, y, TILESIZE, TILESIZE, 0.0f, 0.0f, &shape, Static);
                            break;
                        default:
                            break;
                    }
                    if (newTile) {
                        newTile->setType(tileType);
                        newTile->setX(x);
                        newTile->setY(y);
                        dynamicTiles.push_back(newTile);
                        newTile->addAnimation(0, speed);
                        animation = &(newTile->getAnimation(0));

                        // Instancier un seul timer pour les tiles Water.
                        if (tileType == Water) {
                            if (!waterTimer) {
                                waterTimer = new Timer(speed);
                            }
                            animation->setTimer(waterTimer);
                        }
                    }
                }

                if (!animation) {
                    cerr << "L'animation est null" << endl;
                }

                for (Texture frame : animTextures) {
                    animation->addFrame(frame);
                }
            }
        }
    } else {
        cerr << "Erreur : 'tileMatrix' est manquant ou n'est pas un tableau dans le JSON." << endl;
    }

    // Remet le bon renderer.
    SDL_SetRenderTarget(renderer, rtex);
}

Document Map::extractJsonDataFromTileInfos() {
    // Ouvrir le fichier JSON en lecture
    ifstream file("../maps/tileInfos.json");
    if (!file.is_open()) {
        cerr << "Erreur : Impossible d'ouvrir le fichier TileInfos.json." << endl;
        exit(EXIT_FAILURE);
    }

    // Lecture du contenu du fichier JSON dans une chaîne
    string jsonString((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    // Analyse du contenu JSON
    Document document;
    if (document.Parse(jsonString.c_str()).HasParseError()) {
        cerr << "Erreur : Impossible de parser le fichier TileInfos.json." << endl;
        exit(EXIT_FAILURE);
    }

    return document;
}

Document Map::extractJsonDataForMap(const string& fileName) {
    // Ouvrir le fichier JSON en lecture
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Erreur : Impossible d'ouvrir le fichier JSON." << endl;
        exit(EXIT_FAILURE);
    }

    // Lecture du contenu du fichier JSON dans une chaîne
    string jsonString((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    // Analyse du contenu JSON
    Document document;
    if (document.Parse(jsonString.c_str()).HasParseError()) {
        cerr << "Erreur : Impossible de parser le fichier JSON." << endl;
        exit(EXIT_FAILURE);
    }

    return document;
}

void Map::getMapDimension(int* w, int* h){
    *w = this->w;
    *h = this->h;
}

bool isTileVisible(Tile& tile, Camera& camera) {
    return tile.getX() >= camera.x - (TILESIZE / PPM) && tile.getX() <= camera.x + static_cast<int>(camera.w / PPM) + (TILESIZE / PPM) &&
           tile.getY() >= camera.y - (TILESIZE / PPM) && tile.getY() <= camera.y + static_cast<int>(camera.h / PPM) + (TILESIZE / PPM);
}

bool isDynamicTileVisible(TileDynamique* tile, Camera& camera){
    return tile->getX() >= camera.x - (TILESIZE / PPM) && tile->getX() <= camera.x + static_cast<int>(camera.w / PPM) + (TILESIZE / PPM) &&
           tile->getY() >= camera.y - (TILESIZE / PPM) && tile->getY() <= camera.y + static_cast<int>(camera.h / PPM) + (TILESIZE / PPM);
}

bool isTileWithinRadius(Tile& tile, Vec2 playerPosition, float radius) {
    float dx = tile.getX() - playerPosition.x;
    float dy = tile.getY() - playerPosition.y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

bool isDynamicTileWithinRadius(TileDynamique* tile, Vec2 playerPosition, float radius) {
    float dx = tile->getX() - playerPosition.x;
    float dy = tile->getY() - playerPosition.y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

/**
 * @brief Fonction render de la classe Map.
 * La fonction parcourt les différents types de tiles et demande au renderManager de les afficher à l'aide des fonctions de dessin.
 * Elle effectue également des calculs de collision pour n'afficher que les tiles dans le champ du joueur (taille de l'écran). 
 */
void Map::render() {

    for(Tile& tile : tiles) 
    {
        if (isTileVisible(tile, *camera)) {
            if (!nightOn || isTileWithinRadius(tile, playerZero->getPosition(), lightRadius)) {
                Vec2 coordonnees = renderManager->convertPoint(camera, Vec2(tile.getX(), tile.getY()));
                renderManager->draw_Image(tile.getTexture(), coordonnees.x, coordonnees.y, TILESIZEW, TILESIZEW, 255, 0);
            }
        }
    }

    for(TileDynamique* tile : dynamicTiles) 
    {
        if(isDynamicTileVisible(tile, *camera)){
            if (!nightOn || isDynamicTileWithinRadius(tile, playerZero->getPosition(), lightRadius)) {
                Vec2 coordonnees = renderManager->convertPoint(camera, Vec2(tile->getX(), tile->getY()));
                renderManager->draw_Image(tile->getAnimation(0).getCurrentFrame(), coordonnees.x, coordonnees.y, TILESIZEW, TILESIZEW, 255, 0);
            }
        }
    }
    
}

/**
 * @brief 
 * 
 * @param deltaT 
 */
void Map::update(float deltaT){
    waterTimer->update(deltaT);
    for(TileDynamique* tile : dynamicTiles){
        if(!(tile->getType() == Water)){
            tile->getAnimation(0).getTimer()->update(deltaT);
        }
    }
}
