#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace uniconv::builtins {

class Clipboard {
public:
    struct Result {
        bool success = true;
        std::filesystem::path output;  // Pass-through input path
        std::string error;
    };

    // Execute clipboard operation
    // Copies file content to system clipboard, returns input path unchanged
    static Result execute(const std::filesystem::path& input);

    // Validate clipboard usage in pipeline context
    struct ValidationResult {
        bool valid = true;
        std::string error;
    };

    // Clipboard cannot be the first stage (needs input)
    static ValidationResult validate(size_t current_stage_index);

    // Check if a target name is the clipboard builtin
    static bool is_clipboard(const std::string& target);

    // Copy a file path to clipboard (public, for finalize phase)
    static bool copy_path(const std::filesystem::path& file_path);

    // --- Clipboard input (read) support ---
    struct ReadResult {
        bool success = false;
        std::filesystem::path file;
        std::string detected_format;  // "png", "txt", etc.
        std::string error;
    };

    // Read clipboard content into a temp file.
    // format_hint: if provided, constrains what type of content to read.
    static ReadResult read_to_file(
        const std::filesystem::path& temp_dir,
        const std::optional<std::string>& format_hint = std::nullopt);

private:
    // Copy text file content to clipboard
    static bool copy_text_to_clipboard(const std::filesystem::path& file, std::string& error);

    // Copy image to clipboard
    static bool copy_image_to_clipboard(const std::filesystem::path& file, std::string& error);

    // Copy file path as text to clipboard
    static bool copy_path_to_clipboard(const std::filesystem::path& file, std::string& error);

    // Detect file type
    static bool is_image_file(const std::filesystem::path& file);
    static bool is_text_file(const std::filesystem::path& file);

    // Platform-specific raw clipboard operations (write)
    static bool write_text_to_clipboard(const std::string& text, std::string& error);
    static bool write_image_to_clipboard(const std::filesystem::path& image_path, std::string& error);

    // Platform-specific raw clipboard operations (read)
    struct RawContent {
        std::string data;
        std::string format;  // "png", "txt", etc.
    };
    static std::optional<RawContent> read_image_from_clipboard(std::string& error);
    static std::optional<RawContent> read_text_from_clipboard(std::string& error);
};

} // namespace uniconv::builtins
