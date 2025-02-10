#pragma once
#include <string>

namespace nanoenv::network {
class NetworkManager {
public:
    bool configureNetwork(const std::string& containerName);
    bool removeNetworkConfig(const std::string& containerName);
};
}
