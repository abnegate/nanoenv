#include "storage_manager.h"
#include <iostream>

namespace nanoenv::storage {
bool StorageManager::mountWorkspace(const std::string& containerName, const std::string& path) {
    std::cout << "Mounting " << path << " to " << containerName << std::endl;
    return true;
}

bool StorageManager::unmountWorkspace(const std::string& containerName) {
    std::cout << "Unmounting workspace for " << containerName << std::endl;
    return true;
}
}