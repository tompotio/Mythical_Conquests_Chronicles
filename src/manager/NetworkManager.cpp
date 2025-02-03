#include "../../include/manager/NetworkManager.hpp"
using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;

NetworkManager* NetworkManager::instance = nullptr;

NetworkManager& NetworkManager::getInstance(){
    if(!instance){
        instance = new NetworkManager();
    }
    return *instance;
}

/**
 * @brief Lance le serveur.
 * 
 */
void NetworkManager::startGameServer(){
    if (SDLNet_ResolveHost(&ip, NULL, PORT) == -1) {
        std::cerr << "Impossible de résoudre l'hôte : " << SDLNet_GetError() << std::endl;
    }

    playerZeroSocket = SDLNet_TCP_Open(&ip);

    if (!playerZeroSocket) {
        std::cerr << "Impossible de se connecter au serveur : " << SDLNet_GetError() << std::endl;
    }

    cout << "Serveur créé !" << endl;
}

/**
 * @brief Rejoint un serveur.
 * 
 */
void NetworkManager::startGameClient(){
    if (SDLNet_ResolveHost(&ip, "localhost", PORT) == -1) {
        std::cerr << "Impossible de résoudre l'hôte : " << SDLNet_GetError() << std::endl;
    }

    playerZeroSocket = SDLNet_TCP_Open(&ip);

    if (!playerZeroSocket) {
        std::cerr << "Impossible de se connecter au serveur : " << SDLNet_GetError() << std::endl;
    }
    
    SDLNet_TCP_AddSocket(playerZeroSetSocket,playerZeroSocket);

    cout << "Connecté au serveur" << endl;
}

/**
 * @brief Cherche si un nouveau joueur souhaite se connecter et renvoie l'id du nouveau joueur.
 * 
 */
void NetworkManager::checkForNewPlayer(TCPsocket* socket, short int* id){
    TCPsocket joueur = SDLNet_TCP_Accept(playerZeroSocket);
    if (joueur) {
        joueurs.emplace_back(joueur, cpt, "");
        SDLNet_TCP_AddSocket(setJoueurs, joueur);
        cpt += 1;
        *id = cpt;
        *socket = joueur;
    }
}

/**
 * @brief Fonction réservée au client : envoie des données au serveur.
 */
void NetworkManager::Client_SendDatas(const char* datas, size_t dataLength){
    if (SDLNet_TCP_Send(playerZeroSocket, datas, dataLength) < dataLength) {
        std::cerr << "Erreur lors de l'envoi des données : " << SDLNet_GetError() << std::endl;
    }
}

/**
 * @brief Fonction réservée au serveur : envoie des données aux clients.
 * Optionnellement on peut mettre un socket. : 
 *      - Si ignoreAll est à true, ignore tous les joueurs sauf le socket en option.
 *      - Si ignoreAll est à false, ignore le socket optionnel non null.
 *      - Sinon envoie à tout le monde (par défaut lorsque le socket optionnel est null).
 * Cette option ne s'applique que à la fonction Server_SendDatas() pas à Client_SendDatas, car celle-ci se charge d'envoie les données que au serveur.
 * 
 * @param datas 
 * @param dataLength 
 * @param socket 
 * @param ignoreAll 
 */
