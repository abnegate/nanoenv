#include "system.h"
#include <chrono>
#include <string>

namespace nanoenv::platform {
    std::string System::getCurrentTime() {
        using namespace std::chrono;

        const auto now = system_clock::now();
        const auto nowTimeT = system_clock::to_time_t(now);
        const auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000;

        std::tm tm{};
        gmtime_r(&nowTimeT, &tm);

        constexpr char digits[] = "0123456789";

        char buffer[25] = {
            digits[(tm.tm_year + 1900) / 1000],
            digits[((tm.tm_year + 1900) / 100) % 10],
            digits[((tm.tm_year + 1900) / 10) % 10],
            digits[(tm.tm_year + 1900) % 10],
            '-',
            digits[(tm.tm_mon + 1) / 10],
            digits[(tm.tm_mon + 1) % 10],
            '-',
            digits[tm.tm_mday / 10],
            digits[tm.tm_mday % 10],
            'T',
            digits[tm.tm_hour / 10],
            digits[tm.tm_hour % 10],
            ':',
            digits[tm.tm_min / 10],
            digits[tm.tm_min % 10],
            ':',
            digits[tm.tm_sec / 10],
            digits[tm.tm_sec % 10],
            '.',
            digits[ms / 100],
            digits[(ms / 10) % 10],
            digits[ms % 10],
            'Z',
            '\0'
        };

        return {buffer};
    }
} // namespace nanoenv::platform
