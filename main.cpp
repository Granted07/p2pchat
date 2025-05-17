#include "socket.h"
#include "upnp.h"
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<bool> running(true);

void chatLoop(Socket& inbound, Socket& outbound) {
	std::thread receiver([&] {
		while (running) {
			try
			{
				std::string message = inbound.receive();
				if (!message.empty()) {
					std::cout << "\nPeer: " << message << "\nYou: " << std::flush;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			catch (const std::exception& e)
			{
				std::cerr << "\nReceiving error: " << e.what() << "\n";
				running = false;
			}
		}
		});

	std::string message;
	std::cout << "You: ";
	while (running && std::getline(std::cin, message)) {
		if (message == "exit") break;
		try
		{
			if (!outbound.send(message)) {
				std::cerr << "\nSend failed (connection lost)\n";
				break;
			}
			std::cout << "You: ";
		}
		catch (const std::exception& e)
		{
			std::cerr << "\nSend error: " << e.what() << "\n";
			break;
		}
	}
	running = false;
	receiver.join();
}

static bool check_upnp_support() {
	try
	{
		UPnPForward test;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool establish_connection(Socket& outbound, const std::string& ip, int port) {
	const int max_attempts = 30;
	const int wait_time = 2000; 

	std::cout << "Attempting to connect to " << ip << ":" << port << "\n";

	for (int attempt = 1; attempt <= max_attempts; ++attempt) {
		if (outbound.connect(ip, port)) {
			std::cout << "Connected to " << ip << ":" << port << "\n";
			return true;
		}

		std::cout << "Attempt " << attempt << " failed. Retrying in " << wait_time / 1000 << " seconds...\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
	}
}

int main() {
	try
	{
		std::cout << "Checking UPnP support...\n";
		if (check_upnp_support()) {
			std::cout << "Available\n";
		}
		else {
			std::cout << "Not available\n";
			std::cout << "Note: You may need to manually forward ports on your router\n";
			return 1;
		}

		// get connection details
		int my_port, peer_port;
		std::string peer_ip;
		
		std::cout << "Enter your port: ";
		std::cin >> my_port;
		std::cout << "Enter peer's IP: ";
		std::cin >> peer_ip;
		std::cout << "Enter peer's port: ";
		std::cin >> peer_port;
		std::cin.ignore();

		// Set up UPnP
		std::unique_ptr<UPnPForward> upnp;
		try
		{
			upnp = std::make_unique<UPnPForward>();
			if (upnp->add_port_mapping(my_port)) {
				std::cout << "Port " << my_port << " forwarded successfully\n";
			}
			else {
				std::cerr << "Failed to forward port " << my_port << "\n";
			}
		}
		catch (const std::exception&) {}

		//Initialise sockets
		Socket inbound, outbound;

		if (!inbound.bind(my_port)) {
			std::cerr << "Failed to bind port" << my_port << "\n";
			return 1;
		}
		inbound.set_non_blocking(true);

		if (establish_connection(outbound, peer_ip, peer_port)) {
			std::cout << "Connection established\n";
			std::cout << "Type 'exit' to quit\n";
			chatLoop(inbound, outbound);
		}
		else {
			std::cerr << "Failed to establish connection\n";
			return 1;
		}

	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}