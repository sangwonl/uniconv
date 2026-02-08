#include "collect.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace uniconv::builtins {

Collect::Result Collect::execute(
    const std::vector<std::filesystem::path>& inputs,
    const std::filesystem::path& temp_dir
) {
    Result result;

    if (inputs.empty()) {
        result.success = false;
        result.error = "Collect requires at least one input file";
        return result;
    }

    // Create the collect directory inside the temp dir
    std::filesystem::path collect_dir = temp_dir / "collected";
    try {
        std::filesystem::create_directories(collect_dir);
    } catch (const std::filesystem::filesystem_error& e) {
        result.success = false;
        result.error = "Failed to create collect directory: " + std::string(e.what());
        return result;
    }

    // Copy/link each input file into the collect directory with ordered names
    // Format: 000_originalname.ext, 001_originalname.ext, ...
    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& input = inputs[i];

        if (!std::filesystem::exists(input)) {
            result.success = false;
            result.error = "Input file does not exist: " + input.string();
            return result;
        }

        // Build ordered filename: {index}_{original_filename}
        std::ostringstream prefix;
        prefix << std::setw(4) << std::setfill('0') << i;
        std::string ordered_name = prefix.str() + "_" + input.filename().string();
        std::filesystem::path dest = collect_dir / ordered_name;

        try {
            std::filesystem::copy_file(input, dest,
                std::filesystem::copy_options::overwrite_existing);
        } catch (const std::filesystem::filesystem_error& e) {
            result.success = false;
            result.error = "Failed to collect file: " + std::string(e.what());
            return result;
        }
    }

    result.success = true;
    result.output_dir = collect_dir;
    return result;
}

Collect::ValidationResult Collect::validate(
    size_t current_stage_index,
    size_t total_stages
) {
    ValidationResult result;

    // Collect cannot be the first stage (needs scattered inputs)
    if (current_stage_index == 0) {
        result.valid = false;
        result.error = "Collect cannot be the first stage (it requires scattered inputs)";
        return result;
    }

    (void)total_stages;
    result.valid = true;
    return result;
}

bool Collect::is_collect(const std::string& target) {
    std::string lower_target = target;
    std::transform(lower_target.begin(), lower_target.end(), lower_target.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return lower_target == "collect";
}

} // namespace uniconv::builtins
