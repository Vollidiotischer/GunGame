#include <iostream>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Client.h"
#include "PlayerClass.h"
#include "BulletClass.h"


void draw(); 
void init(); 
void events(); 
void update(); 
void placeWalls(); 
int getDeltaTime();
void interpreteClientEvents();
std::string createClientMessage(); 

void fireBullet(Player&); 
void handleWallsliding(Player*);
bool isCollidingWB(Wall&, Bullet&);
bool isCollidingPB(Player&, Bullet&);
void processPlayerInput(Player*); 
void updateWeaponPosition(Player*);
void handleWallCollisions(Player*);
void handleBulletCollisions(Player*); 
void updatePlayerMovementInfo(Player*, double); 


const int WIDTH = 800;
const int HEIGHT = 1000;

sf::RenderWindow *window; 
Player *localPlayer; 
Player *remotePlayer; 

extern std::string globalMessage; // for recieving client info
extern std::mutex mtx;
extern std::mutex mtx2;

std::string sendingMessage; 

namespace timers {
	sf::Clock clock1;
	sf::Clock clock2;
}

namespace ob {
	sf::Color bgCol;

	std::vector<Wall> walls;

	std::vector<Bullet> bullets; 
}


void startGame() {
	
	init(); 

	placeWalls(); 

	timers::clock2.restart(); // reset clock for correct delta time 

	while (window->isOpen()) {

		interpreteClientEvents(); 

		events(); 

		update(); 

		draw(); 

	}
	

}

void events() {


	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
		localPlayer->A_pressed = true;

	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
		localPlayer->D_pressed = true;

	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){ // player is released from wallsliding with a boost downwards 
		localPlayer->S_pressed = true;

	}


	sf::Event event; 
	
	while (window->pollEvent(event)) {
		if (event.type == sf::Event::Closed) {
			window->close();
		}

		if (event.type == sf::Event::MouseButtonPressed) {
			if (event.key.code == sf::Mouse::Left) {
				localPlayer->M1_pressed = true; 
			}
		}

		if (event.type == sf::Event::MouseButtonReleased) {
			if (event.key.code == sf::Mouse::Left) {
				localPlayer->M1_pressed = false; 
			}
		}

		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Space) { // Jump
				localPlayer->SPACE_pressed = true;


			}
		}

		if (event.type == sf::Event::KeyReleased) {
			if (event.key.code == sf::Keyboard::A) {
				localPlayer->A_pressed = false;
			}

			if (event.key.code == sf::Keyboard::D) {
				localPlayer->D_pressed = false; 
			}

			if (event.key.code == sf::Keyboard::S) {
				localPlayer->S_pressed = false; 
			}

			if (event.key.code == sf::Keyboard::Space) {
				localPlayer->SPACE_pressed = false; 
			}

		}
	}

	// Update mouse position of local player
	auto mousePos = sf::Mouse::getPosition(*window);
	localPlayer->mPosX = mousePos.x; 
	localPlayer->mPosY = mousePos.y; 


}

