#include "info_command.h"
#include "utils/string_utils.h"
#include <iomanip>
#include <sstream>

namespace uniconv::cli::commands {

InfoCommand::InfoCommand(std::shared_ptr<core::Engine> engine,
                         std::shared_ptr<core::output::IOutput> output)
    : engine_(std::move(engine)), output_(std::move(output)) {
}

int InfoCommand::execute(const ParsedArgs& args) {
    if (args.subcommand_args.empty()) {
        output_->error("No file specified");
        return 1;
    }

    const auto& file_path = args.subcommand_args[0];

    try {
        auto info = engine_->get_file_info(file_path);

        // Build text representation
        std::ostringstream text;
        text << "File: " << info.path.filename().string() << "\n";
        text << "Format: " << utils::to_upper(info.format) << " (" << core::file_category_to_string(info.category) << ")\n";
        text << "MIME: " << info.mime_type << "\n";
        text << "Size: " << utils::format_size(info.size);

        if (info.dimensions) {
            text << "\nDimensions: " << info.dimensions->first << " x " << info.dimensions->second;
        }
        if (info.duration) {
            text << "\nDuration: " << std::fixed << std::setprecision(1) << *info.duration << "s";
        }

        output_->data(info.to_json(), text.str());
        return 0;
    } catch (const std::exception& e) {
        output_->error(e.what());
        return 1;
    }
}

} // namespace uniconv::cli::commands
