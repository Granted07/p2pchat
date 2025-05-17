#pragma once
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <string>

class UPnPForward {
public:
    UPnPForward();
    ~UPnPForward();

    bool add_port_mapping(int port);
    bool remove_port_mapping(int port);
    bool test_upnp_available() {
        UPNPDev* devlist = nullptr;
        try {
            unsigned char ttl = 2;
            devlist = upnpDiscover(2000, nullptr, nullptr, 0, 0, ttl, nullptr);
            if (!devlist) return false;

            struct UPNPUrls urls;
            struct IGDdatas data;
            char lanaddr[16] = { 0 };

            int result = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr));
            freeUPNPDevlist(devlist);
            FreeUPNPUrls(&urls);

            return result == 1;
        }
        catch (...) {
            if (devlist) freeUPNPDevlist(devlist);
            return false;
        }
    }

private:
    struct UPNPUrls urls;
    struct IGDdatas data;
    char lanaddr[16];
};