#include "passthrough.h"
#include <algorithm>
#include <cctype>

namespace uniconv::builtins {

Passthrough::Result Passthrough::execute(const std::filesystem::path& input) {
    Result result;

    // Validate input exists
    if (!std::filesystem::exists(input)) {
        result.success = false;
        result.error = "Input file does not exist: " + input.string();
        return result;
    }

    // Passthrough just returns the input unchanged
    result.output = input;
    result.success = true;
    return result;
}

bool Passthrough::is_passthrough(const std::string& target) {
    // Case-insensitive comparison
    std::string lower_target = target;
    std::transform(lower_target.begin(), lower_target.end(), lower_target.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Accept multiple aliases for passthrough
    return lower_target == "_" ||
           lower_target == "echo" ||
           lower_target == "bypass" ||
           lower_target == "pass" ||
           lower_target == "noop";
}

} // namespace uniconv::builtins
