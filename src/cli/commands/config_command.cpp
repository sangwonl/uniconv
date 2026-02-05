#include "config_command.h"
#include <sstream>

namespace uniconv::cli::commands {

ConfigCommand::ConfigCommand(std::shared_ptr<core::ConfigManager> config_manager,
                             std::shared_ptr<core::output::IOutput> output)
    : config_manager_(std::move(config_manager)), output_(std::move(output)) {
}

int ConfigCommand::execute(const ParsedArgs& args) {
    if (args.subcommand_args.empty()) {
        return list(args);
    }

    const auto& action = args.subcommand_args[0];

    if (action == "list") {
        return list(args);
    } else if (action == "get") {
        return get(args);
    } else if (action == "set") {
        return set(args);
    } else if (action == "unset") {
        return unset(args);
    }

    output_->error("Unknown config action: " + action);
    output_->info("Available actions: list, get, set, unset");
    return 1;
}

int ConfigCommand::list(const ParsedArgs& /*args*/) {
    // Load current config
    config_manager_->load();

    auto keys = config_manager_->list_keys();

    if (keys.empty()) {
        output_->info("(no configuration set)");
        return 0;
    }

    // Build JSON data
    nlohmann::json j = config_manager_->to_json();

    // Build text representation
    std::ostringstream text;
    auto defaults = config_manager_->get_all_defaults();
    if (!defaults.empty()) {
        text << "Default plugins:";
        for (const auto& [key, value] : defaults) {
            text << "\n  " << key << " = " << value;
        }
    }

    auto paths = config_manager_->get_plugin_paths();
    if (!paths.empty()) {
        if (!defaults.empty()) text << "\n";
        text << "\nPlugin paths:";
        for (const auto& path : paths) {
            text << "\n  " << path.string();
        }
    }

    output_->data(j, text.str());
    return 0;
}

int ConfigCommand::get(const ParsedArgs& args) {
    if (args.subcommand.empty()) {
        output_->error("Usage: uniconv config get <key>");
        return 1;
    }

    const auto& key = args.subcommand;

    // Load current config
    config_manager_->load();

    // Check if it's a default plugin key
    if (key.starts_with("defaults.") || key.find('.') != std::string::npos) {
        // Try as default plugin key
        std::string lookup_key = key;
        if (key.starts_with("defaults.")) {
            lookup_key = key.substr(9);  // Remove "defaults." prefix
        }

        auto value = config_manager_->get_default_plugin(lookup_key);
        if (value) {
            nlohmann::json j;
            j["key"] = key;
            j["value"] = *value;
            output_->data(j, *value);
            return 0;
        }
    }

    // Try as generic setting
    auto value = config_manager_->get(key);
    if (value) {
        nlohmann::json j;
        j["key"] = key;
        j["value"] = *value;
        output_->data(j, *value);
        return 0;
    }

    output_->error("Config key not found: " + key);
    return 1;
}

int ConfigCommand::set(const ParsedArgs& args) {
    if (args.subcommand.empty() || args.subcommand_args.size() < 2) {
        output_->error("Usage: uniconv config set <key> <value>");
        return 1;
    }

    const auto& key = args.subcommand;
    const auto& value = args.subcommand_args[1];

    // Load current config
    config_manager_->load();

    // Check if it's a default plugin key (format: etl.target or defaults.etl.target)
    std::string lookup_key = key;
    bool is_default = false;

    if (key.starts_with("defaults.")) {
        lookup_key = key.substr(9);
        is_default = true;
    } else if (key.starts_with("transform.") || key.starts_with("extract.") || key.starts_with("load.")) {
        is_default = true;
    }

    if (is_default) {
        config_manager_->set_default_plugin(lookup_key, value);
    } else {
        config_manager_->set(key, value);
    }

    // Save config
    if (!config_manager_->save()) {
        output_->error("Failed to save configuration");
        return 1;
    }

    nlohmann::json j;
    j["success"] = true;
    j["key"] = key;
    j["value"] = value;
    output_->data(j, "Set " + key + " = " + value);

    return 0;
}

int ConfigCommand::unset(const ParsedArgs& args) {
    if (args.subcommand.empty()) {
        output_->error("Usage: uniconv config unset <key>");
        return 1;
    }

    const auto& key = args.subcommand;

    // Load current config
    config_manager_->load();

    bool removed = false;

    // Try as default plugin key
    std::string lookup_key = key;
    if (key.starts_with("defaults.")) {
        lookup_key = key.substr(9);
    }
    removed = config_manager_->unset_default_plugin(lookup_key);

    // Try as generic setting
    if (!removed) {
        removed = config_manager_->unset(key);
    }

    if (!removed) {
        output_->error("Config key not found: " + key);
        return 1;
    }

    // Save config
    if (!config_manager_->save()) {
        output_->error("Failed to save configuration");
        return 1;
    }

    nlohmann::json j;
    j["success"] = true;
    j["key"] = key;
    output_->data(j, "Removed " + key);

    return 0;
}

} // namespace uniconv::cli::commands
