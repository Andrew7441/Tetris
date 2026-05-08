#pragma once
#include<memory>
#include <SFML/Graphics.hpp>

//unsigned int from 0 - 4,294,967,295 or 2^32 - 1
static const std::uint32_t lines   { 24 };  //board height in tiles
static const std::uint32_t cols    { 15 }; // board width in tiles
static const std::uint32_t squares { 4 };
static const std::uint32_t shapes  { 7 };
static const std::uint32_t tileSize{ 40 };

struct Coords {
	int x{};
	int y{};
};

class Tetris {
	std::shared_ptr<sf::RenderWindow> window;
	//textures used by game
	sf::Texture tiles;
	sf::Texture bg;

	//sprites used for drawing
	std::shared_ptr<sf::Sprite> sprite;
	std::shared_ptr<sf::Sprite> background;

	std::vector<std::vector<std::uint32_t>> area;
	std::vector<std::vector<std::uint32_t>> forms;
	Coords z[squares];//current active piece coordinates
	Coords k[squares];//backup copy of z 

	int dirx;
	bool rotate, gameover;
	bool hasPiece{ false };
	sf::Clock clock;
	float timer{ 0.f }; // f means float literal, without it 0.3 is a double
	float delay{ 0.3f };
	int color{ 1 };
	int score;
	sf::Font font;
	sf::Text txtScore, txtGameOver;

protected:
	void events();
	void draw();
	void spawnPiece();
	void changePosition();
	void setRotate();
	void moveToDown();
	void resetValues();
	bool maxLimit();
	void setScore();

public:
	Tetris();
	~Tetris() = default;
	void run();
};