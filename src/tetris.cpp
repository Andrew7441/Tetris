#include "../headers/tetris.h"
#include <iostream>
#include <random>

Tetris::Tetris(): txtScore(font), txtGameOver(font) {
	//create game window
	window = std::make_shared<sf::RenderWindow>(
		sf::VideoMode({ cols * tileSize, lines * tileSize }),
		"Andrew's Tetris",
		sf::Style::Titlebar | sf::Style::Close
	);

	z->x = 0, z->y = 0;
	k->x = 0, k->y = 0;

	window->setPosition(sf::Vector2i(650, 0));

	//load tile texture
	if (!tiles.loadFromFile("./resources/img/squares.png")) {
		std::cerr << "Failed to load squares\n";
	}

	// Load background texture
	if (!bg.loadFromFile("./resources/img/background.png")) {
		std::cerr << "Failed to load background.png\n";
	}

	if (!font.openFromFile("resources/font.ttf")) {
		std::cerr << "Failed to load font.ttf\n";
	}

	score = { 0 };
	gameover = { false };

	// Configure score text
	txtScore.setPosition({ 100.f, 10.f });
	txtScore.setString("SCORE: " + std::to_string(score));
	txtScore.setStyle(sf::Text::Bold);
	txtScore.setCharacterSize(30);
	txtScore.setOutlineThickness(3.f);

	// Configure game-over text
	txtGameOver.setPosition({ 30.f, 330.f });
	txtGameOver.setString("GAME OVER");
	txtGameOver.setStyle(sf::Text::Bold);
	txtGameOver.setCharacterSize(50);
	txtGameOver.setOutlineThickness(3.f);
	background = std::make_shared<sf::Sprite>(bg);

	//scale to fit board size
	background->setScale({
		static_cast<float>(cols * tileSize) / bg.getSize().x,
		static_cast<float>(lines * tileSize) / bg.getSize().y,
	});

	//create sprite from texture
	sprite = std::make_shared<sf::Sprite>(tiles);

	//use only one 36 * 36 from the spritesheet instead of drawing whole image
	sprite->setTextureRect(sf::IntRect({ 0, 0 }, { static_cast<int>(tileSize),static_cast<int>(tileSize)}));

	//set up board size
	area.resize(lines);
	for (std::size_t i{}; i < area.size(); i++) {
		area[i].resize(cols);
	}

	forms = { // a compact way to describe each Tetris piece
	   {1,3,5,7}, // I
	   {2,4,5,7}, // Z
	   {3,5,4,6}, // S
	   {3,5,4,7}, // T
	   {2,3,5,7}, // L
	   {3,5,7,6}, // J
	   {2,3,4,5}, // O
	};

	spawnPiece(); // create new piece randomly

	//initialize movement state
	dirx = { 0 };
	rotate = { false };
}

void Tetris::events() {
	delay = 0.3f;

	while (const std::optional event = window->pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			window->close();
		}
		
		if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
			switch (key->code) {
				case sf::Keyboard::Key::Up :
					rotate = true;
					break;

				case sf::Keyboard::Key::Left :
					--dirx;
					break;
				
				case sf::Keyboard::Key::Right :
					++dirx;
					break;

				case sf::Keyboard::Key::Down :
					delay = 0.05f;
					break;

				default:
					break;
			}
		}
	}
}

void Tetris::draw() {
	window->clear(sf::Color::Black);

	window->draw(*background);

	txtScore.setString("SCORE: " + std::to_string(score));
	window->draw(txtScore);

	//draw all landed blocks stored in the board grid
	for (std::size_t i{}; i < lines; i++) {
		for (std::size_t j{}; j < cols; j++) {

			//0 means empty cell, non zero means a block exists here
			if (area[i][j] != 0) {

				// Pick the correct tile color from the spritesheet
				sprite->setTextureRect(
					sf::IntRect(
						{ static_cast<int>(area[i][j] * tileSize), 0 },
						{ static_cast<int>(tileSize), static_cast<int>(tileSize) }
					)
				);

				// Position the landed block at column j, row i
				sprite->setPosition({
					static_cast<float>(j * tileSize), //SFML positions are floating-point coordinates
					static_cast<float>(i * tileSize)
				});

				//draw landed block
				window->draw(*sprite);
			}
		}
	}

	//draw the currently falling piece
	for (std::size_t i{}; i < squares; i++) {

		//pick the current falling piece color
		sprite->setTextureRect(
			sf::IntRect(
				{ static_cast<int>(color * tileSize), 0},
				{ static_cast<int>(tileSize), static_cast<int>(tileSize) }
			)
		);

		//position each block of the falling piece
		sprite->setPosition({
				static_cast<float>(z[i].x * tileSize),
				static_cast<float>(z[i].y * tileSize)
		});

		window->draw(*sprite);
	}

	// Draw game-over text only if game is over
	if (gameover) {
		window->draw(txtGameOver);
	}

	window->display();
}

