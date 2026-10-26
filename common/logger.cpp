#include "logger.h"

#include <chrono>
#include <iostream>
#include <string>

namespace {

constexpr std::string_view level_name(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO ";
        case LogLevel::Warn:
            return "WARN ";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Off:
            break;
    }
    return "?????";
}

// Solo el nombre del archivo, sin el path. Esto es para que los logs sean más legibles y no
// dependan de la estructura de carpetas del proyecto.
std::string_view file_name(const char* path) {
    std::string_view view(path);
    std::string_view::size_type slash = view.find_last_of('/');
    return slash == std::string_view::npos ? view : view.substr(slash + 1);
}

// Numera los threads en el orden en que loguean por primera vez: T1, T2, ...
unsigned thread_number() {
    static std::atomic<unsigned> next{1};
    thread_local const unsigned number = next++;
    return number;
}

}  // namespace

std::atomic<LogLevel> Logger::level{LogLevel::Info};
std::ostream* Logger::output = &std::cerr;
std::mutex Logger::mutex;

void Logger::set_level(LogLevel new_level) { level = new_level; }

LogLevel Logger::get_level() { return level; }

void Logger::set_output(std::ostream& out) {
    std::lock_guard<std::mutex> lock(mutex);
    output = &out;
}

bool Logger::enabled(LogLevel msg_level) {
    return msg_level != LogLevel::Off && msg_level >= level.load();
}

void Logger::write(LogLevel msg_level, const std::source_location& location,
                   std::string_view message) {
    std::chrono::zoned_time now{
            std::chrono::current_zone(),
            std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now())};
    std::string line =
            std::format("[{:%F %T}] [{}] [T{}] [{}:{}] {}\n", now, level_name(msg_level),
                        thread_number(), file_name(location.file_name()), location.line(), message);
    std::lock_guard<std::mutex> lock(mutex);
    *output << line << std::flush;
}
