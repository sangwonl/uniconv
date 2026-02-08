#pragma once

#include <filesystem>
#include <vector>
#include <string>

namespace uniconv::builtins {

class Collect {
public:
    struct Result {
        bool success = true;
        std::filesystem::path output_dir;  // Directory containing collected files
        std::string error;
    };

    // Execute collect operation
    // Gathers N input files into a single temp directory, preserving order.
    // Files are copied/linked into the directory with ordered names.
    static Result execute(
        const std::vector<std::filesystem::path>& inputs,
        const std::filesystem::path& temp_dir
    );

    // Validate collect usage in pipeline context
    struct ValidationResult {
        bool valid = true;
        std::string error;
    };

    static ValidationResult validate(
        size_t current_stage_index,
        size_t total_stages
    );

    // Check if a target name is the collect builtin
    static bool is_collect(const std::string& target);
};

} // namespace uniconv::builtins
