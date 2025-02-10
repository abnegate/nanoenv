#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace nanoenv::database {
    class Database {
    public:
        explicit Database(const std::string &connectionInfo);

        drogon::Task<> saveApiKey(std::string apiKey) const;

        drogon::Task<> saveImageMetadata(
            std::string fileName,
            std::string filePath,
            uint64_t fileSize,
            std::string dimensions,
            std::string format,
            std::optional<std::string> description,
            std::optional<double> gpsLatitude,
            std::optional<double> gpsLongitude,
            std::string userId,
            std::optional<std::string> originalImageId,
            std::string mlModelUsed,
            double mlInferenceTime,
            Json::Value mlMetadata
        ) const;

    private:
        std::shared_ptr<drogon::orm::DbClient> dbClient;
    };
} // namespace nanoenv::database
