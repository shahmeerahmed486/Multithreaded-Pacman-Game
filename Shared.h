#ifndef SHARED_H
#define SHARED_H

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

const int TILE_SIZE = 30;
sem_t keys;
sem_t exitPermits;
struct Global {
    sf::Sprite playerSprite;
    int playerSpeed = 5;
    int lives = 3;
    char playerDirection = 'r';
    std::vector<std::string> gameMap;
    bool gameRunning = true;
    std::vector<sf::Sprite> ghostSprites = {sf::Sprite(), sf::Sprite(), sf::Sprite(), sf::Sprite()};
    std::vector<char> ghostDirections = {'u', 'd', 'l', 'r'};
    std::vector<char> secondaryDirections = {'d', 'u', 'r', 'l'}; 
    std::vector<int> ghostSpeeds = {3, 2, 3, 2}; 
    std::vector<std::string> ghostImages = {"images/ghost-red.png", "images/ghost-orange.png", "images/ghost-pink.png", "images/ghost-blue.png"}; // Added ghost images
    sf::Texture ghostTextures[4]; 

    sf::Texture pacmanTexture[4];
    
    pthread_mutex_t playerMutex;
    pthread_mutex_t playerDirectionMutex;
    pthread_mutex_t ghostMutex;
    pthread_mutex_t ghostSpriteMutex[4];
    pthread_mutex_t speedBoostMutex;


    sf::Sprite* wallSprites = nullptr;
    sf::FloatRect* wallBounds = nullptr;
    int NUM_BRICKS;
    sf::Texture wallTexture;

    int score = 0; 
    std::vector<sf::Sprite> pellets; //vector of pellets
    sf::Texture pelletTexture;

    std::vector<sf::Sprite> fruits; //vector of power-ups
    sf::Texture fruitTexture;
    bool powerMode = false;
    sf::Clock powerTimer;
    sf::Time maxPowerTime = sf::seconds(5.0f);

    std::vector<sf::Sprite> speedBoosts;
    sf::Texture speedBoostTexture;
    std::vector<sf::Vector2f> speedBoostPositions = { {7 * TILE_SIZE, 14 * TILE_SIZE}, {13 * TILE_SIZE, 23 * TILE_SIZE} };
    sf::Vector2f lastEatenBoostPosition;

    void setInitiaGhosts(){
        resetGhostTextures();
        for (int i=0;i<4;i++){
            ghostInhouse(i);
        }
    }

    void ghostInhouse(int ghostNumber){
        ghostSprites[ghostNumber].setPosition((12+ghostNumber) * TILE_SIZE, (14) * TILE_SIZE); 
    }

    void resetGhostTextures() {
        for (int i = 0; i < 4; i++) {
            ghostTextures[i].loadFromFile(ghostImages[i]);
            ghostSprites[i].setTexture(ghostTextures[i]);
            ghostSprites[i].setScale(1.0f, 1.0f);
        }
    }

    void setBlueGhost(){
        for (int i = 0; i < 4; i++) {
            ghostTextures[i].loadFromFile("images/blueGhost.png");
            ghostSprites[i].setTexture(ghostTextures[i]);
        }
    }
    
    void initializePacTexture() {
        pacmanTexture[0].loadFromFile("images/pacman-up.png");
        pacmanTexture[1].loadFromFile("images/pacman-down.png");
        pacmanTexture[2].loadFromFile("images/pacman-left.png");
        pacmanTexture[3].loadFromFile("images/pacman-png.png");
        playerSprite.setTexture(pacmanTexture[3]);
        playerSprite.setScale(1.0f, 1.0f);
        playerSprite.setPosition(2 * TILE_SIZE, 29 * TILE_SIZE);
    }
    
    void cleanup() {
        delete[] wallSprites;
        delete[] wallBounds;
    }
    
};

extern struct Global global;

#endif // SHARED_H