void update() {

	double delta = getDeltaTime() / 10;

	delta = delta > 3 ? 3 : delta; // limit delta time (so that is doesn#t get humongous when resizing window) 

	// Update Bullet position
	for (Bullet& b : ob::bullets) {
		b.move(delta);
	}

	processPlayerInput(localPlayer); 
	processPlayerInput(remotePlayer); 

	// MANAGE WALLSLIDING (check if Wallsliding has to be stopped) 
	handleWallsliding(localPlayer); 
	handleWallsliding(remotePlayer); 


	// Player Physic update
	updatePlayerMovementInfo(localPlayer, delta);
	updatePlayerMovementInfo(remotePlayer, delta);


	//Collisions 
	handleWallCollisions(localPlayer);
	handleWallCollisions(remotePlayer);

	handleBulletCollisions(remotePlayer);
	handleBulletCollisions(localPlayer);

	// weapon movement / rotation
	updateWeaponPosition(localPlayer); 
	updateWeaponPosition(remotePlayer); 

	// bullet cleanup
	for (int i = 0; i < ob::bullets.size(); i++) {
		if (ob::bullets[i].pos[0] < 0 || ob::bullets[i].pos[0] > WIDTH || ob::bullets[i].pos[1] < 0 || ob::bullets[i].pos[1] > HEIGHT) { // if Bullet goes out of bounds 
			ob::bullets.erase(ob::bullets.begin() + i);
		}

	}

	// Bullet collisions & Wall destroying
	for (int i = 0; i < ob::walls.size(); i++) {
		for (int i2 = 0; i2 < ob::bullets.size(); i2++) {
			if (isCollidingWB(ob::walls[i], ob::bullets[i2])) {
				ob::bullets.erase(ob::bullets.begin() + i2); // erase Bullet
				ob::walls[i].hp -= 10; // decrease wall hp
			}
		}
	}

	// remove destroyed walls
	for (int i = 0; i < ob::walls.size(); i++) {
		if (ob::walls[i].hp < 1) {
			ob::walls.erase(ob::walls.begin() + i); 
		}
	}

	if (mtx2.try_lock()) {
		sendingMessage = createClientMessage();
		mtx2.unlock();
	}
}

void draw() {

	window->clear(ob::bgCol); 

	// Draw Bullets
	for (Bullet& b : ob::bullets) {
		window->draw(b.bulletShape);
	}

	// Draw local Player & Weapon
	window->draw(*localPlayer->playerShape);
	window->draw(*localPlayer->weapon);

	// Draw remote Player & Weapon
	window->draw(*remotePlayer->playerShape);
	window->draw(*remotePlayer->weapon);

	// Draw obstacles
	for (int i = 0; i < ob::walls.size(); i++) {
		window->draw(ob::walls[i].wallShape); 
	}

	window->display(); 
}

void updateWeaponPosition(Player* p) {

	double dx = p->mPosX - (p->pos[0] + p->size[0] / 2.0);
	double dy = p->mPosY - (p->pos[1] + p->size[1] / 3.0);

	double nevl = 1 / sqrt(dx * dx + dy * dy);

	double angle = -acos(dx/sqrt(dx*dx+dy*dy)) * 180.0 / 3.1415;


	//std::cout << "dx: " << dx << "  dy: " << dy << "  angle: " << angle << std::endl; 

	if (dy > 0) {
		angle = 360 - angle; 
	}

	dx *= nevl * 30; // normaleneinheitsvektor * 30 X
	dy *= nevl * 30; // normaleneinheitsvektor * 30 Y

	p->weaponPos[0] = (p->pos[0] + p->size[0] / 2.0) + dx;
	p->weaponPos[1] = (p->pos[1] + p->size[1] / 3.0) - p->weapon->getSize().y + dy;
	p->weaponRotation = angle; 

	p->weapon->setRotation(angle);
	p->weapon->setPosition(p->weaponPos[0], p->weaponPos[1]);

	/*
	For old way put: 
	angle = atan(dy / dx) * 180.0 / 3.1415;
	and weapon Size = { (float)(20 * (dx / abs(dx))), 10 }
	*/
	
}

void updatePlayerMovementInfo(Player* p, double delta) { // Only called in Update to update the player stats
	p->vel[1] += GRAVITY * delta; // gravity

	p->vel[0] += p->acc[0] * delta; // basically useless
	p->vel[1] += p->acc[1] * delta;

	p->pos[0] += p->vel[0] * delta; 
	p->pos[1] += p->vel[1] * delta;
	
	p->pos[0] += p->moveVel[0] * delta;
	p->pos[1] += p->moveVel[1] * delta;

	// Set player position
	p->playerShape->setPosition(p->pos[0], p->pos[1]);
}

