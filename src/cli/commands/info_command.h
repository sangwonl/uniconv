#pragma once

#include "cli/parser.h"
#include "core/engine.h"
#include "core/output/output.h"
#include <memory>

namespace uniconv::cli::commands {

class InfoCommand {
public:
    InfoCommand(std::shared_ptr<core::Engine> engine,
                std::shared_ptr<core::output::IOutput> output);

    int execute(const ParsedArgs& args);

private:
    std::shared_ptr<core::Engine> engine_;
    std::shared_ptr<core::output::IOutput> output_;
};

} // namespace uniconv::cli::commands
