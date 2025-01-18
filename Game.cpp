#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <semaphore.h>

#include "UI.h"
#include "Shared.h"
#include "Ghost.h"

const int MAP_WIDTH = 28;
const int MAP_HEIGHT = 31;

struct Global global;

bool checkCollision(const sf::Sprite& sprite);
void createMap(const std::string& filename);
std::vector<std::string> readMapFromFile(const std::string& filename);
void makePellets();
void handleTeleportation(sf::Sprite& sprite);

int main() {
    createMap("MAP.txt");
    global.setInitiaGhosts();
    
    sem_init(&keys, 0, 2);
    sem_init(&exitPermits, 0, 2);
    
    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH * TILE_SIZE, (MAP_HEIGHT+2) * TILE_SIZE), "Pac-Man");

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("images/pacman_map.png")) {
        std::cerr << "Error loading background image" << std::endl;
        return -1;
    }
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);
    backgroundSprite.setScale(1.87f, 1.87f);
    backgroundSprite.setPosition(-0.25*TILE_SIZE,-3.25*TILE_SIZE);

    sf::Font font;
    if (!font.loadFromFile("fonts/Sansation_Bold.ttf")) { 
        std::cerr << "Error loading font" << std::endl;
        return -1;
    }
    global.initializePacTexture();
    
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(48);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(0 * TILE_SIZE, (MAP_HEIGHT) * TILE_SIZE);
    scoreText.setString("Score: 0");

    sf::Text endGameText;
    endGameText.setFont(font);
    endGameText.setCharacterSize(64);
    endGameText.setFillColor(sf::Color::Red);
    endGameText.setPosition(MAP_WIDTH * TILE_SIZE / 3, MAP_HEIGHT * TILE_SIZE / 2.5);
    endGameText.setString("End Game");

    sf::Text winGameText;
    winGameText.setFont(font);
    winGameText.setCharacterSize(64);
    winGameText.setFillColor(sf::Color::Green);
    winGameText.setPosition(MAP_WIDTH * TILE_SIZE / 3, MAP_HEIGHT * TILE_SIZE / 2.5);
    
    sf::Sprite Lives;
    sf::Texture livesTexture;
    livesTexture.loadFromFile("images/hearts.png");
    Lives.setTexture(livesTexture);
    Lives.setScale(1.0f, 1.0f);
    Lives.setPosition(20 * TILE_SIZE, (MAP_HEIGHT) * TILE_SIZE);

    makePellets();

    int moveX = 0;
    int moveY = 0;

    pthread_t uiThread;
    pthread_create(&uiThread, NULL, uiManager, &window);

    //ghoost threads
    pthread_t ghostThreads[4];
    int ghostNumbers[4] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
        pthread_create(&ghostThreads[i], NULL, ghostController, &ghostNumbers[i]);
    }

    while (window.isOpen()) {
        moveX = 0;
        moveY = 0;

        pthread_mutex_lock(&global.playerDirectionMutex);
        char playerDirection = global.playerDirection;
        pthread_mutex_unlock(&global.playerDirectionMutex);

        if (playerDirection == 'u') {
            moveY = -global.playerSpeed;
        } else if (playerDirection == 'd') {
            moveY = global.playerSpeed;
        } else if (playerDirection == 'l') {
            moveX = -global.playerSpeed;
        } else if (playerDirection == 'r') {
            moveX = global.playerSpeed;
        }

        //check for collisions
        pthread_mutex_lock(&global.playerMutex);
        sf::Vector2f newPosition = global.playerSprite.getPosition();
        newPosition.x += moveX;
        newPosition.y += moveY;
        global.playerSprite.setPosition(newPosition);

        handleTeleportation(global.playerSprite);

        if (checkCollision(global.playerSprite)) {
            //collision detected, restore previous position
            global.playerSprite.setPosition(global.playerSprite.getPosition() - sf::Vector2f(moveX, moveY));
        }
        for (auto it = global.pellets.begin(); it != global.pellets.end(); ) {
            if (global.playerSprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
                it = global.pellets.erase(it);
                global.score += 1;
                scoreText.setString("Score: " + std::to_string(global.score));
            } else {
                ++it;
            }
        }

        if (global.powerMode == true) {
            if (global.powerTimer.getElapsedTime() >= global.maxPowerTime) {
                global.powerMode = false;
                global.resetGhostTextures();
            }
        }

        if (global.powerMode == false) {
            global.powerTimer.restart();
            for (auto it = global.fruits.begin(); it != global.fruits.end();) {
                if (global.playerSprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
                    it = global.fruits.erase(it);
                    global.score += 10;
                    scoreText.setString("Score: " + std::to_string(global.score));
                    global.setBlueGhost();
                    global.powerMode = true;
                    global.powerTimer.restart();
                } else {
                    ++it;
                }
            }
        }

        pthread_mutex_unlock(&global.playerMutex);

        //check for game end
        if (global.pellets.empty() && global.fruits.empty()) {
            winGameText.setString("You Win!\nScore: " + std::to_string(global.score));
            window.clear(sf::Color::Black);
            window.draw(winGameText);
            window.display();
            global.gameRunning=false;
            sf::sleep(sf::seconds(3));
            window.close();
            break;
        }

        window.clear(sf::Color::Black);

        //display walls
        /*
        for (int i = 0; i < global.NUM_BRICKS; i++) {
             window.draw(global.wallSprites[i]);
        }
        */
        
        window.draw(backgroundSprite);
    
        for (const auto& pellet : global.pellets) {
            window.draw(pellet);
        }

        for (const auto& fruit : global.fruits) {
            window.draw(fruit);
        }

        for (const auto& speedBoost : global.speedBoosts) {
            window.draw(speedBoost);
        }

        pthread_mutex_lock(&global.playerMutex);
        window.draw(global.playerSprite);
        pthread_mutex_unlock(&global.playerMutex);

        for (int i = 0; i < global.lives; i++) {
            Lives.setPosition((20 + (i * 2)) * (TILE_SIZE), (MAP_HEIGHT) * TILE_SIZE);
            window.draw(Lives);
        }
        
        for (int i = 0; i < 4; i++) {
            pthread_mutex_lock(&global.ghostSpriteMutex[i]);
            handleTeleportation(global.ghostSprites[i]);
            window.draw(global.ghostSprites[i]);
            pthread_mutex_unlock(&global.ghostSpriteMutex[i]);
        }
    
        window.draw(scoreText);
        if (!global.gameRunning) {
            window.draw(endGameText);
            window.display();
            global.gameRunning=false;
            sf::sleep(sf::milliseconds(2000));
            window.close();
            break;
        }
        
        window.display();
        sf::sleep(sf::milliseconds(20)); 
    }

    global.cleanup();
    sem_destroy(&keys);
    sem_destroy(&exitPermits); 

    return 0;
}

