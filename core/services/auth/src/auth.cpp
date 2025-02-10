#include "auth.h"
#include <iostream>

namespace nanoenv::auth {
bool Auth::authenticateUser(const std::string& username, const std::string& password) {
    std::cout << "Authenticating " << username << std::endl;
    return true;
}

bool Auth::authorizeUser(const std::string& username, const std::string& resource) {
    std::cout << "Authorizing " << username << " for " << resource << std::endl;
    return true;
}
}