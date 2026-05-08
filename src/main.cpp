#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include "../headers/tetris.h"
#include <cstdlib>
#include <ctime>

using namespace std;

int main() {
    //seed rand using current time so random pieces/colors change each run
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    //create & run game
    Tetris tetris;
    tetris.run();



    return 0;
}