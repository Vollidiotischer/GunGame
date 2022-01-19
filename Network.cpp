#include <iostream>
#include "ServerDoc.h"

extern std::string globalMessage; 
extern std::mutex mtx; 
extern std::mutex mtx2; 

extern std::string sendingMessage; 

void recievingLoop(Server *server) {

	while (true) {
		Sleep(20); 
		mtx.lock(); 
		globalMessage = server->recieve(); 
		mtx.unlock(); 
		if (globalMessage == "SOCKET_CLOSED") {
			std::cout << globalMessage << std::endl;
			break; 
		}
	}
}

void sendingLoop(Server *server) {
	while (true) {
		Sleep(16); 

		if (sendingMessage != "") {
			mtx2.lock();
			server->send(sendingMessage);
			mtx2.unlock();

			if (!(server->recieve() == "rec!")) {
				std::cout << "false client return" << std::endl;
				break;
			}
		}

		if (!server->socket->is_open()) {
			std::cout << "Data socket closed" << std::endl; 
			break; 
		}
	}
}


/*

;212;594;196;620;181;645;165;671;281;812;150;697;253;822;135;723;225;832;196;842;3;0;950;800;30;199;300;475;500;300;20;100;100;200;40;184;!
;258;517;242;543;227;568;212;594;366;782;196;620;338;792;181;646;309;802;281;812;3;0;950;800;30;199;300;475;500;300;20;100;100;200;40;184;!

*/