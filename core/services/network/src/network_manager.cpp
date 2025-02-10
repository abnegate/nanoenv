#include "network_manager.h"
#include <iostream>

namespace nanoenv::network {
bool NetworkManager::configureNetwork(const std::string& containerName) {
    std::cout << "Configuring network for " << containerName << std::endl;
    return true;
}

bool NetworkManager::removeNetworkConfig(const std::string& containerName) {
    std::cout << "Removing network config for " << containerName << std::endl;
    return true;
}
}