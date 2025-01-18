#ifndef UI_H
#define UI_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>

#include "Shared.h"

void* uiManager(void* arg) {
    sf::RenderWindow* window = (sf::RenderWindow*)arg;
    while (global.gameRunning) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::KeyPressed) {
                pthread_mutex_lock(&global.playerDirectionMutex);
                if (event.key.code == sf::Keyboard::Up) {
                    global.playerDirection = 'u';

                    global.playerSprite.setTexture(global.pacmanTexture[0]);
                } else if (event.key.code == sf::Keyboard::Down) {
                    global.playerDirection = 'd';
                    global.playerSprite.setTexture(global.pacmanTexture[1]);
                } else if (event.key.code == sf::Keyboard::Left) {
                    global.playerDirection = 'l';
                    global.playerSprite.setTexture(global.pacmanTexture[2]);
                } else if (event.key.code == sf::Keyboard::Right) {
                    global.playerDirection = 'r';
                    global.playerSprite.setTexture(global.pacmanTexture[3]);
                }
                pthread_mutex_unlock(&global.playerDirectionMutex);
            } else if (event.type == sf::Event::Closed) {
                global.gameRunning = false;
            }
        }
    }
    return NULL;
}

#endif // UI_H

