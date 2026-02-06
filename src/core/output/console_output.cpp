#include "console_output.h"

#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace uniconv::core::output {

ConsoleOutput::ConsoleOutput(std::ostream& out, std::ostream& err,
                             bool verbose, bool quiet)
    : out_(out), err_(err), verbose_(verbose), quiet_(quiet),
      is_tty_(false) {
    // Detect TTY only for stderr (where progress goes)
    if (&err == &std::cerr) {
        is_tty_ = isatty(fileno(stderr)) != 0;
    }
    if (is_tty_) {
        spinner_ = std::make_unique<Spinner>(err_);
    }
}

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

void ConsoleOutput::stage_started(size_t current, size_t total, const std::string& target) {
    if (quiet_) return;

    std::ostringstream msg;
    msg << "[" << current << "/" << total << "] Converting to " << target << "...";

    if (is_tty_ && spinner_) {
        spinner_->start(msg.str());
    } else {
        err_ << msg.str() << "\n" << std::flush;
    }
}

void ConsoleOutput::stage_completed(size_t current, size_t total, const std::string& target,
                                    int64_t duration_ms, bool success, const std::string& error) {
    if (quiet_) return;

    if (is_tty_ && spinner_) {
        spinner_->stop();
    }

    std::ostringstream msg;
    if (is_tty_) {
        if (success) {
            msg << "\xe2\x9c\x93 [" << current << "/" << total << "] " << target;
        } else {
            msg << "\xe2\x9c\x97 [" << current << "/" << total << "] " << target;
        }
    } else {
        msg << "[" << current << "/" << total << "] " << target;
        if (success) {
            msg << " OK";
        } else {
            msg << " FAIL";
        }
    }

    // Format duration
    msg << " (" << std::fixed << std::setprecision(1)
        << (static_cast<double>(duration_ms) / 1000.0) << "s)";

    if (!success && !error.empty()) {
        msg << " - " << error;
    }

    err_ << msg.str() << "\n" << std::flush;
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
