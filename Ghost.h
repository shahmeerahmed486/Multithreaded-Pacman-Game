#ifndef GHOST_H
#define GHOST_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <semaphore.h>

#include "Shared.h"

void resetPlayerCoordinates() { 
    global.playerSprite.setPosition(2 * TILE_SIZE, 29 * TILE_SIZE);
    global.lives--;
    if (global.lives <= 0) {
        global.gameRunning = false;
    }
}

void ghostSpawn(int ghostNumber){

    global.ghostInhouse(ghostNumber);
    sem_wait(&keys);
    sem_wait(&exitPermits);

    while (global.gameMap[global.ghostSprites[ghostNumber].getPosition().y / TILE_SIZE+1][global.ghostSprites[ghostNumber].getPosition().x / TILE_SIZE] == '3'||global.gameMap[global.ghostSprites[ghostNumber].getPosition().y / TILE_SIZE+1][global.ghostSprites[ghostNumber].getPosition().x / TILE_SIZE] == '2') {
        global.ghostSprites[ghostNumber].move(0, -global.ghostSpeeds[ghostNumber]);
        sf::sleep(sf::milliseconds(100));
    }

    sem_post(&keys);
    sem_post(&exitPermits);
}

bool checkCollision(const sf::Sprite& sprite);

void* ghostController(void* arg) {
    int ghostNumber = *(int*)arg;
    srand(time(0));

    ghostSpawn(ghostNumber);

    sf::Clock speedBoostClock;
    bool isSpeedBoosted = false;
    sf::Vector2f eatenBoostPosition;

    while (global.gameRunning) {
        pthread_mutex_lock(&global.ghostSpriteMutex[ghostNumber]);
        char direction = global.ghostDirections[ghostNumber];

        sf::Vector2f oldPosition = global.ghostSprites[ghostNumber].getPosition();
        sf::Vector2f newPosition = oldPosition;

        //move the ghost based on primary direction
        if (direction == 'u') {
            newPosition.y -= global.ghostSpeeds[ghostNumber];
        } else if (direction == 'd') {
            newPosition.y += global.ghostSpeeds[ghostNumber];
        } else if (direction == 'l') {
            newPosition.x -= global.ghostSpeeds[ghostNumber];
        } else if (direction == 'r') {
            newPosition.x += global.ghostSpeeds[ghostNumber];
        }

        global.ghostSprites[ghostNumber].setPosition(newPosition);

        if (checkCollision(global.ghostSprites[ghostNumber])) {
            //collision detected, restore previous position
            global.ghostSprites[ghostNumber].setPosition(oldPosition);
            
            //try secondary direction
            direction = global.secondaryDirections[ghostNumber];
            if (direction == 'u') {
                newPosition.y -= global.ghostSpeeds[ghostNumber];
            } else if (direction == 'd') {
                newPosition.y += global.ghostSpeeds[ghostNumber];
            } else if (direction == 'l') {
                newPosition.x -= global.ghostSpeeds[ghostNumber];
            } else if (direction == 'r') {
                newPosition.x += global.ghostSpeeds[ghostNumber];
            }

            global.ghostSprites[ghostNumber].setPosition(newPosition);

            if (checkCollision(global.ghostSprites[ghostNumber])) {
                //collision detected, restore previous position
                global.ghostSprites[ghostNumber].setPosition(oldPosition);
                
                //manually select one of the 2 directions left that are neither primary nor secondary
                char availableDirections[2];
                int numAvailableDirections = 0;
                if (global.ghostDirections[ghostNumber] != 'u' && global.secondaryDirections[ghostNumber] != 'u') {
                    availableDirections[numAvailableDirections++] = 'u';
                }
                if (global.ghostDirections[ghostNumber] != 'd' && global.secondaryDirections[ghostNumber] != 'd') {
                    availableDirections[numAvailableDirections++] = 'd';
                }
                if (global.ghostDirections[ghostNumber] != 'l' && global.secondaryDirections[ghostNumber] != 'l') {
                    availableDirections[numAvailableDirections++] = 'l';
                }
                if (global.ghostDirections[ghostNumber] != 'r' && global.secondaryDirections[ghostNumber] != 'r') {
                    availableDirections[numAvailableDirections++] = 'r';
                }
                direction = availableDirections[rand() % numAvailableDirections];
                
                if (direction == 'u') {
                    newPosition.y -= global.ghostSpeeds[ghostNumber];
                } else if (direction == 'd') {
                    newPosition.y += global.ghostSpeeds[ghostNumber];
                } else if (direction == 'l') {
                    newPosition.x -= global.ghostSpeeds[ghostNumber];
                } else if (direction == 'r') {
                    newPosition.x += global.ghostSpeeds[ghostNumber];
                }

                global.ghostSprites[ghostNumber].setPosition(newPosition);

                if (checkCollision(global.ghostSprites[ghostNumber])) {
                    // If still collides, revert to the old position
                    global.ghostSprites[ghostNumber].setPosition(oldPosition);
                }
            }
        }

        if (!isSpeedBoosted) {
            for (auto it = global.speedBoosts.begin(); it != global.speedBoosts.end(); ++it) {
                if (global.ghostSprites[ghostNumber].getGlobalBounds().intersects(it->getGlobalBounds())) {
                    pthread_mutex_lock(&global.speedBoostMutex);
                   
                    global.ghostSpeeds[ghostNumber] += 1;
                    speedBoostClock.restart();
                    eatenBoostPosition = it->getPosition();
                    global.speedBoosts.erase(it);
                    isSpeedBoosted = true;
                    pthread_mutex_unlock(&global.speedBoostMutex);
                    break;
                }
            }
        }

        pthread_mutex_unlock(&global.ghostSpriteMutex[ghostNumber]);

        if (isSpeedBoosted && speedBoostClock.getElapsedTime().asSeconds() >= 5.0f) {
            pthread_mutex_lock(&global.speedBoostMutex);
            global.ghostSpeeds[ghostNumber] -= 1;
            sf::Sprite speedBoost(global.speedBoostTexture);
            speedBoost.setScale(1.0f, 1.0f);
            speedBoost.setPosition(eatenBoostPosition);
            global.speedBoosts.push_back(speedBoost);
            isSpeedBoosted = false;
            pthread_mutex_unlock(&global.speedBoostMutex);
        }

        pthread_mutex_lock(&global.playerMutex);

        //check if the ghost is eating the player
        if (global.playerSprite.getGlobalBounds().intersects(global.ghostSprites[ghostNumber].getGlobalBounds())) {
            if(global.powerMode==false){
                resetPlayerCoordinates();
            }
            else{
                pthread_mutex_unlock(&global.playerMutex);
                ghostSpawn(ghostNumber);
                pthread_mutex_lock(&global.playerMutex);
                global.score+=50;   //50 points for eating ghost
            }
        }

        //reevaluate primary and secondary direction based on player position
        sf::Vector2f playerPosition = global.playerSprite.getPosition();
        pthread_mutex_unlock(&global.playerMutex);

        sf::Vector2f ghostPosition = global.ghostSprites[ghostNumber].getPosition();
        int deltaX = playerPosition.x - ghostPosition.x;
        int deltaY = playerPosition.y - ghostPosition.y;
        
        if (abs(deltaX) > abs(deltaY)) {
            global.ghostDirections[ghostNumber] = (deltaX > 0) ? 'r' : 'l';
            global.secondaryDirections[ghostNumber] = (deltaY < 0) ? 'u' : 'd';
        } else {
            global.ghostDirections[ghostNumber] = (deltaY < 0) ? 'u' : 'd';
            global.secondaryDirections[ghostNumber] = (deltaX > 0) ? 'r' : 'l';
        }

        sf::sleep(sf::milliseconds(20));
    }
    return NULL;
}

bool checkCollision(const sf::Sprite& sprite) {
    sf::FloatRect bounds = sprite.getGlobalBounds();
    for (int i = 0; i < global.NUM_BRICKS; i++) {
        if (bounds.intersects(global.wallBounds[i])) {
            return true;
        }
    }
    return false;
}

#endif // GHOST_H

