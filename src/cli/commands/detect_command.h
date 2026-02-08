#pragma once

#include "cli/parser.h"
#include "core/output/output.h"
#include <memory>

namespace uniconv::cli::commands {

class DetectCommand {
public:
    explicit DetectCommand(std::shared_ptr<core::output::IOutput> output);

    int execute(const ParsedArgs& args);

private:
    std::shared_ptr<core::output::IOutput> output_;
};

} // namespace uniconv::cli::commands