void fireBullet(Player& p) {

	if (ob::bullets.size() < 10) {
		double dx = p.mPosX - (p.pos[0] + p.size[0] / 2.0);
		double dy = p.mPosY - (p.pos[1] + p.size[1] / 3.0);

		double nevl = 1 / sqrt(dx * dx + dy * dy);

		dx *= nevl * 30;
		dy *= nevl * 30;

		ob::bullets.push_back(Bullet(p.weaponPos[0] + dx, p.weaponPos[1] + dy, dx, dy));
	}

}

void handleWallCollisions(Player *p) {
	double playerXcenter = p->pos[0] + p->size[0] / 2;
	double playerYcenter = p->pos[1] + p->size[1] / 2;

	for (int i = 0; i < ob::walls.size(); i++) {
		auto wallPos = ob::walls[i].pos; ;
		auto wallSize = ob::walls[i].size;

		//double rectXcenter = (double)wallPos[0] + (double)wallSize[0] / 2.0;
		//double rectYcenter = (double)wallPos[1] + (double)wallSize[1] / 2.0;

		double dx = playerXcenter - ((double)wallPos[0] + (double)wallSize[0] / 2.0); // distance between player center and wall center X
		double dy = playerYcenter - ((double)wallPos[1] + (double)wallSize[1] / 2.0); // distance between player center and wall center Y

		double aw = ((double)wallSize[0] + p->size[0]) * 0.5; // average width of player and wall
		double ah = ((double)wallSize[1] + p->size[1]) * 0.5; // average height of player and wall

		if (!(abs(dx) > aw || abs(dy) > ah)) { // player is colliding 

			if (abs(dx / wallSize[0]) > abs(dy / wallSize[1])) {

				if (dx < 0) { // player is on the LEFT
					p->pos[0] = (double)wallPos[0] - p->size[0]; // fix player position 
					p->vel[0] = 0;// stop player from going in the block

				}
				else { // player is on the RIGHT
					p->pos[0] = (double)wallPos[0] + (double)wallSize[0];
					p->vel[0] = 0;

				}

			}
			else {

				if (dy < 0) { // player is on TOP
					p->pos[1] = (double)wallPos[1] - p->size[1];
					p->vel[1] = 0;
					p->jumpCharges = JMP_CHARGES;
				}
				else { // player is at the BOTTOM
					p->pos[1] = (double)wallPos[1] + (double)wallSize[1];
					if (!p->wallsliding) {
						p->wallSlidingTimer = timers::clock1.getElapsedTime();
						p->wallsliding = true;
					}

					p->vel[1] = 0;

				}
			}

		}


	}

}

void handleBulletCollisions(Player *p) {

	for (int i = 0; i < ob::bullets.size(); i++) {
		if (isCollidingPB(*p, ob::bullets[i])) {
			ob::bullets.erase(ob::bullets.begin() + i); 
			
			p->hp -= 20; 

			if (p->hp < 1) {
				std::cout << "player died" << std::endl; 
			}
		}
	}

}

void processPlayerInput(Player *p) {
	if (p->A_pressed) {
		p->moveVel[0] = -5; 
	}
	else {
		if (p->D_pressed) {
			p->moveVel[0] = 5; 
		}
		else {
			p->moveVel[0] = 0; 
		}
	}

	if (p->SPACE_pressed) {
		if (p->jumpCharges > 0 && !p->jumpingPressed) {
			p->jumpingPressed = true;
			p->vel[1] -= 10; 
			p->jumpCharges -= 1; 
		}
	}
	else {
		p->jumpingPressed = false; 
	}

	if (p->S_pressed) {
		if (p->wallsliding) {
			p->wallsliding = false;
			p->vel[1] += 1;
		}
	}

	if (p->M1_pressed) {
		fireBullet(*p); 
	}
}

void handleWallsliding(Player *p) {
	if (p->wallsliding) {
		if (timers::clock1.getElapsedTime().asMilliseconds() - p->wallSlidingTimer.asMilliseconds() > 1000) {
			p->wallsliding = false;
			p->pos[1]++; // push player away from the ceiling 
		}
	}
}

