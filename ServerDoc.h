#pragma once

#define LOG(x) std::cout << x << std::endl; 

#include <iostream>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp> 

struct Server {
	
	int port = 8080; 
	
	asio::io_service io_service; 
	asio::ip::tcp::acceptor *acceptor; 
	asio::ip::tcp::socket *socket;

	Server(int port){
		this->port = port; 
	}

	void startServer() {
		
		// listening for new connection
		acceptor = new asio::ip::tcp::acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)); 
		
		LOG("Listening for Client"); 
		
		// create socket 
		socket = new asio::ip::tcp::socket(io_service); 

		// waiting for new connection
		acceptor->accept(*socket); 
		
		LOG("Accepting Client"); 
	}

	void send(std::string message) {
		asio::write(*socket, asio::buffer(message)); // vielleicht muss zu 'message' noch ein '\n' hinzugefügt werden 
	}

	std::string recieve() {
		asio::streambuf buf;
		asio::read_until(*socket, buf, '!'); 

		std::string data = asio::buffer_cast<const char*>(buf.data()); 
		return data; 
	}

	void cleanup() {
		socket->close(); 
		acceptor->close();
		
		delete acceptor; 
		delete socket; 
	}

};