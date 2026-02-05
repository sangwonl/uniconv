#include "json_output.h"

namespace uniconv::core::output {

JsonOutput::JsonOutput(std::ostream& out, std::ostream& err,
                       bool verbose, bool quiet)
    : out_(out), err_(err), verbose_(verbose), quiet_(quiet) {}

void JsonOutput::output_message(std::ostream& os, const std::string& type,
                                std::string_view msg) {
    nlohmann::json j;
    j["type"] = type;
    j["message"] = msg;
    os << j.dump(2) << "\n";
}

void JsonOutput::error(std::string_view msg) {
    output_message(err_, "error", msg);
}

void JsonOutput::warning(std::string_view msg) {
    if (!quiet_) {
        output_message(err_, "warning", msg);
    }
}

void JsonOutput::info(std::string_view msg) {
    if (!quiet_) {
        output_message(out_, "info", msg);
    }
}

void JsonOutput::success(std::string_view msg) {
    if (!quiet_) {
        output_message(out_, "success", msg);
    }
}

void JsonOutput::debug(std::string_view msg) {
    if (verbose_) {
        output_message(out_, "debug", msg);
    }
}

void JsonOutput::data(const nlohmann::json& data,
                      const std::string& /*text_format*/) {
    // In JSON mode, ignore text_format and output raw JSON
    out_ << data.dump(2) << "\n";
}

void JsonOutput::help(std::string_view text) {
    // Help text is always plain, even in JSON mode
    out_ << text;
}

void JsonOutput::progress(std::string_view /*task*/, int /*percent*/) {
    // No-op for now, future-ready
}

bool JsonOutput::is_verbose() const {
    return verbose_;
}

bool JsonOutput::is_quiet() const {
    return quiet_;
}

void JsonOutput::flush() {
    out_.flush();
    err_.flush();
}

} // namespace uniconv::core::output
