#include "upnp.h"
#include <iostream>

UPnPForward::UPnPForward() {
	unsigned char ttl = 2;
	UPNPDev* devlist = upnpDiscover(2000, nullptr, nullptr, 0, 0, ttl, nullptr);
	if (!devlist) {
		throw std::runtime_error("UPnP discovery failed");
	}

	int error = 0;
	if (!UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))) {
		freeUPNPDevlist(devlist);
		throw std::runtime_error("No valid IGD found");
	}
	freeUPNPDevlist(devlist);
}

UPnPForward::~UPnPForward() {
	FreeUPNPUrls(&urls);
}

bool UPnPForward::add_port_mapping(int port) {
	return UPNP_AddPortMapping(
		urls.controlURL,
		data.first.servicetype,
		std::to_string(port).c_str(),
		std::to_string(port).c_str(),
		lanaddr,
		"P2P Chat",
		"TCP",
		nullptr,
		"0"
	) == UPNPCOMMAND_SUCCESS;
}

bool UPnPForward::remove_port_mapping(int port) {
	return UPNP_DeletePortMapping(
		urls.controlURL,
		data.first.servicetype,
		std::to_string(port).c_str(),
		"TCP",
		nullptr
	) == UPNPCOMMAND_SUCCESS;
}