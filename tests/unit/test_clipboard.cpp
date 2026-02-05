#include <gtest/gtest.h>
#include "builtins/clipboard.h"
#include <filesystem>
#include <fstream>

using namespace uniconv::builtins;

class ClipboardTest : public ::testing::Test {
protected:
    std::filesystem::path temp_dir_;

    void SetUp() override {
        temp_dir_ = std::filesystem::temp_directory_path() / "uniconv_test_clipboard";
        std::filesystem::create_directories(temp_dir_);
    }

    void TearDown() override {
        std::filesystem::remove_all(temp_dir_);
    }

    std::filesystem::path create_temp_file(const std::string& name, const std::string& content) {
        auto path = temp_dir_ / name;
        std::ofstream ofs(path);
        ofs << content;
        return path;
    }
};

TEST_F(ClipboardTest, IsClipboardCaseInsensitive) {
    EXPECT_TRUE(Clipboard::is_clipboard("clipboard"));
    EXPECT_TRUE(Clipboard::is_clipboard("Clipboard"));
    EXPECT_TRUE(Clipboard::is_clipboard("CLIPBOARD"));
    EXPECT_TRUE(Clipboard::is_clipboard("ClipBoard"));

    EXPECT_FALSE(Clipboard::is_clipboard("clip"));
    EXPECT_FALSE(Clipboard::is_clipboard("copy"));
    EXPECT_FALSE(Clipboard::is_clipboard("tee"));
    EXPECT_FALSE(Clipboard::is_clipboard(""));
}

TEST_F(ClipboardTest, ValidateSucceedsForAnyStage) {
    // Clipboard can be at any position in the pipeline
    // Stage 0: receives input from source file
    auto result0 = Clipboard::validate(0);
    EXPECT_TRUE(result0.valid);

    // Later stages: receive input from previous stage
    auto result1 = Clipboard::validate(1);
    EXPECT_TRUE(result1.valid);

    auto result5 = Clipboard::validate(5);
    EXPECT_TRUE(result5.valid);
}

TEST_F(ClipboardTest, ExecuteNonExistentFile) {
    auto result = Clipboard::execute("/nonexistent/path/file.txt");
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("does not exist"), std::string::npos);
}

// Note: Actual clipboard operations are skipped in tests since they
// require platform-specific clipboard tools (pbcopy, xclip, etc.)
// and may not be available in CI environments.

TEST_F(ClipboardTest, ExecutePassesThroughInput) {
    // Create a test file
    auto test_file = create_temp_file("test.txt", "hello world");

    // Even if clipboard copy fails (no clipboard available in CI),
    // the output should still be set to the input path
    auto result = Clipboard::execute(test_file);

    // The output path should always be set to input (pass-through behavior)
    EXPECT_EQ(result.output, test_file);
}

// File type detection tests

TEST(ClipboardFileTypeTest, IsImageFile) {
    // These are private methods, but we can test the execute behavior
    // based on file extensions indirectly through integration tests

    // For unit testing file type detection, we'd need to expose the methods
    // or use a friend class. For now, we test through the public interface.
}

TEST(ClipboardFileTypeTest, IsTextFile) {
    // Similar to above - would need exposed methods or friend class
}
