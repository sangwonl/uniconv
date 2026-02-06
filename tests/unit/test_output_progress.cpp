#include <gtest/gtest.h>
#include <sstream>
#include <nlohmann/json.hpp>
#include "core/output/console_output.h"
#include "core/output/json_output.h"

using namespace uniconv::core::output;

// ConsoleOutput tests (non-TTY path since ostringstream isn't a real fd)

TEST(ConsoleOutputProgress, StageStartedNonTTY) {
    std::ostringstream out, err;
    ConsoleOutput output(out, err);

    output.stage_started(1, 3, "jpg");

    std::string result = err.str();
    EXPECT_NE(result.find("[1/3]"), std::string::npos);
    EXPECT_NE(result.find("Converting to jpg..."), std::string::npos);
}

TEST(ConsoleOutputProgress, StageCompletedSuccessNonTTY) {
    std::ostringstream out, err;
    ConsoleOutput output(out, err);

    output.stage_completed(1, 3, "jpg", 1200, true);

    std::string result = err.str();
    EXPECT_NE(result.find("[1/3]"), std::string::npos);
    EXPECT_NE(result.find("jpg"), std::string::npos);
    EXPECT_NE(result.find("OK"), std::string::npos);
    EXPECT_NE(result.find("1.2s"), std::string::npos);
}

TEST(ConsoleOutputProgress, StageCompletedFailureNonTTY) {
    std::ostringstream out, err;
    ConsoleOutput output(out, err);

    output.stage_completed(2, 3, "png", 500, false, "plugin not found");

    std::string result = err.str();
    EXPECT_NE(result.find("[2/3]"), std::string::npos);
    EXPECT_NE(result.find("FAIL"), std::string::npos);
    EXPECT_NE(result.find("plugin not found"), std::string::npos);
}

TEST(ConsoleOutputProgress, QuietModeSuppressesProgress) {
    std::ostringstream out, err;
    ConsoleOutput output(out, err, false, true); // quiet = true

    output.stage_started(1, 3, "jpg");
    output.stage_completed(1, 3, "jpg", 1200, true);

    EXPECT_TRUE(err.str().empty());
}

// JsonOutput tests

TEST(JsonOutputProgress, StageStartedEmitsValidJSON) {
    std::ostringstream out, err;
    JsonOutput output(out, err);

    output.stage_started(1, 3, "jpg");

    std::string result = err.str();
    ASSERT_FALSE(result.empty());

    auto j = nlohmann::json::parse(result);
    EXPECT_EQ(j["type"], "progress");
    EXPECT_EQ(j["current"], 1);
    EXPECT_EQ(j["total"], 3);
    EXPECT_EQ(j["target"], "jpg");
    EXPECT_EQ(j["phase"], "started");
}

TEST(JsonOutputProgress, StageCompletedSuccessEmitsValidJSON) {
    std::ostringstream out, err;
    JsonOutput output(out, err);

    output.stage_completed(2, 3, "png", 800, true);

    std::string result = err.str();
    ASSERT_FALSE(result.empty());

    auto j = nlohmann::json::parse(result);
    EXPECT_EQ(j["type"], "progress");
    EXPECT_EQ(j["current"], 2);
    EXPECT_EQ(j["total"], 3);
    EXPECT_EQ(j["target"], "png");
    EXPECT_EQ(j["phase"], "completed");
    EXPECT_EQ(j["duration_ms"], 800);
    EXPECT_EQ(j["success"], true);
    EXPECT_FALSE(j.contains("error"));
}

TEST(JsonOutputProgress, StageCompletedFailureIncludesError) {
    std::ostringstream out, err;
    JsonOutput output(out, err);

    output.stage_completed(1, 2, "webp", 300, false, "conversion failed");

    std::string result = err.str();
    ASSERT_FALSE(result.empty());

    auto j = nlohmann::json::parse(result);
    EXPECT_EQ(j["success"], false);
    EXPECT_EQ(j["error"], "conversion failed");
}

TEST(JsonOutputProgress, QuietModeSuppressesProgress) {
    std::ostringstream out, err;
    JsonOutput output(out, err, false, true); // quiet = true

    output.stage_started(1, 3, "jpg");
    output.stage_completed(1, 3, "jpg", 1200, true);

    EXPECT_TRUE(err.str().empty());
}