void Tetris::spawnPiece() {
	color = 1 + std::rand() % shapes;

	std::uint32_t number = std::rand() % shapes;

	for (size_t i{}; i < squares; i++) {
		z[i].x = forms[number][i] % 2; // row
		z[i].y = forms[number][i] / 2; // column
	}

	hasPiece = true;
}

void Tetris::setScore() {
	// Start writing rows from the bottom of the board
	int writeRow = static_cast<int>(lines) - 1;

	// Scan rows from bottom to top
	for (int readRow = static_cast<int>(lines) - 1; readRow >= 0; --readRow) {
		std::uint32_t filledCells{ 0 };

		// Count filled cells and copy row downward
		for (std::size_t col{}; col < cols; ++col) {
			if (area[readRow][col] != 0) {
				// If blocks reach near the top, game is over
				if (readRow == 1) {
					gameover = true;
				}

				++filledCells;
			}

			// Copy current row to the current write row
			area[writeRow][col] = area[readRow][col];
		}

		// Keep non-full rows
		if (filledCells < cols) {
			--writeRow;
		}
		else {
			// Clear full row by not moving writeRow upward
			++score;
			txtScore.setString("SCORE: " + std::to_string(score));
		}
	}
}

void Tetris::moveToDown() {
	//only move down when enough time has passed
	if (timer <= delay) return;

	//save current valid position, then move down
	for (std::size_t i{}; i < squares; i++) {
		k[i] = z[i];
		z[i].y++;
	}

	//if new position is invalid, lock old position into board
	if (maxLimit()) {
		for (std::size_t i{}; i < squares; i++) {
			area[k[i].y][k[i].x] = color; // saave landed block color
		}

		setScore();
		//create new piece after current one lands
		spawnPiece();
			
		if (maxLimit()) gameover = true;
	}
		
	//reset fall timer
	timer = 0.f;
}

void Tetris::changePosition() {
	//move block left/right
	for (std::size_t i{}; i < squares; i++) {
		k[i] = z[i];	// save old position before changing it
		z[i].x += dirx; // move horizontally
	}

	//if new position is invalid, undo the move
	if (maxLimit()) {
		for (std::size_t i{}; i < squares; i++) {
			z[i] = k[i];
		}
	}
}

void Tetris::setRotate() {

	if (rotate) {
		Coords Pivot = z[1]; // Pivot point - the block the piece rotates around

		//save old pisition before attempting rotation
		for (std::size_t i{}; i < squares; i++) {
			k[i] = z[i];
		}

		for (std::size_t i{}; i < squares; i++) { // rotate all 4 blocks around pivot
			int x = z[i].y - Pivot.y;
			int y = z[i].x - Pivot.x;

			z[i].x = Pivot.x - x;	 
			z[i].y = Pivot.y + y;
		}

		if (maxLimit()) { // check whetehr the final rotated piece is invalid, undo rotation
			for (std::size_t i{}; i < squares; i++) { 
				z[i] = k[i];
			}
		}
	}
}

void Tetris::resetValues() {
	dirx = 0;
	rotate = false;
}

bool Tetris::maxLimit() {
	for (std::size_t i{}; i < squares; ++i) {

		if (z[i].x < 0 ||			//too far left
			z[i].x >= cols ||		//too far right
			z[i].y >= lines ||		//too far down
			area[z[i].y][z[i].x])	//already occupied
		{
			return true;
		}
	}
	return false;
}

void Tetris::run() {
	while (window->isOpen()) {
		float time = clock.restart().asSeconds(); // Get seconds since last loop/frame, then reset clock
		timer += time; // add elapsed frame time to the falling timer

		events();

		if (!gameover) {
			changePosition();
			setRotate();

			if (timer > delay) { // fall every delay seconds 0.3 instead of falling every frame
				moveToDown(); // move down only after delay seconds
				timer = 0.f;  // restart fall timer
			}

			resetValues();
		}

		draw();
	}
}
