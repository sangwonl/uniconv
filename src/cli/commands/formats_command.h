#pragma once

#include "cli/parser.h"
#include "core/output/output.h"
#include "core/plugin_manager.h"
#include <memory>

namespace uniconv::cli::commands {

class FormatsCommand {
public:
    FormatsCommand(std::shared_ptr<core::PluginManager> plugin_manager,
                   std::shared_ptr<core::output::IOutput> output);

    int execute(const ParsedArgs& args);

private:
    std::shared_ptr<core::PluginManager> plugin_manager_;
    std::shared_ptr<core::output::IOutput> output_;
};

} // namespace uniconv::cli::commands
