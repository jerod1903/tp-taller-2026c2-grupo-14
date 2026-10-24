#ifndef LOGGER_H
#define LOGGER_H

#include <atomic>
#include <cstdint>
#include <format>
#include <mutex>
#include <ostream>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

enum class LogLevel : uint8_t {
    Debug,
    Info,
    Warn,
    Error,
    Off,
};

template <typename... Args>
struct LogFormat {
    std::format_string<Args...> fmt;
    std::source_location location;

    // Implícito a propósito: permite llamar Logger::info("...") con un string literal.
    template <typename T>
    // cppcheck-suppress noExplicitConstructor
    consteval LogFormat(const T& fmt,
                        std::source_location location = std::source_location::current()):
            fmt(fmt), location(location) {}
};

// Logger global y thread-safe. Formatea fuera del lock y solo serializa la escritura,
// así las líneas de distintos threads no se mezclan.
class Logger {
public:
    static void set_level(LogLevel level);
    static LogLevel get_level();
    // Por defecto escribe a std::cerr. El stream tiene que vivir más que cualquier log.
    static void set_output(std::ostream& out);

    template <typename... Args>
    static void debug(LogFormat<std::type_identity_t<Args>...> fmt, Args&&... args) {
        log(LogLevel::Debug, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void info(LogFormat<std::type_identity_t<Args>...> fmt, Args&&... args) {
        log(LogLevel::Info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void warn(LogFormat<std::type_identity_t<Args>...> fmt, Args&&... args) {
        log(LogLevel::Warn, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void error(LogFormat<std::type_identity_t<Args>...> fmt, Args&&... args) {
        log(LogLevel::Error, fmt, std::forward<Args>(args)...);
    }

private:
    static std::atomic<LogLevel> level;
    static std::ostream* output;
    static std::mutex mutex;

    static bool enabled(LogLevel msg_level);
    static void write(LogLevel msg_level, const std::source_location& location,
                      std::string_view message);

    template <typename... Args>
    static void log(LogLevel msg_level, const LogFormat<std::type_identity_t<Args>...>& fmt,
                    Args&&... args) {
        if (!enabled(msg_level)) {
            return;
        }
        write(msg_level, fmt.location, std::format(fmt.fmt, std::forward<Args>(args)...));
    }
};

#endif  // LOGGER_H
