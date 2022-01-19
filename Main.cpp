#include <thread>
#include <iostream>
#include "ServerDoc.h"


void cleanup();							// delete created objects
void startGame();						// start the main Game
void recievingLoop(Server*);
void sendingLoop(Server*); 
void startRoutine();					// Function to complete the start-game-Routine

void startScreen(int&);					// Function to retrieve Server IP and Port and display a screen for that


std::string globalMessage; 
std::mutex mtx; 
std::mutex mtx2; 

Server *movementServer;
Server *dataServer;


int main(){
    
	startRoutine(); 

    return 0;
}

void startRoutine() {
	int movementServerPort = 8080; 
	int dataServerPort = 8081; 
	
	//startScreen(port);								// Port als Referenz übergeben 

	movementServer = new Server(movementServerPort);	// Create new Server
	movementServer->startServer();						// start Server 

	dataServer = new Server(dataServerPort); 
	dataServer->startServer(); 

	std::thread t1(startGame);							// In nem neuen Thread? 

	std::thread t2(recievingLoop, movementServer);
	//Client sending / Recieving loop (neuer Thread?) 

	std::thread t3(sendingLoop, dataServer); 

	t1.join(); 
	t2.join();
	t3.join(); 


	cleanup(); 
}

void startScreen(int& port) {
	std::string temp;

	std::cout << "Please input port below." << std::endl; 

	std::cin >> temp; 
	port = std::stoi(temp); 
}

void cleanup() {
	movementServer->cleanup();
	delete movementServer;
}


/*

HOW TO START PROGRAM: 
	- start java server
	- start c++ main client
	- start c++ secondary client




startup:
	- IP Eingabe
	- Port Eingabe
	- Connect Button
	-> Folgt in einem Loading / Connecting screen

-> Ready Screen (Wenn beide bereit sind, dann geht es los)

-> Player 1 Zeichnet
-> player 2 Zeichnet

-> Game

- Drawing Doc
- Network Doc
- Calculation Doc

*/