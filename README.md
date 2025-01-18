# Pac-Man Game with Multithreading

This project implements a classic Pac-Man game using multithreading techniques in C++. It follows a modular approach where different game components like the game engine, UI, and ghost controllers run in separate threads, ensuring smooth gameplay and better resource management.

## Project Overview

This project is divided into three phases:

### Dedicated Threads for Game Modules

- **Game Engine Thread**: 
  - Manages the overall game flow.
  - Coordinates player inputs, updates the game state, and handles rendering of game graphics.
  
- **User Interface Thread**: 
  - Manages all UI components (menus, HUD, scoreboards).
  - Handles input events such as keyboard and mouse inputs.
  
- **Ghost Controller Threads**: 
  - Each ghost has its own controller thread.
  - Responsible for managing the ghost's movements and AI behavior.
  - Initially, there are four ghosts, with the potential to spawn more as the game progresses.

### Basic Game Mechanics

- **Game Board Initialization**:
  - Designs the layout for the game, including walls, paths, and pellet positions.
  - Initializes starting positions for Pac-Man and ghosts.
  
- **Movement Mechanics**:
  - Implements logic for Pac-Man’s movement based on player inputs.
  - Implements basic AI behavior for ghosts.
  
- **Eating Mechanics**:
  - Pac-Man can eat pellets, and the game score is updated accordingly.
  - Implement the effects of Pac-Man eating a power pellet (e.g., ghosts turn blue for a limited time).
  
- **Lives System**:
  - Pac-Man starts with a predefined number of lives.
  - Decreases a life each time Pac-Man collides with a ghost.

### Synchronization

- **Ghost Movement Synchronization**:
  - Prevents read/write conflicts when updating the game board and deciding the ghosts’ next moves.
  
- **Eating Power Pellets**:
  - Ensures that Pac-Man can eat power pellets and make ghosts vulnerable while handling synchronization of multiple threads.
  
- **Ghost House Synchronization**:
  - Ensures only one ghost can exit the ghost house at a time while using resources like keys and exit permits.
  
- **Speed Boost Mechanism**:
  - Faster ghosts can receive a limited number of speed boosts to increase their movement speed. Each ghost must acquire a speed boost to proceed.

## Features

- **Multithreading**: Utilizes dedicated threads for game engine, UI, and ghost behavior to improve responsiveness and simulate a real-time game experience.
- **Basic AI for Ghosts**: Initially, ghosts follow predefined paths, but as the game progresses, they may calculate shortest paths to chase Pac-Man.
- **Player Movement**: Player controls Pac-Man’s movements and interacts with the game world, eating pellets and avoiding ghosts.
- **Game Mechanics**: The game includes classic Pac-Man mechanics such as lives, eating pellets, power-ups, and more.
- **Thread Synchronization**: Ensures smooth interaction between game modules through careful synchronization, such as when ghosts and Pac-Man interact with power pellets and exit the ghost house.

## Getting Started

### Prerequisites
- Ubuntu or a Unix-based system
- C++ compiler (g++ recommended)
- SFML graphics library installed for rendering.

### Installation Steps

1. **Clone the repository**:

   ```bash
   git clone https://github.com/shahmeerahmed486/Multithreaded-Pacman-Game.git
   cd pacman-game
    sudo apt-get install libsfml-dev
    g++ -std=c++11 -o pacman game.cpp -lsfml-graphics -lsfml-window -lsfml-system -pthread
    ./pacman
