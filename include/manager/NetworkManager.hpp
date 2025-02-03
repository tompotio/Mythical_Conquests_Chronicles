#pragma ONCE
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <iostream> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <string.h>

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "RenderManager.hpp"
#include "../utils/include/Timer.hpp"
#include "../../libs/qrcodegen/include/qrcodegen.hpp"
#include "../../libs/RapidJson/include/rapidjson/document.h"
#include "../../libs/RapidJson/include/rapidjson/writer.h"
#include "../../libs/RapidJson/include/rapidjson/stringbuffer.h"

using namespace std;
using namespace qrcodegen;

struct Splayer {
    TCPsocket playerSocket;
    int id;
    string name;
    Splayer(TCPsocket socket, int id, const std::string& name) : playerSocket(socket), id(id), name(name) {}
};

/**
 * @brief Classe qui s'occupe de l'aspect réseau du jeu.
 * 
 */
class NetworkManager
{
    public:
        static NetworkManager& getInstance();
       
        void startApplicationServer();
        void closeApplicationServer();
        void acceptApplicationClient();
        void startGameServer();
        void startGameClient();
        void joinServer();
        void checkForNewPlayer(TCPsocket* socket, short int* id);
        
        void Client_SendDatas(const char* datas, size_t dataLength);
        void Server_SendDatas(const char* datas, size_t dataLength,TCPsocket socket = nullptr, bool ignoreAll = false);

        void getDataFromApplicationCLient();
        string getApplicationAddress();
        
        char* Client_ReceiveDatas();
        vector<vector<char>> Server_ReceiveDatas();

        string generateQrCode(string address);

        NetworkManager(){
            if (SDLNet_Init() < 0) {
                std::cerr << "SDLNet initialization failed: " << SDLNet_GetError() << std::endl;
                SDL_Quit();
            }
            playerZeroSetSocket = SDLNet_AllocSocketSet(1);
            clientSet = SDLNet_AllocSocketSet(1);
            setJoueurs = SDLNet_AllocSocketSet(4);
        }

        ~NetworkManager(){
            for (Splayer player : joueurs) {
                SDLNet_TCP_Close(player.playerSocket);
            }
            SDLNet_TCP_Close(playerZeroSocket);
            SDLNet_Quit();
        }
        
    private:
        void removePlayers();
        string getData(TCPsocket);
        static NetworkManager* instance;
        TCPsocket applicationServer = NULL;
        TCPsocket applicationClient = NULL;
        TCPsocket applicationTmp = NULL;
        string applicationAddress;
        int applicationPort;
        SDLNet_SocketSet clientSet;
        int cpt = 0;

        IPaddress ip;
        Uint16 PORT = 1234;

        TCPsocket playerZeroSocket;
        SDLNet_SocketSet playerZeroSetSocket;
        std::vector<TCPsocket> joueursASupprimer;

        std::vector<Splayer> joueurs;
        SDLNet_SocketSet setJoueurs;
        bool active = true;
        Timer timerActive;
        // Quantité maximale de données que l'on peut recevoir.
        int maxlen = 512;
};




#endif