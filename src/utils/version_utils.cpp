#include "version_utils.h"
#include <charconv>

namespace uniconv::utils
{

    std::optional<SemVer> parse_semver(const std::string &s)
    {
        SemVer ver;

        // Skip leading 'v' or 'V'
        const char *start = s.c_str();
        if (*start == 'v' || *start == 'V')
            ++start;

        const char *end = s.c_str() + s.size();

        // Parse major
        auto [ptr1, ec1] = std::from_chars(start, end, ver.major);
        if (ec1 != std::errc{} || ptr1 == end || *ptr1 != '.')
            return std::nullopt;

        // Parse minor
        auto [ptr2, ec2] = std::from_chars(ptr1 + 1, end, ver.minor);
        if (ec2 != std::errc{})
            return std::nullopt;

        // Patch is optional
        if (ptr2 != end && *ptr2 == '.')
        {
            auto [ptr3, ec3] = std::from_chars(ptr2 + 1, end, ver.patch);
            if (ec3 != std::errc{})
                return std::nullopt;
        }

        return ver;
    }

    int compare_versions(const std::string &a, const std::string &b)
    {
        auto va = parse_semver(a);
        auto vb = parse_semver(b);

        if (!va || !vb)
            return 0;

        if (*va < *vb)
            return -1;
        if (*va > *vb)
            return 1;
        return 0;
    }

    bool satisfies_constraint(const std::string &version, const std::string &constraint)
    {
        if (constraint.empty())
            return true;

        auto ver = parse_semver(version);
        if (!ver)
            return false;

        // Parse operator and version from constraint
        std::string op;
        std::string constraint_ver;

        size_t i = 0;
        while (i < constraint.size() && (constraint[i] == '>' || constraint[i] == '<' || constraint[i] == '=' || constraint[i] == '!'))
        {
            op += constraint[i];
            ++i;
        }

        // Skip whitespace between operator and version
        while (i < constraint.size() && constraint[i] == ' ')
            ++i;

        constraint_ver = constraint.substr(i);

        auto cver = parse_semver(constraint_ver);
        if (!cver)
            return false;

        if (op == ">=" || op.empty())
            return *ver >= *cver;
        if (op == ">")
            return *ver > *cver;
        if (op == "<=")
            return *ver <= *cver;
        if (op == "<")
            return *ver < *cver;
        if (op == "=" || op == "==")
            return *ver == *cver;

        return false;
    }

} // namespace uniconv::utils