bool isCollidingWB(Wall& w, Bullet& b) {

	return (b.pos[0] > w.pos[0] && b.pos[0] < w.pos[0] + w.size[0] && b.pos[1] > w.pos[1] && b.pos[1] < w.pos[1] + w.size[1]);

}

bool isCollidingPB(Player& p, Bullet& b) {

	return (b.pos[0] > p.pos[0] && b.pos[0] < p.pos[0] + p.size[0] && b.pos[1] > p.pos[1] && b.pos[1] < p.pos[1] + p.size[1]);

}

void interpreteClientEvents() {
	
	if (mtx.try_lock()) {
		std::string s = globalMessage;
		mtx.unlock();

		if (s.size() > 0) {

			int firstSemicolon = s.find_first_of(';');

			std::string mouseCords = s.substr(firstSemicolon + 1);

			short midSemicolonPos = mouseCords.find_first_of(';');

			remotePlayer->A_pressed = 	  (int)s[0] - (int)'0'; 
			remotePlayer->D_pressed =	  (int)s[1] - (int)'0';
			remotePlayer->S_pressed =	  (int)s[2] - (int)'0';
			remotePlayer->SPACE_pressed = (int)s[3] - (int)'0';
			remotePlayer->M1_pressed =	  (int)s[4] - (int)'0';

			remotePlayer->mPosX = std::stoi(mouseCords.substr(0, midSemicolonPos));
			remotePlayer->mPosY = std::stoi(mouseCords.substr(midSemicolonPos + 1));
		}
	}
	
}

std::string createClientMessage() {
	using namespace std; 

	std::string message = ";";

	message +=
		// Player Data (1 & 2)
		to_string((int)localPlayer->pos[0]) + ';' + to_string((int)localPlayer->pos[1]) + ';' +
		to_string(localPlayer->mPosX) + ';' + to_string(localPlayer->mPosY) + ';' + 
		to_string((int)remotePlayer->pos[0]) + ';' + to_string((int)remotePlayer->pos[1]) + ';' +
		
		// Bullet Data
		to_string(ob::bullets.size()) + ';'; 

	for (int i = 0; i < ob::bullets.size(); i++) {
		message += to_string((int)ob::bullets[i].pos[0]) + ';' + to_string((int)ob::bullets[i].pos[1]) + ';'; 
	}

	message += to_string(ob::walls.size()) + ';'; 

	for (int i = 0; i < ob::walls.size(); i++) {
		message +=
			to_string((int)ob::walls[i].pos[0]) + ';' + to_string((int)ob::walls[i].pos[1]) + ';' +
			to_string(ob::walls[i].size[0]) + ';' + to_string(ob::walls[i].size[1]) + ';' +
			to_string(ob::walls[i].hp) + ';'; 
	}

	message += '!'; 

	return message; 
}

void placeWalls() {
	ob::walls.push_back(Wall(0, HEIGHT - 50, WIDTH, 30));

	ob::walls.push_back(Wall(300, 475, 500, 300));

	ob::walls.push_back(Wall(100, 100, 200, 40)); 

} 

void init() {
	// Initialize & create window
	window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Gun Game"); 
	window->setFramerateLimit(60); 
	window->setKeyRepeatEnabled(false); 

	// Initialize background colour 
	ob::bgCol.r = 130;
	ob::bgCol.g = 113;
	ob::bgCol.b = 96;

	// create local player
	localPlayer = new Player();
	remotePlayer = new Player();

}

int getDeltaTime() {
	return timers::clock2.restart().asMilliseconds();
}


/*

TODO: 
	- Display studd on Clientside
	- schuss cooldown
	- collisions between players and bullets
	- Wallsliding (on walls)

game Loop: 
	- get user input
	- update own game variables
	- draw

async: 
	- recieve other players data
		-> Update game Variables (mutex) 
	- Send own data
		-> Reguarly (mutex) 

*/