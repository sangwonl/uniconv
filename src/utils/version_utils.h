#pragma once

#include <optional>
#include <string>

namespace uniconv::utils
{

    struct SemVer
    {
        int major = 0;
        int minor = 0;
        int patch = 0;

        bool operator==(const SemVer &other) const = default;
        auto operator<=>(const SemVer &other) const = default;
    };

    // Parse a semver string like "1.2.3" or "0.1.0"
    std::optional<SemVer> parse_semver(const std::string &s);

    // Compare two version strings. Returns -1, 0, or 1.
    // Returns 0 if either is unparseable.
    int compare_versions(const std::string &a, const std::string &b);

    // Check if a version satisfies a constraint like ">=0.3.0"
    // Supported operators: >=, >, <=, <, =, ==
    bool satisfies_constraint(const std::string &version, const std::string &constraint);

} // namespace uniconv::utils
