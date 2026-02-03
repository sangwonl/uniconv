#pragma once

#include "registry_types.h"
#include <filesystem>
#include <map>
#include <optional>
#include <string>

namespace uniconv::core
{

    class InstalledPlugins
    {
    public:
        explicit InstalledPlugins(const std::filesystem::path &config_dir);

        // Load installed.json from disk
        bool load();

        // Save installed.json to disk
        bool save() const;

        // Record that a plugin was installed from registry
        void record_install(const std::string &name, const std::string &version);

        // Record removal
        void record_remove(const std::string &name);

        // Get installed version of a plugin (nullopt if not registry-installed)
        std::optional<InstalledPluginRecord> get(const std::string &name) const;

        // Get all installed plugins
        const std::map<std::string, InstalledPluginRecord> &all() const { return plugins_; }

        // Check if a plugin was installed from registry
        bool is_registry_installed(const std::string &name) const;

    private:
        std::filesystem::path file_path_;
        std::map<std::string, InstalledPluginRecord> plugins_;
    };

} // namespace uniconv::core
