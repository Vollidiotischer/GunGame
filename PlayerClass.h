#pragma once

#define GRAVITY 0.3
#define JMP_CHARGES 3

struct Player {

	bool A_pressed = 0; 
	bool D_pressed = 0; 
	bool S_pressed = 0; 
	bool SPACE_pressed = 0; 
	bool M1_pressed = 0; 

	double pos[2] = { 10.f, 500.f };
	double vel[2] = { 0.f, 0.f };
	double moveVel[2] = { 0.f, 0.f };
	double acc[2] = { 0.f, 0.f };

	int mPosX = 0; 
	int mPosY = 0; 

	int size[2] = { 35, 55 };

	bool jumpingPressed = false;							// keeps track if the current space press has to be used for jumping or if the jump already occured 
	int jumpCharges = JMP_CHARGES;							// number of times the player can jump without touching the ground

	bool wallsliding = false;								// if the player is currently wall- (ceiling) sliding
	sf::Time wallSlidingTimer;								// keeps track of how long the player has been wallsliding 

	sf::RectangleShape *playerShape;						// The sfml shape of the player 

	double weaponPos[2] = { 10, 10 };						// position of the weapon
	int weaponRotation = 0;
	sf::RectangleShape *weapon;								// sfml weapon shape 

	int hp = 100; 

	Player(){												// constructor: sets size, colour, outline and position of the player & weapon
		playerShape = new sf::RectangleShape({ 35, 55 });
		playerShape->setFillColor(sf::Color::Magenta);
		playerShape->setOutlineThickness(2); 
		playerShape->setOutlineColor(sf::Color::Black);

		weapon = new sf::RectangleShape({ 20, 10 }); 
		weapon->setFillColor(sf::Color::Black); 

		weapon->setPosition({ 10, 10}); 
	}

};