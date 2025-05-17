#include "socket.h"
#include <stdexcept>
#include <ws2tcpip.h>
#include <system_error>

#pragma comment(lib, "ws2_32.lib")

bool Socket::winsock_initialized = false;

void Socket::initialise_winsock() {
	if (!winsock_initialized) {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			throw std::system_error(WSAGetLastError(), std::system_category(), "WSAStartup failed");
		}
		winsock_initialized = true;

		// Optional: Verify WinSock version
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
			WSACleanup();
			throw std::runtime_error("Could not find a usable Winsock DLL");
		}
	}
}

Socket::Socket() {
	initialise_winsock();  // Ensures Winsock is ready
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == INVALID_SOCKET) {
		throw std::runtime_error("Socket creation failed: " + std::to_string(WSAGetLastError()));
	}
}

Socket::~Socket() {
	if (sockfd != INVALID_SOCKET) {
		closesocket(sockfd);
	}
}

bool Socket::bind(int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
	if (::bind(sockfd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		return false;
	}
	return true;
}

int Socket::send(const std::string& message) {
    return ::send(sockfd, message.c_str(), static_cast<int>(message.size()), 0);
}

std::string Socket::receive() {
	char buffer[1024];
	int bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
	if (bytesReceived > 0) {
		buffer[bytesReceived] = '\0';
		return std::string(buffer);
	}
	return "";
}

bool Socket::set_non_blocking(bool non_blocking) {
	u_long mode = non_blocking ? 1 : 0;
	return ioctlsocket(sockfd, FIONBIO, &mode) == 0;
}

bool Socket::connect(const std::string& ip, int port) {
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	// Convert IP string to binary form
	if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
		return false;
	}

	// Attempt connection
	if (::connect(sockfd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		// Check if connection is in progress (non-blocking mode)
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			fd_set writefds;
			FD_ZERO(&writefds);
			FD_SET(sockfd, &writefds);

			timeval timeout{ 5, 0 };
			if (select(0, nullptr, &writefds, nullptr, &timeout) > 0) {
				int error = 0;
				socklen_t len = sizeof(error);
				getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
				if (error == 0) {
					connected = true;
					return connected;
				}
			}
		}
		return false;
	}

	connected = true;
	return true;
}

