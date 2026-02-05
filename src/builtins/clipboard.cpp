#include "clipboard.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace uniconv::builtins {

namespace {

// Execute a command and capture its exit status
int execute_command(const std::string& cmd) {
    return std::system(cmd.c_str());
}

// Escape string for shell
std::string shell_escape(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.size() + 10);
    escaped += '\'';
    for (char c : str) {
        if (c == '\'') {
            escaped += "'\\''";
        } else {
            escaped += c;
        }
    }
    escaped += '\'';
    return escaped;
}

} // anonymous namespace

Clipboard::Result Clipboard::execute(const std::filesystem::path& input) {
    Result result;
    result.output = input;  // Always pass through the input

    // Validate input exists
    if (!std::filesystem::exists(input)) {
        result.success = false;
        result.error = "Input file does not exist: " + input.string();
        return result;
    }

    // Check file size (warn for large files >10MB but still proceed)
    auto file_size = std::filesystem::file_size(input);
    if (file_size > 10 * 1024 * 1024) {
        // We proceed but this could be slow
    }

    std::string error;
    bool copy_success = false;

    // Detect file type and copy appropriately
    if (is_image_file(input)) {
        copy_success = copy_image_to_clipboard(input, error);
    } else if (is_text_file(input)) {
        copy_success = copy_text_to_clipboard(input, error);
    } else {
        // For other file types, copy the file path
        copy_success = copy_path_to_clipboard(input, error);
    }

    if (!copy_success) {
        result.success = false;
        result.error = error;
        return result;
    }

    result.success = true;
    return result;
}

Clipboard::ValidationResult Clipboard::validate([[maybe_unused]] size_t current_stage_index) {
    // Clipboard can be at any position in the pipeline.
    // When it's the first stage (index 0), it receives input from the source file.
    // When it's a later stage, it receives input from the previous stage.
    // The source file existence is validated elsewhere.
    return ValidationResult{true, ""};
}