std::vector<std::string> readMapFromFile(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> map;
    std::string line;

    while (std::getline(file, line)) {
        map.push_back(line);
    }

    global.gameMap = map;

    return map;
}

void createMap(const std::string& filename) {
    global.gameMap = readMapFromFile(filename);

    int NUM_BRICKS = 0;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (global.gameMap[y][x] == '1' || global.gameMap[y][x] == '2') {
                NUM_BRICKS++;
            }
        }
    }

    global.NUM_BRICKS = NUM_BRICKS; 

    global.wallTexture.loadFromFile("images/brick.png");

    global.wallSprites = new sf::Sprite[global.NUM_BRICKS];
    global.wallBounds = new sf::FloatRect[global.NUM_BRICKS];

    int wallIndex = 0;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (global.gameMap[y][x] == '1' || global.gameMap[y][x] == '2') {
                global.wallSprites[wallIndex].setTexture(global.wallTexture);
                global.wallSprites[wallIndex].setScale(0.5f, 0.5f); 
                global.wallSprites[wallIndex].setPosition(x * TILE_SIZE, y * TILE_SIZE);
                global.wallBounds[wallIndex] = global.wallSprites[wallIndex].getGlobalBounds(); 
                wallIndex++;
            }
        }
    }
}

void makePellets() {
    global.pelletTexture.loadFromFile("images/fruits.png"); 
    global.fruitTexture.loadFromFile("images/cherry.png"); 
    global.speedBoostTexture.loadFromFile("images/speed_boost.png"); 

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (global.gameMap[y][x] == '0') {
                sf::Sprite pellet(global.pelletTexture);
                pellet.setScale(0.5f, 0.5f);
                pellet.setPosition(x * TILE_SIZE + TILE_SIZE / 2 - pellet.getGlobalBounds().width / 2, y * TILE_SIZE + TILE_SIZE / 2 - pellet.getGlobalBounds().height / 2);
                global.pellets.push_back(pellet);
            }
            else if(global.gameMap[y][x] == '@'){
                sf::Sprite Fruit(global.fruitTexture);
                Fruit.setScale(1.0f, 1.0f);
                Fruit.setPosition(x * TILE_SIZE + TILE_SIZE / 2 - Fruit.getGlobalBounds().width / 2, y * TILE_SIZE + TILE_SIZE / 2 - Fruit.getGlobalBounds().height / 2);
                global.fruits.push_back(Fruit);
            }
        }
    }

    for (const auto& position : global.speedBoostPositions) {
        sf::Sprite speedBoost(global.speedBoostTexture);
        speedBoost.setScale(1.0f, 1.0f);
        speedBoost.setPosition(position);
        global.speedBoosts.push_back(speedBoost);
    }
}

void handleTeleportation(sf::Sprite& sprite) {
    sf::Vector2f position = sprite.getPosition();
    if (position.x < 0) {
        sprite.setPosition(MAP_WIDTH * TILE_SIZE, position.y);
    } else if (position.x > MAP_WIDTH * TILE_SIZE) {
        sprite.setPosition(0, position.y);
    }
}

