#pragma once
#include <winsock2.h>
#include <string>

class Socket {
public:
    Socket();
    ~Socket();
    
    bool bind(int port);
    bool connect(const std::string& ip, int port);
	bool set_non_blocking(bool non_blocking);
    int send(const std::string& message);
    std::string receive();
	bool is_connected() const { return sockfd != INVALID_SOCKET; }

private:
    SOCKET sockfd;
    bool connected = false;
	static bool winsock_initialized;
	static void initialise_winsock();
};