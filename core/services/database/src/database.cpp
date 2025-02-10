#include "database.h"
#include "system.h"
#include <drogon/drogon.h>

namespace nanoenv::database {
    Database::Database(
        const std::string &connectionInfo
    ) {
        dbClient = drogon::orm::DbClient::newPgClient(connectionInfo, 1);
    }

    auto Database::saveApiKey(
        std::string apiKey
    ) const -> drogon::Task<> {
        co_await dbClient->execSqlCoro(
            "INSERT INTO api_keys (created_at, hashed_key, is_active) VALUES ($1, $2, TRUE)",
            platform::System::getCurrentTime(),
            std::move(apiKey)
        );

        co_return;
    }

    auto Database::saveImageMetadata(
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
    ) const -> drogon::Task<> {
        co_await dbClient->execSqlCoro(
            R"(
            INSERT INTO image_metadata (
                file_name,
                file_path,
                file_size,
                dimensions,
                format,
                tags,
                description,
                gps_latitude,
                gps_longitude,
                processing_steps,
                user_id,
                original_image_id,
                ml_model_used,
                ml_inference_time,
                ml_metadata
            ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15)
        )",
            std::move(fileName),
            std::move(filePath),
            fileSize,
            std::move(dimensions),
            std::move(format),
            std::move(description.value_or("")),
            gpsLatitude.value_or(0.0),
            gpsLongitude.value_or(0.0),
            std::move(userId),
            std::move(originalImageId.value_or("")),
            std::move(mlModelUsed),
            mlInferenceTime,
            std::move(mlMetadata.toStyledString())
        );

        co_return;
    }
} // namespace nanoenv::database
