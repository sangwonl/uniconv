#pragma once

#include <filesystem>
#include <string>

namespace uniconv::builtins {

class Passthrough {
public:
    struct Result {
        bool success = true;
        std::filesystem::path output;
        std::string error;
    };

    // Execute passthrough operation
    // Takes input and returns it unchanged (bypass)
    static Result execute(const std::filesystem::path& input);

    // Check if a target name is the passthrough builtin
    // Accepts: "_", "echo", "bypass", "pass", "noop"
    static bool is_passthrough(const std::string& target);
};

} // namespace uniconv::builtins
