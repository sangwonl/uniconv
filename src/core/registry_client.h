#pragma once

#include "registry_types.h"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace uniconv::core
{

    class RegistryClient
    {
    public:
        RegistryClient(const std::string &registry_url,
                       const std::filesystem::path &cache_dir);

        // Fetch the registry index (uses cache with TTL)
        std::optional<RegistryIndex> fetch_index(bool force_refresh = false);

        // Fetch a specific plugin's registry entry
        std::optional<RegistryPluginEntry> fetch_plugin(const std::string &name);

        // Search plugins by query (matches name, description, keywords)
        std::vector<RegistryIndexEntry> search(const std::string &query);

        // Resolve the best release for the current platform and uniconv version
        std::optional<RegistryRelease> resolve_release(
            const RegistryPluginEntry &entry,
            const std::optional<std::string> &requested_version = std::nullopt);

        // Resolve artifact URL for current platform
        // Checks platform-specific first, then "any"
        std::optional<RegistryArtifact> resolve_artifact(
            const RegistryRelease &release);

        // Fetch the collections list from the registry
        std::optional<RegistryCollections> fetch_collections();

        // Find a collection by name
        std::optional<RegistryCollection> find_collection(const std::string &name);

        // Download and verify an artifact, extract to destination directory
        // Returns path to extracted plugin directory
        std::optional<std::filesystem::path> download_and_extract(
            const RegistryArtifact &artifact,
            const std::filesystem::path &dest_dir);

    private:
        std::string registry_url_;
        std::filesystem::path cache_dir_;
        static constexpr int kCacheTTLMinutes = 60;

        bool is_cache_fresh() const;
        std::optional<RegistryIndex> load_cached_index() const;
        void save_cached_index(const RegistryIndex &index) const;

        // Case-insensitive substring match
        static bool contains_ci(const std::string &haystack, const std::string &needle);
    };

} // namespace uniconv::core
