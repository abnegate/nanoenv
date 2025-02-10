#pragma once
#include <string>

namespace nanoenv::storage {
class StorageManager {
public:
    bool mountWorkspace(const std::string& containerName, const std::string& path);
    bool unmountWorkspace(const std::string& containerName);
};
}