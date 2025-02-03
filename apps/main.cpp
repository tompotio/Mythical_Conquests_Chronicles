#include "../include/game/Game.hpp"

#define WIDTH 1080
#define HEIGHT 920

using namespace std;

int main() 
{
    std::srand(time(NULL));
    /*FILE* mapFile = fopen("../assets/maps/map.txt", "w");

    // Generate and write the map data to the file
    for (int y = 0; y < 100; ++y) {
        for (int x = 0; x < 100; ++x) {
            // Generate random number between 1 and 3 (inclusive)
            int randomValue = rand() % 3 + 1;
            fprintf(mapFile, "%d", randomValue);
        }
        fprintf(mapFile, "\n");  // Add a newline character after each row
    }

    // Close the map file
    fclose(mapFile);*/

    char gameName[] = "Mythical_Conquest_Chronicles";
    Game* game = new Game(gameName,WIDTH,HEIGHT,true);
    game->gameLoop();
    game->~Game();

    return EXIT_SUCCESS;
}
