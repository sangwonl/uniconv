#include "console_output.h"

namespace uniconv::core::output {

ConsoleOutput::ConsoleOutput(std::ostream& out, std::ostream& err,
                             bool verbose, bool quiet)
    : out_(out), err_(err), verbose_(verbose), quiet_(quiet) {}

void ConsoleOutput::error(std::string_view msg) {
    err_ << "Error: " << msg << "\n";
}

void ConsoleOutput::warning(std::string_view msg) {
    if (!quiet_) {
        err_ << "Warning: " << msg << "\n";
    }
}

void ConsoleOutput::info(std::string_view msg) {
    if (!quiet_) {
        out_ << msg << "\n";
    }
}

void ConsoleOutput::success(std::string_view msg) {
    if (!quiet_) {
        out_ << msg << "\n";
    }
}

void ConsoleOutput::debug(std::string_view msg) {
    if (verbose_) {
        out_ << "[DEBUG] " << msg << "\n";
    }
}

void ConsoleOutput::data(const nlohmann::json& data,
                         const std::string& text_format) {
    if (quiet_) {
        return;
    }

    if (!text_format.empty()) {
        out_ << text_format << "\n";
    } else {
        // Default: pretty-print JSON as text
        out_ << data.dump(2) << "\n";
    }
}

void ConsoleOutput::help(std::string_view text) {
    out_ << text;
}

void ConsoleOutput::progress(std::string_view /*task*/, int /*percent*/) {
    // No-op for now, future-ready
}

bool ConsoleOutput::is_verbose() const {
    return verbose_;
}

bool ConsoleOutput::is_quiet() const {
    return quiet_;
}

void ConsoleOutput::flush() {
    out_.flush();
    err_.flush();
}

} // namespace uniconv::core::output