void NetworkManager::Server_SendDatas(const char* datas, size_t dataLength,TCPsocket socket, bool ignoreAll){
    if(socket){
        if(ignoreAll){
            if (SDLNet_TCP_Send(socket, datas, dataLength) < dataLength) {
                std::cerr << "Erreur lors de l'envoi des données : " << SDLNet_GetError() << std::endl;
            }
        }
        else{
            for(Splayer joueur : joueurs){
                if(joueur.playerSocket != socket){
                    if (SDLNet_TCP_Send(joueur.playerSocket, datas, dataLength) < dataLength) {
                        std::cerr << "Erreur lors de l'envoi des données : " << SDLNet_GetError() << std::endl;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        for(Splayer joueur : joueurs){
            if (SDLNet_TCP_Send(joueur.playerSocket, datas, dataLength) < dataLength) {
                std::cerr << "Erreur lors de l'envoi des données : " << SDLNet_GetError() << std::endl;
                break;
            }
        }
    }
}

/**
 * @brief Récupère simplement les données envoyées par le serveur.
 * Renvoie soit un tableau de caractères, soit '/0' soit nullptr.
 * Si c'est nullptr, c'est qu'il y a eu une erreur.
 * Si c'est vide, c'est qu'il n'y avait rien à récupérer.
 * Et sinon renvoie le contenu des données.
 */
char* NetworkManager::Client_ReceiveDatas() {
    // Je vérifie d'abord qu'il y a quelque chose à récupérer.
    if (SDLNet_CheckSockets(playerZeroSetSocket, 0) == 1) {
        char datas[maxlen];
        if (datas == nullptr) {
            std::cerr << "Erreur d'allocation mémoire." << std::endl;
            return nullptr;
        }

        int nb_recus = SDLNet_TCP_Recv(playerZeroSocket, datas, maxlen);
        if (nb_recus <= 0) {
            if (nb_recus == 0) {
                std::cout << "Le serveur s'est fermé" << std::endl;
                // Il va falloir throw un truc pour que le client se déconnecte du mode en ligne ici.
            } 
            else {
                std::cerr << "Erreur lors de la réception des données : " << SDLNet_GetError() << std::endl;
            }
            return nullptr;
        }
        
        // Alloue dynamiquement de la mémoire pour le résultat
        char* result = new char[nb_recus];
        if (result == nullptr) {
            std::cerr << "Erreur d'allocation mémoire pour le résultat." << std::endl;
            return nullptr;
        }

        std::memcpy(result, datas, nb_recus);
        return result;
    }
    
    // S'il n'y a rien, on renvoie une chaîne de caractères vide
    else {
        char* vide = new char[1];
        vide[0] = '\0'; // Ajoute le caractère nul de fin de chaîne
        return vide;
    }
}

/**
 * @brief Récupère les données envoyées par un joueur et en même temps les renvoie aux autres.
 */
vector<vector<char>> NetworkManager::Server_ReceiveDatas() {
    vector<vector<char>> datasbuffer;

    char datas[maxlen];
    if (datas == nullptr) {
        cerr << "Erreur d'allocation mémoire." << endl;
        return datasbuffer;
    }

    if (SDLNet_CheckSockets(setJoueurs, 0) > 0) {
        for (Splayer player : joueurs) {
            const TCPsocket sock = player.playerSocket;
            if (SDLNet_SocketReady(sock) != 0) {
                int len = SDLNet_TCP_Recv(sock, datas, maxlen);

                // Récupère les données à renvoyer aux autres joueurs.
                if (len > 0) {
                    vector<char> result(datas, datas + len);
                    for (size_t j = 0; j < joueurs.size(); ++j) {
                        if (sock != joueurs[j].playerSocket) {
                            SDLNet_TCP_Send(joueurs[j].playerSocket, result.data(), len);
                        }
                    }

                    datasbuffer.push_back(result);
                } 
                else if (len <= 0) {
                    joueursASupprimer.push_back(sock);
                }
            }
        }
    }
    return datasbuffer;
}

/**
 * @brief Parcourt la liste des joueurs à supprimer et les supprime.
 * 
 */
void NetworkManager::removePlayers(){
    if(joueursASupprimer.size() > 0){
        for(TCPsocket sock : joueursASupprimer){
            std::cout << "Joueur déconnecté" << std::endl;
            SDLNet_TCP_DelSocket(setJoueurs,sock);
            SDLNet_TCP_Close(sock);

            // Application de l'diome erase-remove_if
            joueurs.erase(
                std::remove_if(
                    joueurs.begin(), 
                    joueurs.end(), 
                    [&](Splayer p) { 
                        return p.playerSocket == sock; 
                    }
                )
            );

            if(joueurs.size() == 0){
                cout << "Il n'y a plus de joueurs" << endl;
            }
        }
        joueursASupprimer.clear();
    }
}

/**
 * @brief Rejoint un serveur.
 * 
 */
void NetworkManager::joinServer(){
    if (SDLNet_ResolveHost(&ip, "localhost", 1234) == -1) {
        std::cerr << "Impossible de résoudre l'hôte : " << SDLNet_GetError() << std::endl;
    }
    
    playerZeroSocket = SDLNet_TCP_Open(&ip);
    if (!playerZeroSocket) {
        std::cerr << "Impossible de se connecter au serveur : " << SDLNet_GetError() << std::endl;
    }
}

/**
 * @brief Lance l'application du serveur.
 * 
 */
void NetworkManager::startApplicationServer(){
    if(!applicationServer){
        cout << "je lance le serveur" << endl;
        IPaddress ip, *ipadd;
        this->applicationPort = 1300;
        unsigned int result;
        if(SDLNet_ResolveHost(&ip,NULL,1300) != 0) {
            printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        }

        this->applicationAddress=SDLNet_ResolveIP(&ip);
        cout << this->applicationAddress << endl;
       
        applicationServer = SDLNet_TCP_Open(&ip);
        if(applicationServer == NULL){
            cout << "erreur à la création " << SDLNet_GetError(); 
        }
        getApplicationAddress();
    }
}
string NetworkManager::getApplicationAddress(){
           /* string adresse;
            adresse.append(this->applicationAddress);
            adresse.append(":");
            adresse.append(std::to_string(applicationPort));
            return adresse;*/
            
            return "";
        }
string NetworkManager::getData(TCPsocket socket){
            char length[50];
            int recvLength = SDLNet_TCP_Recv(socket,length,50);
            while (recvLength <50)
            {
                recvLength += SDLNet_TCP_Recv(socket,length,50-recvLength);
            }
            int messageLength = stoi(length);
            char data[messageLength];
            recvLength = SDLNet_TCP_Recv(socket,data,messageLength);
            while(recvLength < messageLength){
                recvLength+= SDLNet_TCP_Recv(socket,data,messageLength-recvLength);
            }
            string message(data);
            return message;
}
void NetworkManager::acceptApplicationClient(){

    if(applicationServer){
     
        //cout << "je vérifie que quelqu'un veut entrer" << endl;
       if(!applicationClient){
            this->applicationClient = SDLNet_TCP_Accept(applicationServer);
            if(applicationClient){
                timerActive.reset();
                active =true;
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                writer.StartObject();
                writer.Key("type");
                writer.String("TTL");
                writer.Key("data");
                writer.String("bienvenue");
                writer.EndObject();
                cout << "quelqu'un s'est connécté"<< endl;;
                SDLNet_TCP_AddSocket(clientSet,applicationClient);
                int sendLength = SDLNet_TCP_Send(applicationClient,buffer.GetString(),buffer.GetLength());
                cout << "donnees envoye " << sendLength << endl;
                if(sendLength !=buffer.GetLength()){
                    cout << "error" << SDLNet_GetError();
                }
                char datarec[maxlen];
                SDLNet_CheckSockets(clientSet, 1000);
                
                
            }
        }
        else{
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            writer.StartObject();
                writer.Key("type");
                writer.String("ttl");
                writer.Key("data");
                writer.String("");
            writer.EndObject();
            SDLNet_TCP_Send(this->applicationClient,buffer.GetString(),buffer.GetLength());
            SDLNet_CheckSockets(clientSet, 1000);
            this->active = SDLNet_SocketReady(applicationClient);
            this->applicationTmp = SDLNet_TCP_Accept(applicationServer);
            if(this->applicationTmp){
                rapidjson::StringBuffer bufferTMP;
                rapidjson::Writer<rapidjson::StringBuffer> writerTMP(bufferTMP);
                writerTMP.StartObject();
                    writerTMP.Key("type");
                    writerTMP.String("notification");
                    writerTMP.Key("data");
                    writerTMP.String("");

                writerTMP.EndObject();
                SDLNet_TCP_Send(this->applicationTmp,bufferTMP.GetString(),bufferTMP.GetLength());
                SDLNet_TCP_Close(applicationTmp);
            }
            
        }
    }

}
void NetworkManager::getDataFromApplicationCLient(){
    char data[this->maxlen];
}
void NetworkManager::closeApplicationServer(){
    SDLNet_TCP_Close(applicationServer);
}
/**
 * @brief Génère un string du QrCode.
 * 
 * @param qr 
 * @param border 
 * @return std::string 
 */
static std::string toSvgString(const QrCode &qr, int border) {
	if (border < 0)
		throw std::domain_error("Border must be non-negative");
	if (border > INT_MAX / 2 || border * 2 > INT_MAX - qr.getSize())
		throw std::overflow_error("Border too large");
	
	string sb;
	sb.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	sb.append("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
	sb.append("<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 ");
	sb.append(std::to_string(qr.getSize() + border * 2));
    sb.append(" ");
    sb.append(std::to_string(qr.getSize() + border * 2));
    sb.append("\" stroke=\"none\">\n");
	sb.append("\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n");
	sb.append("\t<path d=\"");
	for (int y = 0; y < qr.getSize(); y++) {
		for (int x = 0; x < qr.getSize(); x++) {
			if (qr.getModule(x, y)) {
				if (x != 0 || y != 0)
					sb.append(" ");
				    sb.append("M");
                    sb.append(std::to_string(x + border));
                    sb.append(",");
                    sb.append(std::to_string(y + border));
                    sb.append("h1v1h-1z");
			}
		}
	}
	sb.append("\" fill=\"#000000\"/>\n");
	sb.append("</svg>\n");
	return sb;
}

/**
 * @brief Génère un Qr code.
 * 
 * @param adresse 
 * @param port 
 * @return string 
 */
string NetworkManager::generateQrCode(string adresse){
   
    
    QrCode qr0 = QrCode::encodeText(adresse.c_str(), QrCode::Ecc::MEDIUM);
    std::string svg = toSvgString(qr0, 4);
    return svg;
}