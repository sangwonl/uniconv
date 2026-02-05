#pragma once

#include "cli/parser.h"
#include "core/output/output.h"
#include "core/preset_manager.h"
#include <memory>

namespace uniconv::cli::commands {

class PresetCommand {
public:
    PresetCommand(std::shared_ptr<core::PresetManager> preset_manager,
                  std::shared_ptr<core::output::IOutput> output);

    // Execute preset management subcommand
    int execute(const ParsedArgs& args);

    // List all presets
    int list(const ParsedArgs& args);

private:
    std::shared_ptr<core::PresetManager> preset_manager_;
    std::shared_ptr<core::output::IOutput> output_;

    int create_preset(const ParsedArgs& args);
    int delete_preset(const ParsedArgs& args);
    int show_preset(const ParsedArgs& args);
    int export_preset(const ParsedArgs& args);
    int import_preset(const ParsedArgs& args);
};

} // namespace uniconv::cli::commands
