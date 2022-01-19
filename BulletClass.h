#pragma once

struct Bullet {
	double vel[2] = { 1.0, 0.0 }; 

	double pos[2] = { 0.0, 0.0 }; 

	sf::RectangleShape bulletShape;

	Bullet(double x, double y, double vx, double vy) {
		pos[0] = x; 
		pos[1] = y;

		vel[0] = vx; 
		vel[1] = vy; 

		bulletShape.setFillColor(sf::Color::Black); 
		bulletShape.setPosition((float)pos[0], (float)pos[1]); 
		bulletShape.setSize({ 5, 5 }); 
	}

	void move(double delta) {
		pos[0] += vel[0] * delta; 
		pos[1] += vel[1] * delta; 

		bulletShape.setPosition((float)pos[0], (float)pos[1]); 
	}
};


struct Wall {

	double pos[2] = { 0.0, 0.0 };
	int size[2] = { 0, 0 };

	int hp = 0;

	sf::RectangleShape wallShape; 

	Wall(double x, double y, int w, int h){
		pos[0] = x; 
		pos[1] = y; 

		size[0] = w; 
		size[1] = h; 

		hp = w * h * 0.1; 
		hp = 10 + hp * 0.9;

		hp = (1 - (1 / pow(2, (double)hp * 0.005))) * 200; 

		wallShape.setFillColor(sf::Color::Green); 
		wallShape.setOutlineThickness(2);
		wallShape.setOutlineColor(sf::Color::Black);

		wallShape.setSize({ (float)size[0], (float)size[1] });
		wallShape.setPosition({ (float)pos[0], (float)pos[1] }); 
	}
};


struct DrawableObject {
	int x, y;
	int w, h;
	
	int angle; 

	int outlineThickness = 2; 

	sf::Color color; 
	sf::Color outlineColor; 

	sf::RectangleShape objectShape; 

	DrawableObject(int x, int y, int w, int h, int angle) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->angle = angle;
	}
};