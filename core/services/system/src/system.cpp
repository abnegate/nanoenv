#include "system.h"
#include <chrono>
#include <random>
#include <string>

namespace nanoenv::system {
    static bool fastHardwareRNG(
        std::array<uint8_t, 16> &bytes
    ) {
#if defined(__x86_64__)
        // Use RDRAND (hardware RNG) if available
        uint64_t r1, r2;
        if (_rdrand64_step(&r1) && _rdrand64_step(&r2)) {
            std::memcpy(bytes.data(), &r1, 8);
            std::memcpy(bytes.data() + 8, &r2, 8);
            return true;
        }
#elif defined(__aarch64__)
        // Use `rdseed` for ARM64 (when available)
        uint64_t r1 = 0, r2 = 0;
        asm volatile("mrs %0, CNTVCT_EL0" : "=r"(r1));
        asm volatile("mrs %0, CNTPCT_EL0" : "=r"(r2));
        std::memcpy(bytes.data(), &r1, 8);
        std::memcpy(bytes.data() + 8, &r2, 8);
        return true;
#else
        return false;
#endif
    }

    static void fallbackPRNG(
        std::array<uint8_t, 16> &bytes
    ) {
        thread_local std::random_device rd;
        thread_local std::mt19937_64 rng(rd());

        const uint64_t r1 = rng();
        const uint64_t r2 = rng();
        std::memcpy(bytes.data(), &r1, 8);
        std::memcpy(bytes.data() + 8, &r2, 8);
    }

    static std::string formatUUID(
        const std::array<uint8_t, 16> &bytes
    ) {
        constexpr char hex_chars[] = "0123456789abcdef";
        char buffer[37] = {};

        uint8_t index = 0;
        for (uint8_t i = 0; i < 16; ++i) {
            buffer[index++] = hex_chars[bytes[i] >> 4];
            buffer[index++] = hex_chars[bytes[i] & 0xF];

            if (index == 8 || index == 13 || index == 18 || index == 23) {
                buffer[index++] = '-';
            }
        }

        buffer[36] = '\0';

        return {buffer};
    }

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

    std::string System::generateUUID() {
        alignas(16) std::array<uint8_t, 16> bytes{};

#if defined(__x86_64__) || defined(__aarch64__)
        // Fastest approach using CPU's random number generator (if supported)
        if (fastHardwareRNG(bytes)) {
            return formatUUID(bytes);
        }
#endif

        // Fast PRNG using xoshiro256++
        fallbackPRNG(bytes);
        return formatUUID(bytes);
    }

} // namespace nanoenv::system
