#include "common/logger.h"

#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace {

// Cuenta cuántas veces se lo formatea.
struct FormatCounter {
    int* count;
};

}  // namespace

template <>
struct std::formatter<FormatCounter>: std::formatter<int> {
    auto format(const FormatCounter& counter, std::format_context& ctx) const {
        ++*counter.count;
        return std::formatter<int>::format(*counter.count, ctx);
    }
};

namespace {

constexpr int THREADS = 8;
constexpr int LINES_PER_THREAD = 200;

class LoggerTest: public ::testing::Test {
protected:
    void SetUp() override {
        Logger::set_output(out);
        Logger::set_level(LogLevel::Debug);
    }

    // El logger es global: se restaura para no afectar a otros tests.
    void TearDown() override {
        Logger::set_output(std::cerr);
        Logger::set_level(LogLevel::Info);
    }

    std::vector<std::string> lines() const {
        std::vector<std::string> result;
        std::istringstream in(out.str());
        std::string line;
        while (std::getline(in, line)) {
            result.push_back(line);
        }
        return result;
    }

    std::ostringstream out;
};

TEST_F(LoggerTest, LineHasTimestampLevelThreadLocationAndMessage) {
    Logger::info("hola {} {}", "mundo", 42);
    std::vector<std::string> result = lines();
    ASSERT_EQ(result.size(), 1u);
    std::regex expected(
            R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\] \[INFO \] \[T\d+\] \[logger_test\.cpp:\d+\] hola mundo 42)");
    EXPECT_TRUE(std::regex_match(result[0], expected)) << result[0];
}

TEST_F(LoggerTest, ThreadNumberIsStablePerThreadAndDistinctAcrossThreads) {
    Logger::info("a");
    Logger::info("b");
    std::thread other([]() { Logger::info("c"); });
    other.join();
    std::vector<std::string> result = lines();
    ASSERT_EQ(result.size(), 3u);
    std::regex tag(R"(\[(T\d+)\])");
    std::smatch first, second, third;
    ASSERT_TRUE(std::regex_search(result[0], first, tag));
    ASSERT_TRUE(std::regex_search(result[1], second, tag));
    ASSERT_TRUE(std::regex_search(result[2], third, tag));
    EXPECT_EQ(first[1], second[1]);
    EXPECT_NE(first[1], third[1]);
}

TEST_F(LoggerTest, LocationIsTheCallSite) {
    int line = __LINE__ + 1;
    Logger::warn("aca");
    EXPECT_NE(out.str().find("[logger_test.cpp:" + std::to_string(line) + "]"), std::string::npos)
            << out.str();
}

TEST_F(LoggerTest, EachLevelHasItsTag) {
    Logger::debug("d");
    Logger::info("i");
    Logger::warn("w");
    Logger::error("e");
    std::vector<std::string> result = lines();
    ASSERT_EQ(result.size(), 4u);
    EXPECT_NE(result[0].find("[DEBUG]"), std::string::npos);
    EXPECT_NE(result[1].find("[INFO ]"), std::string::npos);
    EXPECT_NE(result[2].find("[WARN ]"), std::string::npos);
    EXPECT_NE(result[3].find("[ERROR]"), std::string::npos);
}

TEST_F(LoggerTest, DiscardsMessagesBelowLevel) {
    Logger::set_level(LogLevel::Warn);
    Logger::debug("no");
    Logger::info("no");
    Logger::warn("si");
    Logger::error("si");
    EXPECT_EQ(lines().size(), 2u);
}

TEST_F(LoggerTest, OffDiscardsEverything) {
    Logger::set_level(LogLevel::Off);
    Logger::error("no");
    EXPECT_TRUE(out.str().empty());
}

TEST_F(LoggerTest, DoesNotFormatDiscardedMessages) {
    Logger::set_level(LogLevel::Error);
    int formatted = 0;
    Logger::debug("{}", FormatCounter{&formatted});
    EXPECT_EQ(formatted, 0);
    Logger::error("{}", FormatCounter{&formatted});
    EXPECT_EQ(formatted, 1);
}

TEST_F(LoggerTest, ConcurrentLinesDoNotInterleave) {
    std::vector<std::thread> threads;
    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back([t]() {
            for (int i = 0; i < LINES_PER_THREAD; ++i) {
                Logger::info("thread {} linea {}", t, i);
            }
        });
    }
    for (std::thread& thread: threads) {
        thread.join();
    }
    std::vector<std::string> result = lines();
    ASSERT_EQ(result.size(), static_cast<std::size_t>(THREADS * LINES_PER_THREAD));
    std::regex line_end(R"(.*\] thread \d+ linea \d+)");
    for (const std::string& line: result) {
        EXPECT_TRUE(std::regex_match(line, line_end)) << line;
    }
}

}  // namespace
