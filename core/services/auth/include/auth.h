#pragma once
#include <string>

namespace nanoenv::auth {
class Auth {
public:
    bool authenticateUser(const std::string& username, const std::string& password);
    bool authorizeUser(const std::string& username, const std::string& resource);
};
}