bool Clipboard::is_clipboard(const std::string& target) {
    // Case-insensitive comparison
    std::string lower_target = target;
    std::transform(lower_target.begin(), lower_target.end(), lower_target.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return lower_target == "clipboard";
}

bool Clipboard::copy_path(const std::filesystem::path& file_path) {
    std::string error;
    return copy_path_to_clipboard(file_path, error);
}

bool Clipboard::is_image_file(const std::filesystem::path& file) {
    std::string ext = file.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    static const std::array<std::string, 7> image_extensions = {
        ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff", ".webp"
    };

    for (const auto& img_ext : image_extensions) {
        if (ext == img_ext) {
            return true;
        }
    }
    return false;
}

bool Clipboard::is_text_file(const std::filesystem::path& file) {
    std::string ext = file.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    static const std::array<std::string, 10> text_extensions = {
        ".txt", ".md", ".json", ".xml", ".csv", ".html", ".htm",
        ".yaml", ".yml", ".log"
    };

    for (const auto& txt_ext : text_extensions) {
        if (ext == txt_ext) {
            return true;
        }
    }
    return false;
}

bool Clipboard::copy_text_to_clipboard(const std::filesystem::path& file, std::string& error) {
    // Read file content
    std::ifstream ifs(file);
    if (!ifs) {
        error = "Failed to open file for reading: " + file.string();
        return false;
    }

    std::ostringstream ss;
    ss << ifs.rdbuf();
    std::string content = ss.str();

    return write_text_to_clipboard(content, error);
}

bool Clipboard::copy_image_to_clipboard(const std::filesystem::path& file, std::string& error) {
    return write_image_to_clipboard(file, error);
}

bool Clipboard::copy_path_to_clipboard(const std::filesystem::path& file, std::string& error) {
    return write_text_to_clipboard(std::filesystem::absolute(file).string(), error);
}

// Platform-specific implementations

#if defined(__APPLE__)

bool Clipboard::write_text_to_clipboard(const std::string& text, std::string& error) {
    // Use pbcopy on macOS
    std::string escaped = shell_escape(text);
    std::string cmd = "printf %s " + escaped + " | pbcopy";

    int ret = execute_command(cmd);
    if (ret != 0) {
        error = "Failed to copy text to clipboard (pbcopy failed)";
        return false;
    }
    return true;
}

bool Clipboard::write_image_to_clipboard(const std::filesystem::path& image_path, std::string& error) {
    // Use osascript with JavaScript to copy image to clipboard on macOS
    // This avoids the Unicode character issues with AppleScript's «class» syntax
    std::string abs_path = std::filesystem::absolute(image_path).string();

    // Escape single quotes in the path
    std::string escaped_abs_path;
    for (char c : abs_path) {
        if (c == '\'') {
            escaped_abs_path += "\\'";
        } else {
            escaped_abs_path += c;
        }
    }

    // Use osascript with Objective-C bridge via JavaScript (JXA)
    // This is more reliable than AppleScript for image handling
    std::string cmd = "osascript -l JavaScript -e '"
        "ObjC.import(\"Cocoa\"); "
        "var path = \"" + escaped_abs_path + "\"; "
        "var image = $.NSImage.alloc.initWithContentsOfFile(path); "
        "if (image.isNil()) { \"error\"; } else { "
        "var pb = $.NSPasteboard.generalPasteboard; "
        "pb.clearContents; "
        "var arr = $.NSArray.arrayWithObject(image); "
        "pb.writeObjects(arr); "
        "\"success\"; }' >/dev/null 2>&1";

    int ret = execute_command(cmd);
    if (ret != 0) {
        // Fallback: copy file path as text
        return write_text_to_clipboard(abs_path, error);
    }

    return true;
}

#elif defined(__linux__)

bool Clipboard::write_text_to_clipboard(const std::string& text, std::string& error) {
    // Try xclip first, then xsel
    std::string escaped = shell_escape(text);

    // Check if we have a display
    const char* display = std::getenv("DISPLAY");
    const char* wayland = std::getenv("WAYLAND_DISPLAY");

    if (!display && !wayland) {
        error = "No display available (DISPLAY or WAYLAND_DISPLAY not set)";
        return false;
    }

    // Try xclip
    std::string cmd = "printf %s " + escaped + " | xclip -selection clipboard 2>/dev/null";
    int ret = execute_command(cmd);
    if (ret == 0) {
        return true;
    }

    // Try xsel as fallback
    cmd = "printf %s " + escaped + " | xsel --clipboard --input 2>/dev/null";
    ret = execute_command(cmd);
    if (ret == 0) {
        return true;
    }

    // Try wl-copy for Wayland
    if (wayland) {
        cmd = "printf %s " + escaped + " | wl-copy 2>/dev/null";
        ret = execute_command(cmd);
        if (ret == 0) {
            return true;
        }
    }

    error = "Failed to copy text to clipboard (xclip, xsel, and wl-copy all failed)";
    return false;
}

bool Clipboard::write_image_to_clipboard(const std::filesystem::path& image_path, std::string& error) {
    std::string escaped_path = shell_escape(std::filesystem::absolute(image_path).string());

    const char* display = std::getenv("DISPLAY");
    const char* wayland = std::getenv("WAYLAND_DISPLAY");

    if (!display && !wayland) {
        error = "No display available (DISPLAY or WAYLAND_DISPLAY not set)";
        return false;
    }

    std::string ext = image_path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::string mime_type = "image/png";
    if (ext == ".jpg" || ext == ".jpeg") {
        mime_type = "image/jpeg";
    } else if (ext == ".gif") {
        mime_type = "image/gif";
    } else if (ext == ".bmp") {
        mime_type = "image/bmp";
    } else if (ext == ".webp") {
        mime_type = "image/webp";
    }

    // Try xclip
    std::string cmd = "xclip -selection clipboard -t " + mime_type + " -i " + escaped_path + " 2>/dev/null";
    int ret = execute_command(cmd);
    if (ret == 0) {
        return true;
    }

    // Try wl-copy for Wayland
    if (wayland) {
        cmd = "wl-copy --type " + mime_type + " < " + escaped_path + " 2>/dev/null";
        ret = execute_command(cmd);
        if (ret == 0) {
            return true;
        }
    }

    error = "Failed to copy image to clipboard (xclip and wl-copy failed)";
    return false;
}

#elif defined(_WIN32)

bool Clipboard::write_text_to_clipboard(const std::string& text, std::string& error) {
    // Use PowerShell Set-Clipboard
    std::string escaped = shell_escape(text);
    std::string cmd = "powershell -command \"Set-Clipboard -Value " + escaped + "\"";

    int ret = execute_command(cmd);
    if (ret != 0) {
        error = "Failed to copy text to clipboard (PowerShell Set-Clipboard failed)";
        return false;
    }
    return true;
}

bool Clipboard::write_image_to_clipboard(const std::filesystem::path& image_path, std::string& error) {
    // Use PowerShell to copy image
    std::string escaped_path = shell_escape(std::filesystem::absolute(image_path).string());

    std::string cmd = "powershell -command \"Add-Type -AssemblyName System.Windows.Forms; "
                      "[System.Windows.Forms.Clipboard]::SetImage([System.Drawing.Image]::FromFile(" +
                      escaped_path + "))\"";

    int ret = execute_command(cmd);
    if (ret != 0) {
        error = "Failed to copy image to clipboard (PowerShell failed)";
        return false;
    }
    return true;
}

#else

// Unsupported platform
bool Clipboard::write_text_to_clipboard(const std::string& /*text*/, std::string& error) {
    error = "Clipboard operations not supported on this platform";
    return false;
}

bool Clipboard::write_image_to_clipboard(const std::filesystem::path& /*image_path*/, std::string& error) {
    error = "Clipboard operations not supported on this platform";
    return false;
}

#endif

} // namespace uniconv::builtins
