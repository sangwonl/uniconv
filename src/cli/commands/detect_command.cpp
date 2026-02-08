#include "detect_command.h"
#include "utils/file_utils.h"
#include "utils/mime_detector.h"
#include <fstream>
#include <sstream>

namespace uniconv::cli::commands {

DetectCommand::DetectCommand(std::shared_ptr<core::output::IOutput> output)
    : output_(std::move(output)) {
}

int DetectCommand::execute(const ParsedArgs& args) {
    if (args.subcommand_args.empty()) {
        output_->error("No file specified");
        return 1;
    }

    const auto& file_path = args.subcommand_args[0];
    std::filesystem::path path(file_path);

    if (!std::filesystem::exists(path)) {
        output_->error("File not found: " + file_path);
        return 1;
    }

    if (!std::filesystem::is_regular_file(path)) {
        output_->error("Not a regular file: " + file_path);
        return 1;
    }

    // Read first 4096 bytes for magic detection
    constexpr std::size_t kMagicBufSize = 4096;
    std::vector<char> buf(kMagicBufSize);

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        output_->error("Cannot open file: " + file_path);
        return 1;
    }

    ifs.read(buf.data(), static_cast<std::streamsize>(kMagicBufSize));
    auto bytes_read = static_cast<std::size_t>(ifs.gcount());

    if (bytes_read == 0) {
        output_->error("File is empty: " + file_path);
        return 1;
    }

    // Detect MIME type from content
    utils::MimeDetector detector;
    std::string mime_type = detector.detect_mime(buf.data(), bytes_read);
    std::string format = detector.detect_extension(buf.data(), bytes_read);
    std::string category = core::file_category_to_string(utils::detect_category(format));

    // Build JSON
    nlohmann::json j;
    j["path"] = path.string();
    j["mime_type"] = mime_type;
    j["format"] = format;
    j["category"] = category;

    // Build text
    std::ostringstream text;
    text << "File:     " << path.filename().string() << "\n";
    text << "MIME:     " << mime_type << "\n";
    text << "Format:   " << format << "\n";
    text << "Category: " << category;

    output_->data(j, text.str());
    return 0;
}

} // namespace uniconv::cli::commands
